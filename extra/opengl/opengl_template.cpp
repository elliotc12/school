#include <stdio.h>
#include <stdlib.h>
#include <thread>

#include <GL/glew.h>

#if defined __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
    #include <GLUT/glut.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
    #include <GL/glut.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

GLFWwindow* window;

const GLchar* vertex_shader_cstr =
    "#version 150\n"
    "in vec2 position;\n"
    "void main() {\n"
    "   gl_Position = vec4(position, 0.0, 1.0);\n"
    "}\n";

  const GLchar* fragment_shader_cstr =
    "#version 150\n"
    "out vec4 outColor;\n"
    "void main() {\n"
    "   outColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "}\n";

void check_for_gl_error(const char* s) {
  if (glGetError() != GL_NO_ERROR) {
    //puts(s);
    exit(glGetError());
  }
}

void initialize_visualization() {
  glfwInit(); // turn on context-creation code -- no parameters, writes to global variables defined in above headers

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // set the values of global variables
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // make our big state machine a core state machine
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr); // GLFW talks to X11 to get display info, somehow talks to X11 and makes a window
  check_for_gl_error("There was an error creating a window.");

  glfwMakeContextCurrent(window); // GLFW sets the current window's context to ours

  glewExperimental = GL_TRUE;
  glewInit();  // library which talks to drivers and finds where their functions are defined (dynamically linking in code)

  GLuint vertexBuffer;
  glGenBuffers(1, &vertexBuffer);
  check_for_gl_error("There was an error creating the vertexBuffer object.");

  GLuint vao;
  glGenVertexArrays(1, &vao); // a datatype to store genbuffers?
  glBindVertexArray(vao);
  check_for_gl_error("There was an error creating the vertex array.");

  GLuint vbo;
  glGenBuffers(1, &vbo); // Generate 1 buffer, an OpenGL "pointer"
  glBindBuffer(GL_ARRAY_BUFFER, vbo); // set vbo as the "current array"
  check_for_gl_error("There was an error creating the vbo object.");

  GLuint ebo;
  glGenBuffers(1, &ebo); // Generate 1 buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); // set ebo as the "current element array"
  check_for_gl_error("There was an error creating the element buffer object.");

  // glEnable(GL_PROGRAM_POINT_SIZE); // make program expect gl_Pointsize from vertex shader
  // check_for_gl_error("There was an error setting GL_PROGRAM_POINT_SIZE.");

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // make a vertex shader object
  glShaderSource(vertexShader, 1, &vertex_shader_cstr, NULL);
  glCompileShader(vertexShader);
  check_for_gl_error("There was an error creating the vertex shader.");

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // make a vertex shader object
  glShaderSource(fragmentShader, 1, &fragment_shader_cstr, NULL);
  glCompileShader(fragmentShader);
  check_for_gl_error("There was an error creating the fragment shader.");

  GLint status;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status); // store compile status of v shader in status
  if (!status) {
    char buffer[512];
    glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
    printf("Couldn't compile vertex shader! MSG: %s\n", buffer);
    exit(EXIT_FAILURE);
  }

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status); // store compile status of f shader in status
  if (!status) {
    char buffer[512];
    glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
    printf("Couldn't compile fragment shader! MSG: %s\n", buffer);
    exit(EXIT_FAILURE);
  }

  GLuint shaderProgram = glCreateProgram();   // add the shaders to our program
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  check_for_gl_error("There was an error attaching shaders.");

  glBindFragDataLocation(shaderProgram, 0, "outColor"); // specify the out variables in fragment shader
  check_for_gl_error("There was an error binding fragment shader variable.");

  glLinkProgram(shaderProgram); // link the program to ours
  glUseProgram(shaderProgram); // "start using" program
  check_for_gl_error("There was an error linking/using shader program.");

  GLint posAttrib = glGetAttribLocation(shaderProgram, "position"); // get a handle to the "position" input to v shader

  glVertexAttribPointer(posAttrib, // the handle, implicitly knows about vbo bc it's the "current array"
			2,         // the size of the vector
			GL_FLOAT,  // the type
			GL_FALSE,  // whether to normalize
			0,         // spacing betwen each element in the tuple (datapoint)
			0);        // spacing between tuples (datapoints)

  glEnableVertexAttribArray(posAttrib);
  check_for_gl_error("There was an error setting vbo as the vertex shader input.");
}

int simulation_is_running() {
  return !glfwWindowShouldClose(window);
}

void update_visualization(float* vertices, GLuint* elements) {
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW); // copy data over to the "current array"
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), (GLuint*) elements, GL_DYNAMIC_DRAW); // use glBufferSubData when you start only editing part of the vertices

  glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0); // GL_POINTS, GL_LINES, GL_TRIANGLES, ...
  glfwSwapBuffers(window);
  glfwPollEvents();
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
}

void terminate_visualization() {
  glfwTerminate();
}

int main() {
  initialize_visualization();

  while(simulation_is_running()) {
    // double vertices[] = {0.0, 0.0};
    // unsigned int elements[] = {0, 1, 1, 0};

    float vertices[24] = {
		0.0f, 0.0f,
		0.5f, 0.0f,
		0.5f, 0.5f,
    };

    GLuint elements[] = {
      0, 1, 2
    };
    
    update_visualization(vertices, elements);
  }
  terminate_visualization();
}
