#include <vector>
#include <iostream>
#include <iomanip>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../core/renderer/ShaderProgram.h"
#include "camera.h"
#include "../core/utils/ColladaParser.h"
#include "utils.hpp"
#include "../core/model/Mesh.h"
#include "Joint.h"
#include "../objects/Animator.h"
#include "PositionalLight.h"

int SCR_WIDTH = 800;
int SCR_HEIGHT = 700;
constexpr float fov = 80.0f;
glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)(SCR_HEIGHT), 0.1f, 20000.f);
struct GuiData
{
	bool setmanual = false;
	bool freeCamera = false;
	float angleX = 0;
	float angleY = 0;
	float angleZ = 0;
	glm::vec3 scaleJoints  { 1.0f };
	glm::vec3 scale{ 1.0f };
};

struct OpenGLBufferInfo
{
	unsigned int vao;
	unsigned int vbo;
	unsigned int ebo;
	unsigned int indexSize;
};

OpenGLBufferInfo CreateSkeletonLinesBuffers();
unsigned CreateSkeletonJointsBuffers();
OpenGLBufferInfo CreateWorldGrid(int slides,std::vector<float>& grid);
void processInput(GLFWwindow* window, Camera& camera, float elapsedTime, float velocity, ShaderProgram& skelProgram);
void FillInBindPoseTransforms(Joint* node, std::vector<glm::mat4>& inout_transforms);
void GetGlobalPositions(Joint* node, const glm::mat4& parentTransform, std::vector<glm::mat4>& transform);
void PrepareSkeletonLines(Joint* node, const glm::vec4& parent, std::vector<glm::mat4>& transforms, std::vector<glm::vec4>& points);
GLFWwindow* InitWindow(const char* tittle, int width, int height);
void BreathFirstSearchPrint(Joint* node, std::string identation);
void setupImGui(GLFWwindow*);
void startImGuiFrame();
void cleanUpImGui();
void renderImGui(GuiData& data);


int main()
{
	GLFWwindow* window = InitWindow("3D animation", SCR_WIDTH, SCR_HEIGHT);
	setupImGui(window);
	Camera camera(0.0, 400, 500, fov);
	glfwSetWindowUserPointer(window,&camera);

	PositionalLight light(glm::vec3(0.4f, 8.5f, 0.3));
	//skeleton shader
	ShaderProgram skelProgram("Shaders/skel_vert.sh", "Shaders/skel_frag.sh");
	//lines shader
	ShaderProgram linesProgram("Shaders/vertexLines.sh", "Shaders/fragmentLines.sh");
	//grid shader
	ShaderProgram gridProgram("Shaders/vertex_grid.sh", "Shaders/fragment_grid.sh");
	
	ColladaParser parser;
	Joint* root = parser.GetJointHerarchy("assets/air_flip.dae");
	BreathFirstSearchPrint(root, " ");
	std::vector<JointAnimation>animation = parser.GetAnimation("assets/air_flip.dae");
	std::vector<glm::mat4> transforms(animation.size(), glm::mat4(1.0f));

	Animator animator{ root,animation };
	unsigned VAO = CreateSkeletonJointsBuffers();

	// points to make lines between different joints
	std::vector<glm::vec4> points{ };
	OpenGLBufferInfo skelBuffLinesInfo = CreateSkeletonLinesBuffers();

	// create grid
	std::vector<float> grid = {};
	OpenGLBufferInfo gridBufferInfo = CreateWorldGrid(50,grid);

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	GuiData data;
	
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (!data.freeCamera)
			camera.CheckMouseMovement(*window, glm::vec3(0, 0, 0));
		else
			camera.CheckMouseMovement(*window);
		processInput(window, camera, 1, deltaTime, skelProgram);

		glm::mat4 cameraTranslation = camera.GetCameraTranslationMatrix();
		//transform joints
		glm::mat4 jointTransform(1.0f);
		jointTransform = glm::scale(glm::mat4(1.0f),data.scaleJoints);
		//transform model
		glm::mat4 p(1.0f);
		p = glm::rotate(glm::mat4(1.0f), data.angleX, glm::vec3(1, 0, 0));
		p = glm::rotate(p, data.angleY, glm::vec3(0, 1, 0));
		p = glm::rotate(p, data.angleZ, glm::vec3(0, 0, 1));
		p = glm::scale(p, data.scale);

		if (!data.setmanual)
			animator.Update(deltaTime);
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			animator.Update(deltaTime);

		transforms = animator.GetBoneTransforms();
		GetGlobalPositions(root, p, transforms);
		
		glClearColor(0.1, 0.1, 0.2, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//--------------------------------------------------------
		startImGuiFrame();

		//Draw grid
		glLineWidth(1);
		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(gridBufferInfo.vao);
		glm::mat4 mod(1.f);
		mod = glm::translate(mod, glm::vec3(-500, 0, -500));
		mod = glm::scale(mod, glm::vec3(1000, 1000, 1000.f));
		gridProgram.useProgram();
		gridProgram.setMatrix("proj", projectionMatrix);
		gridProgram.setMatrix("model", mod);
		gridProgram.setMatrix("cam", cameraTranslation);
		glDrawElements(GL_LINES, gridBufferInfo.indexSize, GL_UNSIGNED_INT, NULL);
		
		//Draw animated joints
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(VAO);
		skelProgram.useProgram();
		skelProgram.setMatrix("proj", projectionMatrix);
		skelProgram.setMatrix("jointTransform", jointTransform);
		skelProgram.setMatrix("cam", cameraTranslation);
		skelProgram.setVector3f("camera_pos", camera.GetCameraPosition());
		light.SetUniforms(skelProgram);
	
		for (int i = 0; i < transforms.size(); ++i)
		{
			skelProgram.setMatrix("animationTransform", transforms[i]);
		    glDrawArrays(GL_TRIANGLES,0, 36);
		}

		///Draw lines for the skeleton
		glLineWidth(3);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(skelBuffLinesInfo.vao);
		glBindBuffer(GL_ARRAY_BUFFER, skelBuffLinesInfo.vbo);
		linesProgram.useProgram();
		linesProgram.setMatrix("proj", projectionMatrix);
		linesProgram.setMatrix("cam", cameraTranslation);
		PrepareSkeletonLines(root, glm::vec4(0.0f,0.0f,0.0f,1.0f), transforms, points);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)* points.size(), &points[0], GL_DYNAMIC_DRAW);
		glDrawArrays(GL_LINES, 0, points.size());
		points.clear();
		glBindVertexArray(0);
		renderImGui(data);
		//---------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	cleanUpImGui();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}


