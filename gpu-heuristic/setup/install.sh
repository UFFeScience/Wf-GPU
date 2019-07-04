#!/bin/bash


#
# This setup file was successfully tested on:
#  * Ubuntu 14.04
#  * Ubuntu 16.04
#  * Ubuntu 17.04


sudo apt-get update

# lib-dev
sudo apt-get install build-essentials
sudo apt-get install libtclap-dev
sudo apt-get install libboost-all-dev
sudo apt-get install cmake

# g++-5
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install gcc-5 g++-5

# setup g++5 
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 10

# pip install 
sudo apt-get install python-dev
sudo apt-get install python-pip
sudo pip install numpy

