#!/bin/bash

# install google test suite
sudo apt-get install googletest 
cd /usr/src/googletest
sudo cmake . && sudo cmake --build . --target install