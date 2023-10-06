#!/bin/bash

git submodule init
git submodule update


# General deps + deps for folly not installed
sudo apt install build-essential lld clang libssl-dev libfmt-dev linux-tools-common linux-tools-generic

# Folly deps
(cd externalDeps/folly && sudo ./build/fbcode_builder/getdeps.py install-system-deps --recursive)

# Buck install
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source "$HOME/.cargo/env"
rustup install nightly-2023-07-10
cargo +nightly-2023-07-10 install --git https://github.com/facebook/buck2.git buck2
export PATH=$HOME/.cargo/bin:$PATH

