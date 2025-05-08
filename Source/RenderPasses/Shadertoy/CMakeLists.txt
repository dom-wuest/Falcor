add_plugin(Shadertoy)

target_sources(Shadertoy PRIVATE
    Shadertoy.cpp
    Shadertoy.h
    Shadertoy.ps.slang
)
target_copy_shaders(Shadertoy RenderPasses/Shadertoy)
target_source_group(Shadertoy "RenderPasses")
