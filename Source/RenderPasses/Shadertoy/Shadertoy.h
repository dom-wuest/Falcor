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
#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "Core/Pass/FullScreenPass.h"

using namespace Falcor;

/**
 * ShadertoyInputs holds the variables which may be used by Shadertoy snippets.
 * These variables are passed to the shader as uniforms.
 *
 * This container also provides setter and getter methods for each variable which are used for the python bindings.
 * The variables are set by the Shadertoy Pass and can be accessed in the shader using the iTime, iResolution, etc. variables.
 */
class ShadertoyInputs
{
public:
    ShadertoyInputs(const Properties& props)
        : iResolution(props.get<float3>("iResolution", float3(1.0f)))
        , iTime(props.get<float>("iTime", 0.0f))
        , iTimeDelta(props.get<float>("iTimeDelta", 0.0f))
        , iFrameRate(props.get<float>("iFrameRate", 60.0f))
        , iFrame(props.get<int>("iFrame", 0))
        , iMouse(props.get<float4>("iMouse", float4(0.0f))) { }

    void setResolution(float3 resolution) { iResolution = resolution; }
    void setTime(float time) { iTime = time; }
    void setTimeDelta(float timeDelta) { iTimeDelta = timeDelta; }
    void setFrameRate(float frameRate) { iFrameRate = frameRate; }
    void setFrame(int frame) { iFrame = frame; }
    void setMouse(float4 mouse) { iMouse = mouse; }

    float3 getResolution() const { return iResolution; }
    float getTime() const { return iTime; }
    float getTimeDelta() const { return iTimeDelta; }
    float getFrameRate() const { return iFrameRate; }
    int getFrame() const { return iFrame; }
    float4 getMouse() const { return iMouse; }

    Properties getProperties() const
    {
        Properties props;
        props.set("iResolution", iResolution);
        props.set("iTime", iTime);
        props.set("iTimeDelta", iTimeDelta);
        props.set("iFrameRate", iFrameRate);
        props.set("iFrame", iFrame);
        props.set("iMouse", iMouse);
        return props;
    }

private:
    float3 iResolution; ///< viewport resolution (in pixels)
    float iTime;        ///< shader playback time (in seconds)
    float iTimeDelta;   ///< render time (in seconds)
    float iFrameRate;   ///< shader frame rate
    int iFrame;         ///< shader frame count
    float4 iMouse;      ///< mouse position (in pixels)
};

class Shadertoy : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(Shadertoy, "Shadertoy", "Run a Shadertoy snippet in Falcor.");

    static ref<Shadertoy> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<Shadertoy>(pDevice, props);
    }

    Shadertoy(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override {}
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override {}
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override { return false; }
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }

    void setShaderPath(const std::string& path);
    std::string getShaderPath() const { return mShaderPath; }
    ShadertoyInputs& getInputs() { return mInputs; }
    bool getShaderLoaded() const { return mShaderLoaded; }

private:
    ref<FullScreenPass> mpFullScreenPass;
    bool mpReloadShader = false;
    ref<Fbo> mpFbo;
    std::string mShaderPath;
    ShadertoyInputs mInputs;

    bool mShaderLoaded = false;
    void loadShader();
};
