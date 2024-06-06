emcmake cmake --fresh .. && emmake make clean && emmake make -j 32
cp SurrealEngine.data /srv/http/SurrealEngine/SurrealEngine.data
cp SurrealEngine.js /srv/http/SurrealEngine/SurrealEngine.js
cp SurrealEngine.wasm /srv/http/SurrealEngine/SurrealEngine.wasm