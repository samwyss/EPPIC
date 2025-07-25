#!/usr/bin/env bash

# create python virtual environment
python3 -m venv venv
source ./venv/bin/activate

# install standard python packages
pip3 install -r ./scripts/requirements.txt