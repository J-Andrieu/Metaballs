#include <iostream>
#include <map>
#include <cmath>

#include "GUIWindow.h"
#include "EventHandler.hpp"
#include "Timer.h"
#include "CMDParser.h"
#include "TermFormatter.hpp"

#define INVALID_UNIFORM_LOCATION 0x7fffffff

//Set up float-based times for frame-capping and such
typedef std::chrono::duration<float, std::micro> microseconds;
typedef std::chrono::duration<float, std::ratio<1, 1>> seconds;

//cmd line output formatting for some flare
TermFormatter::Formatter green({TermFormatter::FG_Green});
TermFormatter::Formatter red({TermFormatter::FG_Red});
TermFormatter::Formatter reset({TermFormatter::ResetAll});

//dealing with the command line and such
typedef struct {
  int height;
  int width;
  double fps_cap;
} cmdParams;
bool parseCMD(int argc, char* argv[], cmdParams& params);

//dear ImGui stuffs
typedef struct {
  GUIWindow* window;
  float* vals;
  float* speed;
} guiParams;
void drawMenu(void*);

int WindowHeight;
int WindowWidth;
bool WindowSizeUpdated = true;
GLuint computeUniformTime = 100; 
GLuint renderUniformSize; 

/** for later use
//shaders
std::string MoveCircles = R"comp(#version 430

layout (std430, binding = 0) buffer inputData {
  int height, width;
  int speedMult;
  int numBalls;
  struct {
    int radius; 
    struct {
      int x;
      int y;
    } pos; 
    struct {
      int x;
      int y;
    } vel;
    struct {
      int r;
      int g;
      int b;
    } col;
  } balls[];
};

int main() {
  int index = gl_GlobalInvocationID.x;// 'tis 1-dimensional
  if (index < inputData.numBalls && index >= 0) {// just to make sure nothin' silly happens
    int speedMult = inputData.speedMult;
    int height = inputData.height;
    int width = inputData.width;
    int radius = inputData.radius;
    
    //move balls according to velocity
    xPos = inputData.balls[index].pos.x + (inputData.balls[index].vel.x * speedMult);
    yPos = inputData.balls[index].pos.y + (inputData.balls[index].vel.y * speedMult);
    
    //reflect velocity if reaching/past edges
    if (xPos + radius >= width) {
      inputData.balls[index].vel.x *= -1;
    }

    if (yPos + radius >= height) {
      inputData.balls[index].vel.y *= -1;
    }
  }
})comp";

std::string ComputeColors = R"comp(#version 430

layout (std430, binding = 0) buffer inputData {
  int height, width;
  int speedMult;
  int numBalls;
  struct {
    int radius; 
    struct {
      int x;
      int y;
    } pos; 
    struct {
      int x;
      int y;
    } vel;
    struct {
      int r;
      int g;
      int b;
    } col;
  } balls[];
};

writeonly uniform image2D destTex;
layout (local_size_x = 16, local_size_y = 16) in;

int main() {
  ivec2 pixelPos = ivec2(gl_GlobalInvocationID.xy);
  for (int i = 0; i < inputData.numballs; i++) {
    if (sqrt(pow(pixelPos.x - inputData.balls[i].pos.x, 2) + pow(pixelPos.y - inputData.balls[i].pos.y, 2)) < inputData.bals[i].radius) {
      int r = inputData.balls[i].col.r;
      int g = inputData.balls[i].col.g;
      int b = inputData.balls[i].col.b;
      imageStore(destTex, pixelPos, vec4(r, g, b, 0.0));
      break;
    }
  }
})comp";
*/

const GLfloat quad_vertex_buffer_data[] = {-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f};
GLuint quad_vao;

//generate a gradient
std::string GradientShader = 
R"comp(#version 430

layout (local_size_x = 1, local_size_y = 1) in;
layout (rgba32f, binding = 0) uniform image2D img_out;

uniform float time;

void main() {
    ivec2 idx = ivec2(int(gl_GlobalInvocationID.x), int(gl_GlobalInvocationID.y));
    ivec2 image_size = ivec2(int(gl_NumWorkGroups.x), int(gl_NumWorkGroups.y));
    float r = float(idx.x) / float(image_size.x);
    float g = float(idx.y) / float(image_size.y);
    vec4 color = vec4(r, g, time, 1.0f);
    imageStore(img_out, idx, color);
})comp";

//passthrough vertex shader
std::string VertShader = 
R"vert(#version 330 core