void processInput(GLFWwindow* window, Camera& camera, float elapsedTime, float velocity, ShaderProgram& skelProgram)
{

	
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		velocity += 4.5f;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
	{
		if (velocity > 20.f)
			velocity -= 0.5f;
	}

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		camera.MoveFront(elapsedTime, velocity);
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		camera.MoveBack(elapsedTime, velocity);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		camera.MoveLeft(elapsedTime, velocity);
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		camera.MoveRight(elapsedTime, velocity);
	}

}

void GetGlobalPositions(Joint* node, const glm::mat4& parentTransform, std::vector<glm::mat4>&  transform)
{
	glm::mat4 currentLocalTransform = transform[node->ID];

	transform[node->ID] =  parentTransform * currentLocalTransform;

	for (Joint* child : node->Children)
	{
		GetGlobalPositions(child, transform[node->ID], transform);
	}
}

void FillInBindPoseTransforms(Joint* node, std::vector<glm::mat4>& inout_transforms)
{
	inout_transforms[node->ID] = node->localBindTransform;

	for (Joint* child : node->Children)
	{
		FillInBindPoseTransforms(child, inout_transforms);
	}
}

OpenGLBufferInfo CreateSkeletonLinesBuffers()
{
	OpenGLBufferInfo info = {};
	glGenBuffers(1, &info.vbo);
	glGenVertexArrays(1, &info.vao);
	glBindVertexArray(info.vao);
	glBindBuffer(GL_ARRAY_BUFFER, info.vbo);
	glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindVertexArray(0);
	
	return info;
}

GLFWwindow* InitWindow(const char* tittle, int width, int height)
{
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(width, height, tittle, nullptr, nullptr);
	glfwSetWindowUserPointer(window, window);
	if (!window) throw std::runtime_error("could not create window");
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glewExperimental = GL_TRUE;
	//init opengl context
	if (glewInit() != GLEW_OK) throw std::runtime_error("could not initalize openGl context");

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
		{
			glfwGetFramebufferSize(window, &width, &height);

			if (width > 0 && height > 0)
			{
				SCR_WIDTH = width;
				SCR_HEIGHT = height;
				projectionMatrix = glm::perspective(glm::radians(fov), (float)width / (float)(height), 0.5f, 1000.f);
				glViewport(0, 0, width, height);
			}
		});
	glfwSetScrollCallback(window,[](GLFWwindow * window, double xoffset, double yoffset)
	{
			Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));

			cam->Distance += (float)yoffset * 0.6f;

			if (cam->Distance < 1.0f)
				cam->Distance = 1.0f;

	});
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP);

	return window;
}

// create the points for the skeleton lines in a breath first search fashion
void PrepareSkeletonLines(Joint* node, const glm::vec4& parent, std::vector<glm::mat4>& transforms, std::vector<glm::vec4>& points)
{
	points.push_back(parent);
	points.push_back(transforms[node->ID] * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	for (Joint* child : node->Children)
	{
		PrepareSkeletonLines(child, transforms[node->ID] * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), transforms, points);
	}
}

