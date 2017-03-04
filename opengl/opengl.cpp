#include "stdafx.h"

#define _USE_MATH_DEFINES
#include <cmath>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <vector>
#include <string>
#include <chrono>

using namespace std;

std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());


GLuint compileShader(std::string& code, GLenum type);
GLuint createProgram();

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
};

struct Polygon {

	std::vector<Vertex> vertices = {
		{ { -0.25f,-0.25f,0.0f },{ 1.0f,0.f,0.f } },
		{ { 0.25f,-0.25f,0.0f },{ 0.0f,1.f,0.f } },
		{ { 0.0f,0.25f,0.0f },{ 0.0f,0.f,1.f } }
	};
	glm::vec3 translation{ 0.5f,-0.5f,0 };
	float rotation = 0;

	int type = GL_TRIANGLE_FAN;
	float radius = 0.0f;
	glm::vec2 vel{ 0.f, 0.f };
	float angularVelocity = 0;

	void createRegular(int verticesCount) {
		vertices.clear();
		vertices.push_back({ { 0,0,0 },randomColor() });
		radius = 0.1;
		for (size_t i = 0; i < verticesCount + 1; i++)
		{
			float angle = ((M_PI * 2) / (verticesCount))*i;
			vertices.push_back({ { radius*cos(angle),radius*sin(angle),0 },randomColor() });
		}
		vel = glm::vec2(0.3f, 0.3f);
		angularVelocity = 2.f;

	}

	static glm::vec3 randomColor() {
		std::uniform_real_distribution<float> distribution(0.0, 1.0);
		return{ distribution(generator),distribution(generator) ,distribution(generator) };
	}

	void readVerticesFormConsole() {
		std::cout << "iloœæ wierzcho³ków" << std::endl;
		int count = 0;
		cin >> count;
		std::cout << "podawaj wierzcho³ki w kolejnoœci przeciwnej do ruchu wskazówek zagara" << std::endl;

		vertices.clear();
		for (size_t i = 0; i < count; i++)
		{
			float xPos, yPos;
			std::cout << "wierzcho³ek:" << i << std::endl;;

			std::cout << "sk³adowa x" << std::endl;;
			cin >> xPos;
			std::cout << "sk³adowa y" << std::endl;;
			cin >> yPos;
			vertices.push_back({ { xPos,yPos,0.0f },randomColor() });
		}

		/*
		//przyk³adowy input
		vertices.push_back({ { -0.2f,-0.2f,0.0f },randomColor() });
		vertices.push_back({ { 0.2f,-0.2f,0.0f },randomColor() });
		vertices.push_back({ { 0.2f,0.1f,0.0f },randomColor() });
		vertices.push_back({ { 0.0f,0.2f,0.0f },randomColor() });
		vertices.push_back({ { -0.2f,0.2f,0.0f },randomColor() });
		*/


	}


}polygon;

struct Camera {
	glm::mat4 projection;
	//glm::mat4 view;

	float viewWidth = 0;
	float viewHeigth = 0;

	void updateProjection(GLsizei width, GLsizei heigth) {

		if (heigth == 0)
			heigth = 1;

		if (width <= heigth) {
			viewWidth = 1.0f;
			viewHeigth = heigth / (float)width;
		}
		else {
			viewWidth = width / (float)heigth;
			viewHeigth = 1.f;
		}

		projection = glm::ortho(0.f, viewWidth, -viewHeigth, 0.f, 0.0f, 10.f);

	}

} camera;




struct {

	void init() {
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*polygon.vertices.size(), polygon.vertices.data(), GL_STATIC_DRAW);
		glClearColor(0.6f, 0.4f, 0.12f, 1.0f);
		program = createProgram();

		uniformMatrix = glGetUniformLocation(program, "MVP");
	}


	GLuint vertexArray;
	GLuint vertexBuffer;

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint program;

	GLint uniformMatrix;

} renderer;

void draw() {
	glClear(GL_COLOR_BUFFER_BIT);


	glm::mat4 rotation = glm::rotate(glm::mat4(), polygon.rotation, glm::vec3(0.f, 0.f, 1.f));
	glm::mat4 translation = glm::translate(glm::mat4(), polygon.translation);

	auto mvp = camera.projection * translation* rotation;

	glUseProgram(renderer.program);
	glUniformMatrix4fv(renderer.uniformMatrix, 1, GL_FALSE, glm::value_ptr(mvp));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, renderer.vertexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

	glDrawArrays(polygon.type, 0, polygon.vertices.size());
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glutSwapBuffers();

}