layout (location = 0) in vec4 in_position;

void main() {
    gl_Position = in_position;
})vert";

//simple texture render
std::string FragShader = 
R"frag(#version 330 core

uniform sampler2D tex;
uniform vec2 tex_size;

layout (location = 0) out vec4 out_color;

void main() {
    out_color = texture(tex, gl_FragCoord.xy / tex_size);
})frag";

GLuint buildRenderShadersProgram(const std::string& vert, const std::string& frag);
GLuint buildComputeShader(const std::string& comp);
typedef struct {
  GLuint computeProg;
  GLuint renderProg;
  float* speed;
} graphicsParams;
void renderGraphics(void*);

int main(int argc, char* argv[]) {
  //default parameters
  cmdParams params;
  params.height = 0;
  params.width = 0;
  params.fps_cap = 60;
  if (!parseCMD(argc, argv, params)) {
    return -1;
  }

  //set up frame capping
  float FPS = params.fps_cap;
  seconds FRAME_TIME = (seconds) 1.0f/FPS;
  size_t frame_count = 0;
  auto main_start = Timer::getCurrentTime();

  Timer frame_timer;

  //create the main window
  //Window::DefaultWindowFlags() | SDL_WINDOW_UTILITY = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_UTILITY
  //SDL_WINDOW_UTILITY maintains WM compositing features
  GUIWindow MainWindow("This Is A Test", params.height, params.width, Window::DefaultWindowFlags() | SDL_WINDOW_UTILITY);//initializes SDL2, OpenGL, and ImGUI
  
  SDL_SetWindowMinimumSize(MainWindow, 700, 400);
  SDL_GL_MakeCurrent(MainWindow, MainWindow.getContext());

  ImGui::StyleColorsDark();

  //prepare the shaders
  GLuint renderProg = buildRenderShadersProgram(VertShader, FragShader);
  renderUniformSize = glGetUniformLocation(renderProg, "tex_size");
  if (renderUniformSize == INVALID_UNIFORM_LOCATION) {
    throw std::runtime_error(std::string("Uniform time could not be found"));
  }
  GLuint gradientProg = buildComputeShader(GradientShader);
  computeUniformTime = glGetUniformLocation(gradientProg, "time");
  if (computeUniformTime == INVALID_UNIFORM_LOCATION) {
    throw std::runtime_error(std::string("Uniform time could not be found"));
  }

  //prepare vertex array for rendering texture to window
  glGenVertexArrays(1, &quad_vao);
  glBindVertexArray(quad_vao);
  GLuint quad_vbo;
  glGenBuffers(1, &quad_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertex_buffer_data), quad_vertex_buffer_data, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);    
  glDeleteBuffers(1, &quad_vbo);

  //attach draw functions to MainWindow
  float gradient_speed = 0.01f;
  MainWindow.setDrawFunc(renderGraphics);
  graphicsParams graphics_params = {gradientProg, renderProg, &gradient_speed};
  MainWindow.setDrawParams((void*) &graphics_params);
  MainWindow.setGUIFunc(drawMenu);
  float slider_quad[] = {42, 69, 420, 9001};
  guiParams gui_params = {&MainWindow, slider_quad, &gradient_speed};
  MainWindow.setGUIParams((void*) &gui_params);

  //zeh game loop
  bool running = true;
  while (running) {
    frame_timer.start();
    EventHandler::poll();

    for (auto event : EventHandler::events) {
      MainWindow.handleEvent(event);
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }

    if (EventHandler::keyDown[EventHandler::keys::ESC] || MainWindow.isHidden()) {
      running = false;
    }

    //update window
    if (running) {
      int newWidth, newHeight;
      SDL_GetWindowSize(MainWindow, &newWidth, &newHeight);
      if (newWidth != WindowWidth || newHeight != WindowHeight) {
        WindowWidth = newWidth;
        WindowHeight = newHeight;
        WindowSizeUpdated = true;
      }
      MainWindow.draw();
      MainWindow.drawGUI();
      MainWindow.swap();
      WindowSizeUpdated = false;
    }

    auto end_time = (microseconds) frame_timer.getMicrosecondsElapsed();
    //print out current frame rate, green if it's fast, red if it's slow
    if(frame_count % 15 == 0) {
      float curr_fps = 1.0f / (end_time.count() / 1000000);
      std::cout << "\r                      \r" << std::flush;
      if (curr_fps >= FPS) {
        std::cout << green << curr_fps;
      } else {
        std::cout << red << curr_fps;
      }
      std::cout << std::flush;
    }
    //Timer::wait(FRAME_TIME - end_time);
    frame_count++;
  }

  std::cout << reset << '\r';

  return 0;
}

