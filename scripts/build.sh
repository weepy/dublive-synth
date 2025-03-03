#!/usr/bin/env bash

DIR=".wasm"

if [ ! -d $DIR ]; then
  mkdir -p $DIR
fi

# NOTE `-std`: To use modern c++11 features like std::tuple and std::vector,
# we need to enable C++ 11 by passing the parameter to gcc through emcc.
emcc src/cpp/*.cpp \
  -std=c++17 \
  -O3 \
  -s WASM=1 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s WASM_ASYNC_COMPILATION=0 \
  -s SINGLE_FILE=1 \
  -s MODULARIZE=1 \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
  -s EXPORTED_FUNCTIONS='["_malloc","_free"]' \
  --bind \
  -o $DIR/main.js

# NOTE: We concate the emscripten generated js and wasm file with the worklet
# such that we don't have to import it later as an es6 module. This
# achieves better cross-browser compatibility.
mkdir -p static/worklets
cat $DIR/main.js src/lib/worklets/synth_worklet.js > static/worklets/synth_worklet.js



# "build:wasm": "emcc wasm/synth.cpp wasm/bindings.cpp -o static/synth.js -std=c++17 -s WASM=1 -s EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]' -s EXPORTED_FUNCTIONS='[\"_malloc\",\"_free\"]' -s ALLOW_MEMORY_GROWTH=1 -s \"EXPORT_NAME='Module'\" -s MODULARIZE=1 -s ENVIRONMENT='web' -O3 --bind",