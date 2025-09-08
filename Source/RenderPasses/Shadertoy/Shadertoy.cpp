/***************************************************************************
 # Copyright (c) 2015-23, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#include "Shadertoy.h"

/**
 * here we define the bindings for the Shadertoy and ShadertoyInputs classes
 */
static void regShadertoy(pybind11::module& m)
{
    pybind11::class_<ShadertoyInputs> inputs(m, "ShadertoyInputs");
    inputs.def_property("iResolution", &ShadertoyInputs::getResolution, &ShadertoyInputs::setResolution);
    inputs.def_property("iTime", &ShadertoyInputs::getTime, &ShadertoyInputs::setTime);
    inputs.def_property("iTimeDelta", &ShadertoyInputs::getTimeDelta, &ShadertoyInputs::setTimeDelta);
    inputs.def_property("iFrameRate", &ShadertoyInputs::getFrameRate, &ShadertoyInputs::setFrameRate);
    inputs.def_property("iFrame", &ShadertoyInputs::getFrame, &ShadertoyInputs::setFrame);
    inputs.def_property("iMouse", &ShadertoyInputs::getMouse, &ShadertoyInputs::setMouse);

    pybind11::class_<Shadertoy, RenderPass, ref<Shadertoy>> pass(m, "Shadertoy");
    pass.def_property("shaderPath", &Shadertoy::getShaderPath, &Shadertoy::setShaderPath);
    pass.def_property("shaderInputs", [](Shadertoy& self) -> ShadertoyInputs& { return self.getInputs(); }, nullptr);
    pass.def_property_readonly("shaderLoaded", &Shadertoy::getShaderLoaded);
    pass.def_property("texturePath", &Shadertoy::getTexturePath, &Shadertoy::setTexturePath);
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, Shadertoy>();
    ScriptBindings::registerBinding(regShadertoy);      // register the bindings
}

Shadertoy::Shadertoy(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice), mInputs(props.get<Properties>("shaderInputs", Properties()))
{
    // Initialize the shader path (or use our default example)
    if(props.has("shaderPath")){
        mShaderPath = props.get<std::string>("shaderPath");
        loadShader();
    }
    mpFbo = Fbo::create(pDevice);
}

Properties Shadertoy::getProperties() const
{
    Properties props;
    props.set("shaderPath", mShaderPath);
    props.set("shaderInputs", mInputs.getProperties());
    props.set("shaderLoaded", mShaderLoaded);
    props.set("texturePath", getTexturePath());
    return props;
}

RenderPassReflection Shadertoy::reflect(const CompileData& compileData)
{
    // Define the required resources here
    RenderPassReflection reflector;
    reflector.addOutput("output", "Shadertoy output");
    return reflector;
}

void Shadertoy::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    if (mpReloadShader)
    {
        // Reload the shader if needed
        loadShader();

        mpReloadShader = false;

        if (mShaderLoaded && !mTexturePaths[0].empty())
        {
            loadTextures();

            // Bind textures to the shader inputs
            auto pShaderInputs = mpFullScreenPass->getRootVar();
            pShaderInputs["iChannel0"] = mpTextures[0];
        }
    }
    if (!mShaderLoaded) return;

    // renderData holds the requested resources
    const auto& pOutput = renderData.getTexture("output");
    // Set the output texture as the render target
    mpFbo->attachColorTarget(pOutput, 0);

    const float4 clearColor(0.0f, 0.0f, 1.0f, 1.0f);
    pRenderContext->clearFbo(mpFbo.get(), clearColor, 1.0f, 0);

    // Set the shader inputs
    auto pShaderInputs = mpFullScreenPass->getRootVar();
    pShaderInputs["iResolution"] = mInputs.getResolution();
    pShaderInputs["iTime"] = mInputs.getTime();
    pShaderInputs["iTimeDelta"] = mInputs.getTimeDelta();
    pShaderInputs["iFrameRate"] = mInputs.getFrameRate();
    pShaderInputs["iFrame"] = mInputs.getFrame();
    pShaderInputs["iMouse"] = mInputs.getMouse();

    // Execute the full-screen pass
    mpFullScreenPass->execute(pRenderContext, mpFbo);
}

void Shadertoy::renderUI(Gui::Widgets& widget) {}

void Shadertoy::setShaderPath(const std::string& path)
{
    if (path == mShaderPath) return; // No change

    mShaderPath = path;
    loadShader();
}

void Shadertoy::loadShader()
{
    if (mShaderPath.empty())
    {
        logError("Shader path is empty. Cannot load shader.");
        mShaderLoaded = false;
        return;
    }
    if (!Falcor::getFileModifiedTime(mShaderPath))
    {
        logError("Shader file does not exist: " + mShaderPath);
        mShaderLoaded = false;
        return;
    }
    try {
        ProgramDesc desc;
        desc.addShaderLibrary(mShaderPath).psEntry("main");
        desc.addShaderLibrary("RenderPasses/Shadertoy/FullScreenPass.vs.slang").vsEntry("main");
        mpFullScreenPass = FullScreenPass::create(mpDevice, desc);
        mShaderLoaded = true;
    } catch (const std::exception& e) {
        logError("Failed to compile shader: " + std::string(e.what()));
        mShaderLoaded = false;
    }
}

void Shadertoy::loadTextures()
{
    mpTextures[0] = Texture::createFromFile(mpDevice, mTexturePaths[0], false, false);

    if (!mpTextures[0]) {
        logError("Failed to load texture from file.");
    } else {
        uint32_t width = mpTextures[0]->getWidth();
        uint32_t height = mpTextures[0]->getHeight();
        logInfo(fmt::format("Texture loaded: {}x{}", width, height));
    }
}