//parse the command line... yeah.
bool parseCMD(int argc, char* argv[], cmdParams& params) {
  std::string size; //HxW
  CMDParser parser;
  parser.bindVar<std::string>("-size", size, 1, "<Height>x<Width> of the window");
  parser.bindVar<double>("-fps", params.fps_cap, 1, "FPS cap");
  if (!parser.parse(argc, argv)) {
    return false;
  }
  if (size.size() != 0) {
    std::string height;
    std::string width;
    size_t x_index = 0;

    //split on x
    while (x_index < size.size() && size[x_index] != 'x') {
      x_index++;
    }
    if (x_index >= size.size()) {
      std::cout << "Incorrect format, -size requires format <Height>x<Width>" << std::endl;
      parser.printHelp();
      return false;
    }
    height = size.substr(0, x_index);
    width = size.substr(x_index + 1, size.size() - x_index - 1);

    //attempt to convert substrings to integers
    try {
      params.height = std::stoi(height);
    } catch (...) {
      std::cout << "Could not convert " << height << " to integer height" << std::endl;
      parser.printHelp();
      return false;
    }
    try {
      params.width = std::stoi(width);
    } catch (...) {
      std::cout << "Could not convert " << width << " to integer width" << std::endl;
      parser.printHelp();
      return false;
    }
  }
  return true;
}

//returns a program built from the provided shaders
//will throw a runtime exception if something breaks
GLuint buildRenderShadersProgram(const std::string& vert, const std::string& frag) {
  //create the shader program
  GLuint shaderProg = glCreateProgram();

	if (shaderProg == 0) {
		throw std::runtime_error(std::string("Error creating shader program"));
	}

  //compile shaders and attach to program
  std::vector<GLuint> shaderObjectFiles;
  auto attachShader = [shaderProg, &shaderObjectFiles](const std::string& shader, GLenum shaderType) -> void {
    GLuint shaderObj = glCreateShader(shaderType);
    shaderObjectFiles.push_back(shaderObj);

    if (shaderObj == 0) {
		  throw std::runtime_error(std::string("Error creating shader type ") + std::to_string(shaderType));
	  }

    const GLchar* p[1];
    p[0] = shader.c_str();
    GLint lengths[1] = { (GLint) shader.size() };

    glShaderSource(shaderObj, 1, p, lengths);
	  glCompileShader(shaderObj);

    GLint success;
	  glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);

    if (!success) {
		  GLchar log[1024];
		  glGetShaderInfoLog(shaderObj, 1024, NULL, log);
		  std::cerr << "Error compiling: " << log << std::endl;
      throw std::runtime_error(std::string("Error compiling shader, shader type: ") + std::to_string(shaderType));
	  }

    glAttachShader(shaderProg, shaderObj);
  };

  attachShader(vert, GL_VERTEX_SHADER);
  attachShader(frag, GL_FRAGMENT_SHADER);

  //link the shader program
  GLint success = 0;
	GLchar errorLog[1024] = { 0 };

	glLinkProgram (shaderProg);

  glGetProgramiv(shaderProg, GL_LINK_STATUS, &success);
	if (success == 0) {
		glGetProgramInfoLog(shaderProg, sizeof(errorLog), NULL, errorLog);
		std::cerr << "Error linking shader program: " << errorLog << std::endl;
		throw std::runtime_error(std::string("Error linking rendering shader program"));
	}

  glValidateProgram(shaderProg);
	glGetProgramiv(shaderProg, GL_VALIDATE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProg, sizeof(errorLog), NULL, errorLog);
		std::cerr << "Invalid shader program: " << errorLog << std::endl;
		throw std::runtime_error(std::string("Rendering pipeline program is invalid"));
	}

  for (auto i = shaderObjectFiles.begin(); i != shaderObjectFiles.end(); i++) {
		glDeleteShader(*i);
	}

  return shaderProg;
}

