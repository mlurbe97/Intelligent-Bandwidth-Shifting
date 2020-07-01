#!/bin/bash

# Update repos
sudo apt-get update

# intall python and libs
sudo apt install python-dev python-pip python3-dev python3-pip python-tk python3-tk
sudo pip install numpy pandas matplotlib tensorflow bigml
sudo pip3 install numpy pandas matplotlib tensorflow bigml

# install required libs for perlbench 2017 (doesn't work)
sudo apt-get -y install spamassassin
sudo systemctl enable spamassassin
sudo systemctl start spamassassin