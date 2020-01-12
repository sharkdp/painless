#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <condition_variable>
#include <fstream>
#include <mutex>
#include <thread>

namespace painless {

static constexpr const char* BASE_PATH = "/tmp/painless/";

template <typename T>
class Parameter {
 public:
  Parameter(const char* name, T default_value)
      : m_parameter_name(name),
        m_default_value(default_value),
        m_current_value(default_value),
        m_file_watcher{} {
    mkdir(BASE_PATH, 0777);  // TODO: error handling

    const auto filename = getFilename();
    {
      std::ofstream parameter_file{filename};
      parameter_file << m_default_value << "\n";
      parameter_file << "# Parameter '" << m_parameter_name << "'\n";
      parameter_file << "# Default value: '" << m_default_value << "'\n";
    }

    m_file_watcher = std::thread{&Parameter::fileWatcher, this};

    // Wait for watcher thread to be initialized
    std::unique_lock<std::mutex> lock(m_watcher_initialized_mutex);
    m_watcher_initialized_cv.wait(lock,
                                  [this] { return m_watcher_initialized; });
  }

  T operator*() const {
    const std::lock_guard<std::mutex> lock(m_current_value_mutex);
    return m_current_value;
  }

  ~Parameter() {
    m_file_watcher.detach();  // TODO
  }

  const char* name() const {
    return m_parameter_name;
  }

 private:
  std::string getFilename() const {
    std::string filename = BASE_PATH;
    filename += m_parameter_name;
    return filename;
  }

  void fileWatcher() {
    static constexpr size_t EVENT_SIZE = sizeof(inotify_event);
    static constexpr size_t BUF_LEN = 1024 * (EVENT_SIZE + 16);

    char buffer[BUF_LEN];

    int fd = inotify_init();

    if (fd < 0) {
      perror("inotify_init");
    }

    int wd = inotify_add_watch(fd, BASE_PATH, IN_MODIFY);

    {
        const std::lock_guard<std::mutex> lock(m_watcher_initialized_mutex);
        m_watcher_initialized = true;
    }
    m_watcher_initialized_cv.notify_one();

    int length;
    do {
      length = read(fd, buffer, BUF_LEN);

      if (length < 0) {
        perror("read");
      }

      int i = 0;
      while (i < length) {
        inotify_event* event = reinterpret_cast<inotify_event*>(&buffer[i]);
        if (event->len) {
          if (event->mask & IN_MODIFY) {
            const T value = readCurrentValue();
            {
              const std::lock_guard<std::mutex> lock(m_current_value_mutex);
              m_current_value = value;
            }
          }
        }
        i += EVENT_SIZE + event->len;
      }

    } while (length > 0);

    inotify_rm_watch(fd, wd);
    close(fd);
  }

  T readCurrentValue() {
    std::ifstream param_file(getFilename());
    if (param_file.is_open()) {
      T current_value;
      if (param_file >> current_value) {
        return current_value;
      }
    }

    return m_default_value;
  }

  const char* m_parameter_name;
  T m_default_value;

  mutable std::mutex m_current_value_mutex;
  T m_current_value;

  std::mutex m_watcher_initialized_mutex;
  std::condition_variable m_watcher_initialized_cv;
  bool m_watcher_initialized = false;

  std::thread m_file_watcher;
};

}  // namespace painless

#define PAINLESS_PARAMETER(name, default_value)                   \
  static painless::Parameter<decltype(default_value)> name{#name, \
                                                           default_value};