//returns a gpgpu program built from the provided shader
//will throw a runtime exception if something breaks
GLuint buildComputeShader(const std::string& comp) {
  GLuint shaderObj = glCreateShader(GL_COMPUTE_SHADER);

  if (shaderObj == 0) {
		throw std::runtime_error(std::string("Error creating compute shader"));
	}

  const GLchar* p[1];
  p[0] = comp.c_str();
  GLint lengths[1] = { (GLint) comp.size() };

  glShaderSource(shaderObj, 1, p, lengths);
	glCompileShader(shaderObj);

  GLint success;
	glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);

  if (!success) {
	  GLchar log[1024];
	  glGetShaderInfoLog(shaderObj, 1024, NULL, log);
		std::cerr << "Error compiling: " << log << std::endl;
    throw std::runtime_error(std::string("Error compiling compute shader"));
	}

  GLuint program = glCreateProgram();

  if (program == 0) {
		throw std::runtime_error(std::string("Error creating compute shader program"));
	}
  
  glAttachShader(program, shaderObj);
  glLinkProgram(program);

  success = 0;
	GLchar errorLog[1024] = { 0 };
  glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (success == 0) {
		glGetProgramInfoLog(program, sizeof(errorLog), NULL, errorLog);
		std::cerr << "Error linking shader program: " << errorLog << std::endl;
		throw std::runtime_error(std::string("Error linking compute shader program"));
	}

  glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, sizeof(errorLog), NULL, errorLog);
		std::cerr << "Invalid shader program: " << errorLog << std::endl;
		throw std::runtime_error(std::string("Compute program is invalid"));
	}

  glDeleteShader(shaderObj);
  return program;
}

//clear the screen and render a gradient
void renderGraphics(void* _params) {
  static float time_offset = 0;
  static GLuint tex_out = 0;
  //this is made of memory leaks, should be stored in object 
  //when finalized for proper resource freeing
  if (WindowSizeUpdated || tex_out == 0) {
    if (tex_out != 0) {
      glDeleteTextures(1, &tex_out);
    }
    glGenTextures(1, &tex_out);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_out);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WindowWidth - 300, WindowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, tex_out, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  }

  graphicsParams* params = (graphicsParams*) _params;

  //empty the window
  glClearColor(0, 123, 225, 225);
  glClear(GL_COLOR_BUFFER_BIT);
  
  //compute the gradient
  {
    glUseProgram(params->computeProg);
    time_offset += *params->speed;
    float i = std::sin(time_offset);
    i += 1.0f;
    i /= 2.0f;
    glUniform1f(computeUniformTime, i);
    glDispatchCompute((GLuint) WindowWidth - 300, (GLuint) WindowHeight, 1);
  }
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  //render the texture
  {
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(300, 0, WindowWidth - 300, WindowHeight);
    glUseProgram(params->renderProg);
    glUniform2f(renderUniformSize, (float) WindowWidth, (float) WindowHeight);
    glBindVertexArray(quad_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_out);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
}

//draw the test panel
void drawMenu(void* _params) {
  guiParams* params = (guiParams*) _params;

  //start the frame
  GUIWindow::NewFrame(*params->window);

  //set up styling and location
  ImGuiStyle& style = ImGui::GetStyle();
  style.WindowRounding = 0.0f;
  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Appearing);
  int width = WindowWidth;
  int height = WindowHeight;
  ImGui::SetNextWindowSize(ImVec2(300, height), ImGuiCond_Always);

  //begin the side panel
  bool barAndStuffs = true;
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoResize;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  window_flags |= ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoScrollbar;
  window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
  ImGui::Begin("Bar and stuffs", &barAndStuffs, window_flags);
  
  //zeh stuff in the panel
  ImGui::Text("Important stuff will go here >.>");

  //block of configurable values
  ImGui::SliderFloat("Gradient Speed", params->speed, 0.0f, 0.1f);
  for (int i = 0; i < 4; i++) {
    ImGui::SliderFloat4("lol,", params->vals, 0, 42);
    ImGui::SliderFloat4("random", params->vals, 0, 42);
    if (i != 3) { 
      ImGui::SliderFloat4("sliders", params->vals, 0, 42);
    }
  }

  //block of graphs (scrollable)
  window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoResize;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  window_flags |= ImGuiWindowFlags_NoMove;
  ImGui::BeginChildFrame(ImGui::GetID("Bar and stuffs"), ImVec2(300, height - 305), window_flags);
  for (int i = 0; i < 150; i++) {
    ImGui::Text("Lol, this is where the graphs would go");
  }
  ImGui::Text(">.>");
  ImGui::EndChildFrame();

  ImGui::End();

  //end the frame
  GUIWindow::RenderFrame();
}
