import subprocess

shader_files = ["cells.comp", "circles.comp", "meta_bg.comp", "meta_params.comp", "meta_rgb.comp", "meta_ro.comp"]
for shader in shader_files:
    subprocess.call(["glslangValidator", "-G", shader, "-o", shader + ".spv"])
