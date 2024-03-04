#!/bin/bash

git submodule update --init --recursive


# General deps + deps for folly not installed
sudo apt install apt-transport-https curl gnupg -y
curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor >bazel-archive-keyring.gpg
sudo mv bazel-archive-keyring.gpg /usr/share/keyrings
echo "deb [arch=amd64 signed-by=/usr/share/keyrings/bazel-archive-keyring.gpg] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
sudo apt update
sudo apt install build-essential lld clang libssl-dev libfmt-dev linux-tools-common linux-tools-generic bazel zlib libcurl-dev 

# Folly deps
(cd external_deps/folly && sudo ./build/fbcode_builder/getdeps.py install-system-deps --recursive)

# Buck install
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source "$HOME/.cargo/env"
rustup install nightly-2023-12-11
cargo +nightly-2023-12-11 install --git https://github.com/facebook/buck2.git buck2
export PATH=$HOME/.cargo/bin:$PATH

buck2 init
