#!/bin/bash
curdir=`pwd`
echo "export LD_LIBRARY_PATH=$curdir:$LD_LIBRARY_PATH" >> ~/.bashrc
echo "export PATH=$PATH:/home/seineo/Desktop/CS/Gitlet" >> ~/.bashrc
echo "alias gitlet=\"main\"" >> ~/.bashrc
