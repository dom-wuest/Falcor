import falcor
from pathlib import Path
import time

DIR = Path(__file__).parent
WIDTH = 800
HEIGHT = 600

testbed = falcor.Testbed(create_window=True, width=WIDTH, height=HEIGHT)
testbed.show_ui = True
device = testbed.device

# === Create a Render Graph ===
graph = testbed.create_render_graph("ShadertoyRenderGraph")

#params = {"shaderPath": shader_path}
params = {}
shadertoy = graph.create_pass("Shadertoy", "Shadertoy", params)

graph.mark_output("Shadertoy.output")

testbed.render_graph = graph


print(shadertoy.properties)

# === Time tracking ===
prev_time = time.time()
start_time = prev_time
frame_idx = 0

# === Render Loop ===
while not testbed.should_close:
    current_time = time.time()
    delta_time = current_time - prev_time
    prev_time = current_time
    elapsed_time = current_time - start_time
    frame_idx += 1

    shadertoy.shaderInputs.iResolution = [WIDTH, HEIGHT, 0]
    shadertoy.shaderInputs.iTime = elapsed_time
    shadertoy.shaderInputs.iTimeDelta = delta_time
    shadertoy.shaderInputs.iFrameRate = 1.0 / delta_time
    shadertoy.shaderInputs.iFrame = frame_idx

    testbed.frame()

print(shadertoy.properties)
