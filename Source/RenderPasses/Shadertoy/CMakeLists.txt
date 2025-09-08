add_plugin(Shadertoy)

target_sources(Shadertoy PRIVATE
    Shadertoy.cpp
    Shadertoy.h
    ShadertoyParams.h
    ShadertoyTexture.h
    ShadertoyTexture.cpp
    Shadertoy.ps.slang
    FullScreenPass.vs.slang
)
target_copy_shaders(Shadertoy RenderPasses/Shadertoy)
target_source_group(Shadertoy "RenderPasses")
