#! /bin/bash

# Install essential Ubuntu packages
sudo apt install g++ unzip zip

# Install LLVM/Clang version 15
wget -P /tmp/ https://apt.llvm.org/llvm.sh
chmod +x /tmp/llvm.sh
sudo /tmp/llvm.sh 15 all

# Install Bazel 6.0.0
wget -P /tmp/ https://github.com/bazelbuild/bazel/releases/download/6.0.0/bazel-6.0.0-installer-linux-x86_64.sh
chmod +x /tmp/bazel-6.0.0-installer-linux-x86_64.sh
sudo /tmp/bazel-6.0.0-installer-linux-x86_64.sh