void BreathFirstSearchPrint(Joint* node, std::string identation)
{
	std::cout << identation << node->Name << std::endl;

	for (int c = 0; c < 4; ++c)
	{
		std::cout << identation;
		for (int r = 0; r < 4; ++r)
		{
			std::cout << std::setprecision(5) << std::fixed << node->InverseTransform[c][r] << " ";
		}
		std::cout << "\n";
	}

	for (Joint* child : node->Children)
	{
		BreathFirstSearchPrint(child, identation += "   ");
	}
}


unsigned CreateSkeletonJointsBuffers()
{
	static float vertices[] = {
		//back
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,0.0f,-1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,0.0f,-1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,0.0f,-1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,0.0f,-1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,0.0f,-1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,0.0f,-1.0f,
		//front
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,0.0f,1.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f,0.0f,1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,0.0f,1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,0.0f,1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,0.0f,1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,0.0f,1.0f,
		//left
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,0.0f,0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  -1.0f,0.0f,0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   -1.0f,0.0f,0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   -1.0f,0.0f,0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   -1.0f,0.0f,0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   -1.0f,0.0f,0.0f,
		//right
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,0.0f,0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  1.0f,0.0f,0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f,0.0f,0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f,0.0f,0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f,0.0f,0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f,0.0f,0.0f,
		 //Bottom
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,-1.0f,0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,-1.0f,0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,-1.0f,0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,-1.0f,0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,-1.0f,0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,-1.0f,0.0f,
		// Up
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,1.0f,0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,1.0f,0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,1.0f,0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,1.0f,0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,1.0f,0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,1.0f,0.0f
	};

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);
	//unbind
	glBindVertexArray(0);
	return VAO;
}


void setupImGui(GLFWwindow* window)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

}


void startImGuiFrame()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}


void cleanUpImGui()
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}


void renderImGui(GuiData& data)
{

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("load animation"))
			{
				std::cout << "load animation" << std::endl;
			}
			if (ImGui::MenuItem("manual update"))
			{
				data.setmanual = !data.setmanual;
			}
			if (ImGui::MenuItem("free camera"))
			{
				data.freeCamera = true;
			}
			if (ImGui::MenuItem("lock camera"))
			{
				data.freeCamera = false;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	
	ImGui::SliderAngle("x rotate", &data.angleX);
	ImGui::SliderAngle("y rotate", &data.angleY);
	ImGui::SliderAngle("z rotate", &data.angleZ);
	ImGui::SliderFloat("scale model x", &data.scale.x, -0.5f, 20.0f, "ratio = %.01f");
	ImGui::SliderFloat("scale model y", &data.scale.y, -0.5f, 20.0f, "ratio = %.01f");
	ImGui::SliderFloat("scale model z", &data.scale.z, -0.5f, 20.0f, "ratio = %.01f");
	ImGui::Separator();
	ImGui::SliderFloat("scale x", &data.scaleJoints.x, 0.0f, 100.0f, "ratio = %.01f");
	ImGui::SliderFloat("scale y", &data.scaleJoints.y, 0.0f, 100.0f, "ratio = %.01f");
	ImGui::SliderFloat("scale z", &data.scaleJoints.z, 0.0f, 100.0f, "ratio = %.01f");

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	
	
}

OpenGLBufferInfo CreateWorldGrid(int slices, std::vector<float>& grid)
{
	for (int r = 0; r <= slices; ++r)
	{
		for (int c = 0; c <= slices; ++c)
		{
			float x = (float)r / slices;
			float y = 0;
			float z = (float)c / slices;
			grid.push_back(x);
			grid.push_back(y);
			grid.push_back(z);
		}
	}

	std::vector<glm::uvec4> indices;
	for (int j = 0; j < slices; ++j) 
	{
		for (int i = 0; i < slices; ++i) 
		{
			int row1 = j * (slices + 1);
			int row2 = (j + 1) * (slices + 1);

			indices.push_back(glm::uvec4(row1 + i, row1 + i + 1, row1 + i + 1, row2 + i + 1));
			indices.push_back(glm::uvec4(row2 + i + 1, row2 + i, row2 + i, row1 + i));
		}
	}

	OpenGLBufferInfo info = {};
	info.indexSize = indices.size()*4;
	glGenBuffers(1, &info.vbo);
	glGenBuffers(1, &info.ebo);
	glGenVertexArrays(1, &info.vao);
	glBindVertexArray(info.vao);
	glBindBuffer(GL_ARRAY_BUFFER, info.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * grid.size(), grid.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(*indices.data()), indices.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);

	
	return info;

}
