#!/usr/bin/env python

import os.path as path
from os import listdir, unlink
from os.path import isfile
from threading import Thread

from flask import Flask, render_template
from flask_socketio import SocketIO

from inotify_simple import INotify, flags


BASE_PATH = "/tmp/painless"

app = Flask(__name__)
socketio = SocketIO(app, async_mode="threading")


def parse_parameter_file(entry):
    with open(path.join(BASE_PATH, entry), "r") as f:
        value = f.readline().strip()

    return {"name": entry, "value": value}


def get_parameters():
    parameters = []
    for entry in sorted(listdir(BASE_PATH)):
        if isfile(path.join(BASE_PATH, entry)):
            parameters.append(parse_parameter_file(entry))
    return parameters


def send_parameters():
    socketio.emit("parameter_list", {"parameters": get_parameters()}, namespace="/painless")


@app.route("/")
def index():
    return render_template("base.html")


@socketio.on("connect", namespace="/painless")
def connect():
    global thread
    if not thread.isAlive():
        thread = Thread(target=filesystem_watcher)
        thread.daemon = True
        thread.start()

    send_parameters()


@socketio.on("update", namespace="/painless")
def update(msg):
    name = msg["parameter"]
    value = msg["value"]
    print(f"Updating parameter '{name}' to value '{value}'")
    with open(path.join(BASE_PATH, name), "w") as f:
        f.write(value)
    return ""


@socketio.on("remove", namespace="/painless")
def remove(msg):
    name = msg["parameter"]
    print(f"Removing parameter '{name}'")
    unlink(path.join(BASE_PATH, name))
    send_parameters()


def filesystem_watcher():
    inotify = INotify()
    watch_flags = flags.CREATE | flags.DELETE | flags.MODIFY | \
        flags.DELETE_SELF | flags.ATTRIB | flags.MOVED_TO
    inotify.add_watch(BASE_PATH, watch_flags)
    while True:
        for event in inotify.read():
            print(event)
            send_parameters()


if __name__ == "__main__":
    thread = Thread()
    socketio.run(app)
