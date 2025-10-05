#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "Shader.h"
#include "ViewerCamera.h"
#include "Model.h"

#include <iostream>
#include <filesystem>
#include <string>

void error_callback(int error, const char* description);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void proccess_input(GLFWwindow* window);
glm::vec3 GetLightDirection(float x, float y);
void SetLitMode();
void SetWireframeMode();
unsigned int loadTexture(const char* path, bool gammaCorrection = false);

// settings
float wWidth = 1200.0f, wHeight = 800.0f;
bool imgui_mouse_capture = false;
bool imgui_keyboard_capture = false;
bool default_model = true;

// camera
ViewerCamera camera(glm::vec3(0.0f, 0.0f, 5.0f));

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main(int argc, char* argv[])
{
	try {
		std::filesystem::path exeDir = std::filesystem::path(argv[0]).parent_path();
		std::filesystem::current_path(exeDir);
	}
	catch (const std::exception& e) {
		std::cerr << "Nelze nastavit pracovní adresáø: " << e.what() << std::endl;
	}

	glfwSetErrorCallback(error_callback);
	
	if (!glfwInit())
		return -1;
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

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

	// setting window icon
	GLFWimage images[1];
	images[0].pixels = stbi_load("res/icons/segrec_logo_s.png", &images[0].width, &images[0].height, 0, 4);
	if (images[0].pixels)
	{
		glfwSetWindowIcon(window, 1, images);
		stbi_image_free(images[0].pixels);
	}

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		std::cerr << "Error: Failed to initialize glad" << std::endl;

	// enabling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);

	Shader shader("res/shaders/vertex/default.shader", "res/shaders/fragment/default.shader");
	Shader depthShader("res/shaders/vertex/depth.shader", "res/shaders/fragment/depth.shader");
	Shader wireframeShader("res/shaders/vertex/wireframe.shader", "res/shaders/fragment/wireframe.shader");
	Shader unlitShader("res/shaders/vertex/unlit.shader", "res/shaders/fragment/unlit.shader");
	
	Shader* current_shader = &shader;

	// data
	float vertices[] = {
		// position           // normals         // texcoords // tangents        // bitangents
		-5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  5.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
		 5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 5.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  5.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
		 5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 5.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
		 5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  5.0f, 5.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
	};

	// filling buffers with data and sending to shader
	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));

	glBindVertexArray(0);

	// loading assets
	Model barrel("res/assets/A_ApetrolBarrel_UE.fbx");
	Model* current_model = &barrel;

	if (argc > 1)
	{
		current_model = new Model(argv[1]);
		std::cout << argv[1] << std::endl;
		default_model = false;
	}

	// textures
	unsigned int stone_floor_diffuse = loadTexture("res/textures/stone_floor.jpg", true);
	unsigned int stone_floor_roughness = loadTexture("res/textures/stone_floor_roughness.jpg");
	unsigned int apetrol_diffuse = loadTexture("res/textures/T_ApetrolBarrel_diff_1k.jpg", true);
	unsigned int apetrol_roughness = loadTexture("res/textures/T_ApetrolBarrel_rough_1k.jpg");
	unsigned int apetrol_normal = loadTexture("res/textures/T_ApetrolBarrel_normal_gl_1k.jpg");
	unsigned int debug_diffuse = loadTexture("res/textures/tex_DebugUVTiles.png", true);
	unsigned int default_roughness = loadTexture("res/textures/T_DefaultRoughness.jpg");
	unsigned int empty_normal = loadTexture("res/textures/T_EmptyNormal.jpg");
	unsigned int lit_icon = loadTexture("res/icons/lit_button_icon.png");
	unsigned int wireframe_icon = loadTexture("res/icons/wireframe_button_icon.png");
	unsigned int unlit_icon = loadTexture("res/icons/unlit_button_icon.png");

	// shadows
	// -------
	const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// shader configuration
	shader.Use();
	shader.SetInt("material.diffuse", 0);
	shader.SetInt("material.roughness", 1);
	shader.SetInt("material.normal", 2);
	shader.SetInt("shadowMap", 3);
	shader.SetFloat("material.shininess", 64);

	// light
	glm::vec3 light_direction = glm::vec3(0.5f, -1.0f, -0.5f);
	shader.SetVec3("light.direction", light_direction);
	shader.SetVec3("lightDirection", light_direction);
	shader.SetVec3("light.ambient", 0.1f, 0.1f, 0.1f);
	shader.SetVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
	shader.SetVec3("light.specular", 0.3f, 0.3f, 0.3f);

	// ImGui inizialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui::StyleColorsClassic();
    ImGui_ImplOpenGL3_Init((char*)glGetString(330));

	ImGuiIO& io = ImGui::GetIO();

	// ImGui variables
	// ---------------

	// textures
	static char diffuse_path_buffer[512];
	std::string diffuse_map_path;
	static char roughness_path_buffer[512];
	std::string roughness_map_path;
	static char normal_path_buffer[512];
	std::string normal_map_path;

	unsigned int diffuse_map;
	unsigned int roughness_map;
	unsigned int normal_map;

	if (!default_model)
	{
		diffuse_map = debug_diffuse;
		roughness_map = default_roughness;
		normal_map = empty_normal;
	}
	else
	{
		diffuse_map = apetrol_diffuse;
		roughness_map = apetrol_roughness;
		normal_map = apetrol_normal;
	}

	// light
	float light_intensity = 1.0f;
	float ambient_intensity = 1.0f;
	float specular_intensity = 1.0f;
	float shininess = 64.0f;
	float light_color[3] = { 1.0f, 1.0f, 1.0f };
	bool render_shadows = true;
	float light_rotation[2] = { 45.0f, 45.0f };

	// asset
	float asset_translation[3] = { 0.0f, 0.0f, 0.0f };
	float asset_rotation[3] = { 0.0f, 0.0f, 0.0f };
	float uniform_scale = 1.0f;

	// editor
	float background_color[3] = { 0.05, 0.05, 0.05f};
	float wire_color[3] = { 0.9f, 0.9f, 0.9f };
	bool render_plane = true;

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// refreshing buffers
		glClearColor(background_color[0], background_color[1], background_color[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// imgui starts new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		imgui_mouse_capture = io.WantCaptureMouse;
		imgui_keyboard_capture = io.WantCaptureKeyboard;
		ImGuiMouseCursor cursor = ImGuiMouseCursor_Arrow;

		// per frame logic
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		proccess_input(window);

		// render
		current_shader->Use();
		current_shader->SetVec3("viewPos", camera.m_Position);

		// matrices
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFOV()), wWidth / wHeight, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		current_shader->SetMat4("model", model);
		current_shader->SetMat4("view", view);
		current_shader->SetMat4("projection", projection);

		// rendering depth to texture
		// --------------------------
		depthShader.Use();

		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 25.0f);
		glm::vec3 target = glm::vec3(0.0f);
		glm::vec3 lightPos = target - light_direction * 10.0f;
		glm::mat4 lightView = glm::lookAt(lightPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		depthShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
		depthShader.SetMat4("model", model);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		// render scene
		if (render_plane)
		{
			glBindVertexArray(VAO);
			model = glm::mat4(1.0f);
			depthShader.SetMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
		model = glm::translate(model, glm::vec3(asset_translation[0], asset_translation[1], asset_translation[2]));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(asset_rotation[0]), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(asset_rotation[1]), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(asset_rotation[2]), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(uniform_scale));
		depthShader.SetMat4("model", model);
		current_model->Draw(shader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// rendering scene
		// ---------------

		// reset viewport
		glViewport(0, 0, wWidth, wHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		current_shader->Use();
		current_shader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);

		current_shader->SetFloat("lightIntensity", light_intensity);
		current_shader->SetFloat("ambientIntensity", ambient_intensity);
		current_shader->SetFloat("specularIntensity", specular_intensity);
		current_shader->SetFloat("material.shininess", shininess);
		current_shader->SetVec3("light.diffuse", light_color[0], light_color[1], light_color[2]);
		current_shader->SetInt("renderShadows", render_shadows);

		light_direction = GetLightDirection(light_rotation[0], light_rotation[1]);
		current_shader->SetVec3("light.direction", light_direction);
		current_shader->SetVec3("lightDirection", light_direction);
		current_shader->SetVec3("wire_color", wire_color[0], wire_color[1], wire_color[2]);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, stone_floor_diffuse);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, stone_floor_roughness);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, empty_normal);

		if (render_plane)
		{
			glBindVertexArray(VAO);
			model = glm::mat4(1.0f);
			current_shader->SetMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuse_map);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, roughness_map);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, normal_map);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
		model = glm::translate(model, glm::vec3(asset_translation[0], asset_translation[1], asset_translation[2]));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(asset_rotation[0]), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(asset_rotation[1]), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(asset_rotation[2]), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(uniform_scale));
		current_shader->SetMat4("model", model);
		current_model->Draw(*current_shader);

		ImTextureRef ref_button_lit((ImTextureID)(intptr_t)lit_icon);
		ImTextureRef ref_button_wireframe((ImTextureID)(intptr_t)wireframe_icon);
		ImTextureRef ref_button_unlit((ImTextureID)(intptr_t)unlit_icon);
		ImVec4 wireframe_icon_tint = ImVec4(1.0f - background_color[0], 1.0f - background_color[1], 1.0f - background_color[2], 1);

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(wWidth - 425.0f, 60), ImGuiCond_Always);
			ImGui::Begin("##empty", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar
				| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
				| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
			if (ImGui::ImageButton("button_lit", ref_button_lit, ImVec2(32, 32)))
			{
				SetLitMode();
				current_shader = &shader;
			}
			if (ImGui::IsItemHovered())
			{
				cursor = ImGuiMouseCursor_Hand;
			}
			ImGui::SameLine();
			if (ImGui::ImageButton("button_wireframe", ref_button_wireframe, ImVec2(32, 32)))
			{
				SetWireframeMode();
				current_shader = &wireframeShader;
			}
			if (ImGui::IsItemHovered())
			{
				cursor = ImGuiMouseCursor_Hand;
			}
			ImGui::SameLine();
			if (ImGui::ImageButton("button_unlit", ref_button_unlit, ImVec2(32, 32)))
			{
				SetLitMode();
				current_shader = &unlitShader;
			}
			if (ImGui::IsItemHovered())
			{
				cursor = ImGuiMouseCursor_Hand;
			}

			ImGui::PopStyleColor(3);

			ImGui::End();
		}
		// ImGui window
		{
			float windowWidth = 425.0f;  // šíøka panelu
			float windowHeight = io.DisplaySize.y;  // výška = výška okna

			ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - windowWidth, 0), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Always);
			ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

			ImGui::Text("Textures");
			if (ImGui::InputText("Diffuse map path", diffuse_path_buffer, sizeof(diffuse_path_buffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				std::string newPath = diffuse_path_buffer;

				if (newPath != diffuse_map_path)
				{
					unsigned int newTexture = loadTexture(newPath.c_str(), true);
					if (newTexture != 0)
					{
						// uvolni starou texturu, pokud existuje
						if (diffuse_map != 0)
							glDeleteTextures(1, &diffuse_map);

						diffuse_map = newTexture;
						diffuse_map_path = newPath;
					}
				}
			}
			if (ImGui::InputText("Roughness map path", roughness_path_buffer, sizeof(roughness_path_buffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				std::string newPath = roughness_path_buffer;

				if (newPath != roughness_map_path)
				{
					unsigned int newTexture = loadTexture(newPath.c_str(), false);
					if (newTexture != 0)
					{
						if (roughness_map != 0)
							glDeleteTextures(1, &roughness_map);

						roughness_map = newTexture;
						roughness_map_path = newPath;
					}
				}
			}
			if (ImGui::InputText("Normal map path", normal_path_buffer, sizeof(normal_path_buffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				std::string newPath = normal_path_buffer;

				if (newPath != normal_map_path)
				{
					unsigned int newTexture = loadTexture(newPath.c_str(), false);
					if (newTexture != 0)
					{
						if (normal_map != 0)
							glDeleteTextures(1, &normal_map);

						normal_map = newTexture;
						normal_map_path = newPath;
					}
				}
			}

			ImGui::Text("");

			ImGui::Text("Light");
			ImGui::SliderFloat("Intensity", &light_intensity, 0.0f, 3.0f, "%.2f");
			ImGui::SliderFloat("Ambient intensity", &ambient_intensity, 0.0f, 3.0f, "%.2f");
			ImGui::SliderFloat("Specular intensity", &specular_intensity, 0.0f, 3.0f, "%.2f");
			ImGui::SliderFloat("Shininess", &shininess, 2.0f, 512.0f, "%.0f");
			ImGui::ColorEdit3("Color", &light_color[0]);
			ImGui::SliderFloat("Angle", &light_rotation[0], -90.0f, 90.0f, "%.2f");
			ImGui::SliderFloat("Rotation", &light_rotation[1], -180.0f, 180.0f, "%.2f");
			ImGui::SetCursorPosX(265);
			ImGui::Checkbox("Shadows", &render_shadows);

			ImGui::Text("");

			ImGui::Text("Asset");
			ImGui::DragFloat3("Translation", &asset_translation[0], 0.01f, -10.0f, 10.0f, "%.2f");
			ImGui::DragFloat3("Rotation", &asset_rotation[0], 0.25f, -180.0f, 180.0f, "%.2f");
			ImGui::DragFloat("Scale", &uniform_scale, 0.01f, 0.1f, 10.0f, "%.2f");

			ImGui::Text("");

			ImGui::Text("Editor");
			ImGui::ColorEdit3("Background color", &background_color[0]);
			ImGui::ColorEdit3("Wireframe mesh color", &wire_color[0]);
			ImGui::SetCursorPosX(265);
			ImGui::Checkbox("Render plane", &render_plane);


			// test
			ImGui::SetCursorPosY(wHeight - 25);
			ImGui::Text("@ 2025, Segrec Cegrec");

			ImGui::End();
		}

		ImGui::SetMouseCursor(cursor);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

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
	if (!imgui_keyboard_capture)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		// polygon mode
		if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_CULL_FACE);
		}
		if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_CULL_FACE);
		}

		// antialiasing
		if (key == GLFW_KEY_3 && action == GLFW_PRESS)
			glEnable(GL_MULTISAMPLE);
		if (key == GLFW_KEY_4 && action == GLFW_PRESS)
			glDisable(GL_MULTISAMPLE);
	}
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
	if (!imgui_mouse_capture)
		camera.ProcessMouseScroll(yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (!imgui_mouse_capture)
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
}

void proccess_input(GLFWwindow* window)
{
	
}

glm::vec3 GetLightDirection(float x, float y)
{
	x -= 90.0f;
	float yaw = glm::radians(y);
	float pitch = glm::radians(x);

	glm::vec3 direction;
	direction.x = cos(pitch) * cos(yaw);
	direction.y = sin(pitch);
	direction.z = cos(pitch) * sin(yaw);

	return glm::normalize(direction);
}

void SetLitMode()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
}

void SetWireframeMode()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_CULL_FACE);
}

unsigned int loadTexture(const char* path, bool gammaCorrection)
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	int dataFormat = GL_RGB;
	int internalFormat = GL_RGB;
	if (nrChannels == 1)
	{
		internalFormat = dataFormat = GL_RED;
	}
	else if (nrChannels == 3)
	{
		dataFormat = GL_RGB;
		internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
	}
	else if (nrChannels == 4)
	{
		dataFormat = GL_RGBA;
		internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
	}

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cerr << "Failed to load texture" << std::endl;
		return 0;
	}

	stbi_image_free(data);

	return texture;
}