import falcor
from pathlib import Path
import time
from PIL import Image

DIR = Path(__file__).parent
WIDTH = 800
HEIGHT = 450
CREATE_WINDOW = True
SHOW_UI = True

# === Screenshot Utility ===
def screenshot_falcor(filename):
    # write the output to a file using falcor api
    testbed.capture_output(DIR.parent.parent.parent / "captures" / filename, 0)

def screenshot_numpy(filename):
    # use the output resource and convert to numpy array
    output_resource = graph.get_output("Shadertoy.output")

    img = output_resource.to_numpy().reshape((HEIGHT, WIDTH, 4))
    img_rgba8 = img[:,:, [2,1,0,3]] # reorder channels to RGBA

    Image.fromarray(img_rgba8).save(DIR.parent.parent.parent / "captures" / filename)

# === Initialize the Testbed ===
testbed = falcor.Testbed(create_window=CREATE_WINDOW, width=WIDTH, height=HEIGHT)
testbed.show_ui = SHOW_UI
device = testbed.device

# === Create a Render Graph ===
graph = testbed.create_render_graph("ShadertoyRenderGraph")

#params = {"shaderPath": DIR / "primitives.ps.slang"}
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

    # === Update the Uniform Variables ===
    shadertoy.shaderInputs.iResolution = [WIDTH, HEIGHT, 0]
    shadertoy.shaderInputs.iTime = elapsed_time
    shadertoy.shaderInputs.iTimeDelta = delta_time
    shadertoy.shaderInputs.iFrameRate = 1.0 / delta_time
    shadertoy.shaderInputs.iFrame = frame_idx

    testbed.frame() # render the frame to the framebuffer

    if frame_idx == 10000: # take a screenshot at 10000 frames
        screenshot_falcor(f"ShadertoyOutput{frame_idx}_falcor.png")
        screenshot_numpy(f"ShadertoyOutput{frame_idx}_numpy.png")

        # switch to a different shader
        shadertoy.shaderPath = str(DIR / "primitives.ps.slang")


print(shadertoy.properties)
