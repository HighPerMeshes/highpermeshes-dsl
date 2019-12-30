#!/bin/bash

# add unsupported packages for gcc-8 and g++-8 installation
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt update

# install gcc-8 and g++-8
sudo apt-get install gcc-8 g++-8
    
# update gcc and g++ symlinks
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 60