#pragma once
#include "Falcor.h"

using namespace Falcor;

/**
 * ShadertoyParams holds the variables which may be used by Shadertoy snippets.
 * These variables are passed to the shader as uniforms.
 *
 * This container also provides setter and getter methods for each variable which are used for the python bindings.
 * The variables are set by the Shadertoy Pass and can be accessed in the shader using the iTime, iResolution, etc. variables.
 */
class ShadertoyParams
{
public:
    ShadertoyParams(const Properties& props)
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
