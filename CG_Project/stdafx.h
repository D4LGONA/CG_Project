#pragma once

#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#define STB_IMAGE_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <random>
#include <vector>

#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

using namespace std;

#define WIDTH 800
#define HEIGHT 800

extern std::random_device rd;
extern std::mt19937 dre;
extern std::uniform_real_distribution<float> uidC;
extern glm::mat4 proj;
extern glm::vec3 cameraPos;
extern glm::vec3 cameraDirection;
extern glm::vec3 cameraUp;
extern glm::mat4 view;
extern glm::vec3 cameraAngle;
extern glm::vec3 cameraAngle_self;
extern glm::vec3 lightPos;
extern glm::vec3 lightColor;
extern bool light_onf;
extern float light_hardness;

float dist(glm::vec3 p1, glm::vec3 p2);
bool obb(class Object& a, class Object& b);
bool obb_ray(class Object& a, glm::vec3 origin, glm::vec3 direction);
