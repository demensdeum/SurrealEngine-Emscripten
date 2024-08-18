# USE EMSCRIPTEN 3.1.51
# https://github.com/bkaradzic/bgfx/discussions/3266

source /home/demensdeum_stream/Apps/emsdk-3.1.51/emsdk_env.sh

clear
../buildScripts/shared/buildShaders.sh
emcmake cmake --fresh .. && emmake make clean && emmake make -j 16
cp SurrealEngine.data /srv/http/SurrealEngine/SurrealEngine.data
cp SurrealEngine.js /srv/http/SurrealEngine/SurrealEngine.js
cp SurrealEngine.wasm /srv/http/SurrealEngine/SurrealEngine.wasm
cp ../buildScripts/Emscripten/index.html /srv/http/SurrealEngine/index.html
cp ../buildScripts/Emscripten/background.jpg /srv/http/SurrealEngine/background.jpg