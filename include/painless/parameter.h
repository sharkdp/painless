#ifndef PAINLESS_PARAMETER_H
#define PAINLESS_PARAMETER_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <condition_variable>
#include <fstream>
#include <ios>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

namespace painless {

static constexpr const char* BASE_PATH = "/tmp/painless/";

namespace parser {

template <typename T>
inline bool from_string(const std::string& input, T& value) {
  std::istringstream ss(input);
  return static_cast<bool>(ss >> value);
}

template <>
inline bool from_string(const std::string& input, bool& value) {
  std::istringstream ss(input);
  return static_cast<bool>(ss >> std::boolalpha >> value);
}

template <>
inline bool from_string(const std::string& input, std::string& value) {
  value = input;
  return true;
}

}  // namespace parser

namespace printer {

template <typename T>
inline std::string to_string(const T& value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

template <>
inline std::string to_string(const bool& value) {
  std::stringstream ss;
  ss << std::boolalpha << value;
  return ss.str();
}

}  // namespace printer

std::ostream& error() {
  return std::cerr << "\x1b[31;1m[painless error]\x1b[0m ";
}

enum class WatcherState {
  NotInitialized,
  Watching,
  Error,
};

template <typename T>
class Parameter {
 public:
  Parameter(const char* name, T default_value)
      : m_parameter_name(name),
        m_default_value(default_value),
        m_current_value(default_value),
        m_file_watcher{} {
    mkdir(BASE_PATH, 0777);

    bool file_creation_successful = false;
    const auto filename = getFilename();
    {
      std::ofstream parameter_file{filename};
      if (!parameter_file.good()) {
        error() << "Could not create file '" << filename << "'." << std::endl;
      } else {
        parameter_file << printer::to_string(m_default_value) << "\n";
        parameter_file << "# Parameter '" << m_parameter_name << "'\n";
        parameter_file << "# Default value: '" << m_default_value << "'\n";
        file_creation_successful = true;
      }
    }

    if (file_creation_successful) {
      m_file_watcher = std::thread{&Parameter::fileWatcher, this};

      // Wait for watcher thread to be initialized
      {
        std::unique_lock<std::mutex> lock(m_watcher_state_mutex);
        m_watcher_state_cv.wait(lock, [this] {
          return m_watcher_state != WatcherState::NotInitialized;
        });
      }

      {
        std::lock_guard<std::mutex> lock(m_watcher_state_mutex);
        if (m_watcher_state == WatcherState::Error) {
          error() << "Could not set up watcher for file '" << getFilename()
                  << std::endl;
        }
      }
    }
  }

  T value() const {
    const std::lock_guard<std::mutex> lock(m_current_value_mutex);
    return m_current_value;
  }

  T operator*() const { return value(); }

  operator T() const { return value(); }

  ~Parameter() {
    if (unlink(getFilename().c_str()) == 0) {
      // Success
      m_file_watcher.join();
    } else {
      // Failed to remove file, detach the watcher thread
      if (m_file_watcher.joinable()) {
        m_file_watcher.detach();
      }
    }
  }

  const char* name() const { return m_parameter_name; }

 private:
  std::string getFilename() const {
    std::string filename = BASE_PATH;
    filename += m_parameter_name;
    return filename;
  }

  void fileWatcher() {
    static constexpr size_t EVENT_SIZE = sizeof(inotify_event);
    static constexpr size_t BUFFER_LENGTH = 1024 * (EVENT_SIZE + 16);

    std::array<char, BUFFER_LENGTH> buffer;

    const int fd = inotify_init();

    if (fd < 0) {
      error() << "Could not initialize inotify." << std::endl;
    }

    const int wd = inotify_add_watch(fd, getFilename().c_str(),
                                     IN_DELETE_SELF | IN_MODIFY);

    // Signal to main thread that the watch has been set up.
    {
      const std::lock_guard<std::mutex> lock(m_watcher_state_mutex);
      if (wd < 0) {
        m_watcher_state = WatcherState::Error;
      } else {
        m_watcher_state = WatcherState::Watching;
      }
    }
    m_watcher_state_cv.notify_one();

    if (wd < 0) {
      return;
    }

    int length;
    bool running = true;
    while (running) {
      length = read(fd, buffer.data(), BUFFER_LENGTH);

      if (length < 0) {
        break;
      }

      int i = 0;
      while (i < length) {
        const inotify_event* event =
            reinterpret_cast<inotify_event*>(&buffer[i]);

        if (event->mask & IN_MODIFY) {
          const T value = readCurrentValue();
          {
            const std::lock_guard<std::mutex> lock(m_current_value_mutex);
            m_current_value = value;
          }
        } else if (event->mask & IN_DELETE_SELF) {
          running = false;
        }
        i += EVENT_SIZE + event->len;
      }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
  }

  T readCurrentValue() {
    std::ifstream parameter_file(getFilename());
    if (parameter_file.is_open()) {
      std::string line;
      std::getline(parameter_file, line);

      T current_value;
      if (parser::from_string(line, current_value)) {
        return current_value;
      }
    }

    return m_default_value;
  }

  const char* m_parameter_name;
  const T m_default_value;

  mutable std::mutex m_current_value_mutex;
  T m_current_value;

  std::mutex m_watcher_state_mutex;
  std::condition_variable m_watcher_state_cv;
  WatcherState m_watcher_state = WatcherState::NotInitialized;

  std::thread m_file_watcher;
};

namespace detail {

template <typename T>
struct value_type_to_parameter_type {
  using type = T;
};

// Treat string literals as 'std::string' parameters
template <size_t N>
struct value_type_to_parameter_type<const char (&)[N]> {
  using type = std::string;
};

}  // namespace detail

template <typename T>
using value_type_to_parameter_type_t =
    typename detail::value_type_to_parameter_type<T>::type;

}  // namespace painless

template <typename T>
std::ostream& operator<<(std::ostream& out, const painless::Parameter<T>& v) {
  out << *v;
  return out;
}

#define PAINLESS_PARAMETER(name, default_value)                          \
  static painless::Parameter<                                            \
      painless::value_type_to_parameter_type_t<decltype(default_value)>> \
      name{#name, default_value};

#endif  // PAINLESS_PARAMETER_H
