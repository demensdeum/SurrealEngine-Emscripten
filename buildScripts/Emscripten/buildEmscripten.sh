clear
emcmake cmake --fresh .. && emmake make clean && emmake make -j 16
cp SurrealEngine.data /srv/http/SurrealEngine/SurrealEngine.data
cp SurrealEngine.js /srv/http/SurrealEngine/SurrealEngine.js
cp SurrealEngine.wasm /srv/http/SurrealEngine/SurrealEngine.wasm
cp ../buildScripts/Emscripten/index.html /srv/http/SurrealEngine/index.html
cp ../buildScripts/Emscripten/background.jpg /srv/http/SurrealEngine/background.jpg