void changeSize(GLsizei width, GLsizei heigth) {
	camera.updateProjection(width, heigth);
	glViewport(0, 0, width, heigth);
}


GLuint compileShader(std::string& code, GLenum type) {

	GLuint shader = glCreateShader(type);
	const char* c = code.c_str();
	glShaderSource(shader, 1, &c, NULL);
	glCompileShader(shader);

	GLint result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	int logInfoLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logInfoLength);
	if (logInfoLength > 0) {
		std::vector<char> errorMsg(logInfoLength + 1);
		glGetShaderInfoLog(shader, logInfoLength, NULL, errorMsg.data());
		std::cout << errorMsg.data() << std::endl;
	}
	return shader;

}


GLuint createProgram() {
	std::string vertexShaderCode =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vertexPosition;"
		"layout(location = 1) in vec3 inColor;\n"
		"uniform mat4 MVP;\n"
		"out vec3 fragColor;\n"
		"void main(){\n"
		"fragColor = inColor;\n"
		"gl_Position =MVP* vec4(vertexPosition,1);"
		"gl_PointSize = 4.0;"
		"}";

	std::string fragmentShaderCode =
		"#version 330 core\n"
		"out vec3 color;"
		"in vec3 fragColor;"
		"void main(){"
		"color = fragColor;"
		"}";
	GLuint vertex = compileShader(vertexShaderCode, GL_VERTEX_SHADER);
	GLuint fragment = compileShader(fragmentShaderCode, GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);

	GLint result;
	glGetShaderiv(program, GL_LINK_STATUS, &result);
	int logInfoLength;
	glGetShaderiv(program, GL_INFO_LOG_LENGTH, &logInfoLength);
	if (logInfoLength > 0) {
		std::vector<char> errorMsg(logInfoLength + 1);
		glGetShaderInfoLog(program, logInfoLength, NULL, errorMsg.data());
		std::cout << errorMsg.data() << std::endl;
	}

	glDetachShader(program, vertex);
	glDetachShader(program, fragment);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	return program;

}


void update(int n) {

	glm::vec2& vel = polygon.vel;
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();

	auto time = static_cast<std::chrono::duration<float>>(currentTime - startTime).count();
	startTime = std::chrono::high_resolution_clock::now();


	if (polygon.translation.y + polygon.radius + (vel.y*time) >= 0) {
		polygon.translation.y = -polygon.radius;

		vel.y = -vel.y;
	}
	else if (polygon.translation.y - polygon.radius + (vel.y*time) <= -camera.viewHeigth) {
		polygon.translation.y = -camera.viewHeigth + polygon.radius;
		vel.y = -vel.y;
	}

	if (polygon.translation.x - polygon.radius + (vel.x*time) <= 0) {
		polygon.translation.x = polygon.radius;
		vel.x = -vel.x;
	}
	else if (polygon.translation.x + polygon.radius + (vel.x*time) >= camera.viewWidth) {
		polygon.translation.x = camera.viewWidth - polygon.radius;
		vel.x = -vel.x;
	}

	polygon.translation.x += vel.x* time;
	polygon.translation.y += vel.y* time;
	polygon.rotation += polygon.angularVelocity*time;


	glutTimerFunc(16, update, 0);
	glutPostRedisplay();

}



int main(int argc, char* argv[]) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutSetOption(GLUT_MULTISAMPLE, 4);

	glutInitWindowSize(800, 600);


	std::cout << "1: odbijaj¹cy siê oœmiok¹t" << std::endl;
	std::cout << "2: wpisywanie wierzcho³ków" << std::endl;
	int answer;
	cin >> answer;

	if (answer == 1) {
		polygon.createRegular(8);
	}
	else if (answer == 2) {
		polygon.readVerticesFormConsole();
	}



	glutCreateWindow("opengl");

	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);



	renderer.init();
	glutDisplayFunc(draw);
	glutReshapeFunc(changeSize);

	glutTimerFunc(16, update, 100);
	glutMainLoop();
	return 0;
}
