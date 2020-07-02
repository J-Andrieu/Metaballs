# Metaballs

This project was created to learn more about compute shaders and to be a small refresher course in OpenGL. Currently this program supports 5 different 2D renderings of metaballs, as well as a default shader that simply renders circles. 

## Dependencies

This project relies on OpenGL 4.3, GLEW, and SDL2, as well as using Dear ImGui.

## Compiling and Running

If your GPU drivers are current and GLEW and SDL2 are installed, then the project can be built with the following commands. Make sure to start in the project root directory.
```bash
mkdir build
cd build
cmake ..; make -j
```
cmake can be provided the parameter -DCMAKE_BUILD_TYPE to set it to either Release or Debug if it suits your fancy, but the default is Release.

In order to run the metaballs application, ensure that the shaders folder is in the same directory as the executable, and use the command `<./metaballs>`.

Currently the executable works with two command line arguments, `<-fps>` to set a target frames per second, and `<-size>` which sets the initial size of the application window. The window size should be formated as `<HeightxWidth>`. You can also use `<-h>` to view a small help page.

## Usage

While the program is running you can use the sliders on the left to change the velocity, position, and size of each individual ball. Each ball also has a color selector associated with it. You can also use the Add/Remove Ball buttons in the upper left to add a randomized ball, or remove a ball from the end of the list. There is a dropdown at the top to select a shader, which will show any parameters associated with a shader after selection. 

In order to close the program, you can either hit the ESC key or just close the window.


## Pretty Pictures

Here's the default shader just rendering some circles.
![Circles](/images/circles.png)

This is the Cells shader, it's not as fuzzy as the others when it gloops balls together.
![Cells](/images/cells.png)

The next three are all fairly similar. The first is the Green-Blue shader, the second is the Red-Orange shader, and the last is the RGB shader.
![BlueGreen](/images/bg.png)
![RedOrange](/images/ro.png)
![RGB](/images/rgb.png)

And the last shader is the parameterized shader, basically just meant to be as flexible as it can be for the user.
![Parameterized](/images/params.png)