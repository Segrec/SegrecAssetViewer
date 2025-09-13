#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "ViewerCamera.h"

#include <iostream>

void error_callback(int error, const char* description);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void proccess_input(GLFWwindow* window);

// settings
float wWidth = 800.0f, wHeight = 600.0f;

// camera
ViewerCamera camera(glm::vec3(0.0f, 0.0f, 5.0f));
//bool firstMouse = true;
//float lastX = wWidth / 2.0f, lastY = wHeight / 2.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(wWidth, wHeight, "Segrec Asset Viewer", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		std::cerr << "Error: Failed to initialize glad" << std::endl;

	// enabling
	glEnable(GL_DEPTH_TEST);

	// shaders
	Shader shader("res/shaders/vertex/default.shader", "res/shaders/fragment/default.shader");

	// data
	float vertices[] = {
	// positins           // colors
	-0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  1.0f, 0.5f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.5f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.5f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.5f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.5f, 0.0f,
	-0.5f, -0.5f,  0.5f,  1.0f, 0.5f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  0.7f, 1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  0.7f, 1.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  0.7f, 1.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  0.7f, 1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  0.7f, 1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  0.7f, 1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 0.6f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 0.6f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.6f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.6f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.6f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.6f, 1.0f,

	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,
	};

	// filling buffers with data and sending to shader
	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// refreshing buffers
		glClearColor(0.0f, 0.5f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// per frame logic
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		proccess_input(window);

		// render
		glUseProgram(shader.GetID());
		
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFOV()), wWidth / wHeight, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		shader.SetMat4("model", model);
		shader.SetMat4("view", view);
		shader.SetMat4("projection", projection);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// polygon mode
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	wWidth = width;
	wHeight = height;
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	camera.CursorMovement(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		camera.SetFocus(true);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		camera.SetFocus(false);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glfwSetCursorPos(window, wWidth / 2.0, wHeight / 2.0);
	}
}

void proccess_input(GLFWwindow* window)
{
	
}