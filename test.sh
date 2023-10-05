#!/bin/bash

buck2 build psychopomp/test:test

perf record -g buck2 run psychopomp/test:test
perf script > out.perf

mv out.perf externalDeps/FlameGraph

(cd externalDeps/FlameGraph && ./stackcollapse-perf.pl out.perf | perl flamegraph.pl > kernel.svg)

mv externalDeps/FlameGraph/kernel.svg kernel.svg
rm externalDeps/FlameGraph/out.perf


