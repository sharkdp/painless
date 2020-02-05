#!/usr/bin/env python

import os.path as path
from os import listdir, unlink
from os.path import isfile

from flask import Flask, render_template, request


BASE_PATH = "/tmp/painless"

app = Flask(__name__)


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


@app.route("/")
def index():
    parameters = get_parameters()
    return render_template("base.html", parameters=parameters)


@app.route("/update", methods=["POST"])
def update():
    data = request.json
    name = data["name"]
    value = data["value"]
    print(f"Updating parameter '{name}' to value '{value}'")
    with open(path.join(BASE_PATH, name), "w") as f:
        f.write(value)
    return ""


@app.route("/remove", methods=["POST"])
def remove():
    data = request.json
    name = data["name"]
    print(f"Removing parameter '{name}'")
    unlink(path.join(BASE_PATH, name))
    return "", 204
