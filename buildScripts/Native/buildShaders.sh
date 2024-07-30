cp ../SurrealEngine/RenderDevice/Bgfx/bgfx_shader.sh . 
cp ../SurrealEngine/RenderDevice/Bgfx/varying.def.sc . 

shaderc -f ../SurrealEngine/RenderDevice/Bgfx/BgfxRenderDeviceDrawTileVertex.vs -o ./BgfxRenderDeviceDrawTileVertex.glsl --type v -p 440
if [ $? -eq 0 ]; then
    echo "Vertex shader compile success"
else
    exit 1
fi

shaderc -f ../SurrealEngine/RenderDevice/Bgfx/BgfxRenderDeviceDrawTileFragment.fs -o ./BgfxRenderDeviceDrawTileFragment.glsl --type f -p 440

if [ $? -eq 0 ]; then
    echo "Fragment shader compile success"
else
    exit 1
fi

shaderc -f ../SurrealEngine/RenderDevice/Bgfx/BgfxRenderDeviceDrawTileVertex.vs -o ./BgfxRenderDeviceDrawTileVertex.spirv --type v -p spirv
if [ $? -eq 0 ]; then
    echo "Vertex shader compile success"
else
    exit 1
fi

shaderc -f ../SurrealEngine/RenderDevice/Bgfx/BgfxRenderDeviceDrawTileFragment.fs -o ./BgfxRenderDeviceDrawTileFragment.spirv --type f -p spirv

if [ $? -eq 0 ]; then
    echo "Fragment shader compile success"
else
    exit 1
fi
