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
 * here we define the bindings for the Shadertoy and ShadertoyParams classes
 */
static void regShadertoy(pybind11::module& m)
{
    pybind11::class_<ShadertoyParams> params(m, "ShadertoyParams");
    params.def_property("iResolution", &ShadertoyParams::getResolution, &ShadertoyParams::setResolution);
    params.def_property("iTime", &ShadertoyParams::getTime, &ShadertoyParams::setTime);
    params.def_property("iTimeDelta", &ShadertoyParams::getTimeDelta, &ShadertoyParams::setTimeDelta);
    params.def_property("iFrameRate", &ShadertoyParams::getFrameRate, &ShadertoyParams::setFrameRate);
    params.def_property("iFrame", &ShadertoyParams::getFrame, &ShadertoyParams::setFrame);
    params.def_property("iMouse", &ShadertoyParams::getMouse, &ShadertoyParams::setMouse);

    pybind11::enum_<Filter>(m, "Filter")
        .value("Nearest", Filter::Nearest)
        .value("Linear", Filter::Linear)
        .value("Mipmap", Filter::Mipmap);

    pybind11::enum_<Wrap>(m, "Wrap")
        .value("Repeat", Wrap::Repeat)
        .value("Clamp", Wrap::Clamp);

    pybind11::class_<ShadertoyTexture, ref<ShadertoyTexture>> texture(m, "ShadertoyTexture");
    texture.def(pybind11::init([](ref<Device> pDevice, const std::string& texturePath, Filter filter, Wrap wrap, bool srgb) {
        return ShadertoyTexture::create(pDevice, texturePath, filter, wrap, srgb);
    }), pybind11::arg("device"), pybind11::arg("texturePath"), pybind11::arg("filter") = Filter::Linear, pybind11::arg("wrap") = Wrap::Repeat, pybind11::arg("srgb") = false);
    texture.def_property_readonly("path", &ShadertoyTexture::getTexturePath);
    texture.def_property_readonly("filter", &ShadertoyTexture::getFilter);
    texture.def_property_readonly("wrap", &ShadertoyTexture::getWrap);
    texture.def_property_readonly("srgb", &ShadertoyTexture::isSrgb);

    pybind11::class_<Shadertoy, RenderPass, ref<Shadertoy>> pass(m, "Shadertoy");
    pass.def_property("shaderPath", &Shadertoy::getShaderPath, &Shadertoy::setShaderPath);
    pass.def_property("shaderInputs", [](Shadertoy& self) -> ShadertoyParams& { return self.getParams(); }, nullptr);
    pass.def_property_readonly("shaderLoaded", &Shadertoy::getShaderLoaded);
    for (int i = 0; i < 4; ++i)
    {
        pass.def_property(
            ("iChannel" + std::to_string(i)).c_str(),
            [i](Shadertoy& self) { return self.getTexture(i); },
            [i](Shadertoy& self, const ref<ShadertoyTexture>& tex) { self.setTexture(i, tex); }
        );
    }
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, Shadertoy>();
    ScriptBindings::registerBinding(regShadertoy);      // register the bindings
}

Shadertoy::Shadertoy(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice), mParams(props.get<Properties>("shaderInputs", Properties()))
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
    props.set("shaderInputs", mParams.getProperties());
    props.set("shaderLoaded", mShaderLoaded);
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

        // Re-bind the textures
        for (int i = 0; i < 4; ++i)
        {
            if (mpTextures[i])
            {
                bindTex(i);
            }
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
    pShaderInputs["iResolution"] = mParams.getResolution();
    pShaderInputs["iTime"] = mParams.getTime();
    pShaderInputs["iTimeDelta"] = mParams.getTimeDelta();
    pShaderInputs["iFrameRate"] = mParams.getFrameRate();
    pShaderInputs["iFrame"] = mParams.getFrame();
    pShaderInputs["iMouse"] = mParams.getMouse();

    // Execute the full-screen pass
    mpFullScreenPass->execute(pRenderContext, mpFbo);
}

void Shadertoy::renderUI(Gui::Widgets& widget) {
    // Shader path
    if (widget.textbox("Shader Path", mShaderPath)) {
        mpReloadShader = true;
    }

    // display textures bound to channels
    for (int i = 0; i < 4; ++i) {
        std::string label = "iChannel" + std::to_string(i);
        if (mpTextures[i]) {
            widget.text(label + ": " + mpTextures[i]->getTexturePath());
        } else {
            widget.text(label + ": <none>"); 
        }
    }
}

void Shadertoy::setShaderPath(const std::string& path)
{
    if (path == mShaderPath) return; // No change

    mShaderPath = path;
    loadShader();
}

void Shadertoy::setTexture(int channel, const ref<ShadertoyTexture>& texture)
{
    if (channel < 0 || channel >= 4) {
        logError("Channel index out of bounds. Must be between 0 and 3.");
        return;
    }
    mpTextures[channel] = texture;
    bindTex(channel);
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

void Shadertoy::bindTex(int channel)
{
    if (channel < 0 || channel >= 4) {
        logError("Channel index out of bounds. Must be between 0 and 3.");
        return;
    }
    if (!mpTextures[channel]) {
        logError("Texture for channel " + std::to_string(channel) + " is not set.");
        return;
    }
    if (mShaderLoaded) {
        auto pShaderInputs = mpFullScreenPass->getRootVar();
        std::string channelName = "iChannel" + std::to_string(channel);
        std::string samplerName = "_iChannel" + std::to_string(channel) + "_sampler";
        pShaderInputs[channelName] = mpTextures[channel]->getTexture();
        pShaderInputs[samplerName] = mpTextures[channel]->getSampler();
    }
}
