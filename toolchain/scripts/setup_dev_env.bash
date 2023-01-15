#! /bin/bash

# Download the install script for LLVM/Clang
wget https://apt.llvm.org/llvm.sh

# Install LLVM/Clang version 15
chmod +x llvm.sh
sudo ./llvm.sh 15 all