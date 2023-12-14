#include "stdafx.h"
#include "object.h"

GLchar* vertexSource, * fragmentSource; //--- �ҽ��ڵ� ���� ����
GLuint vertexShader, fragmentShader; //--- ���̴� ��ü
GLuint shaderProgramID; //--- ���̴� ���α׷�

uniform_real_distribution<float> uidp{-20.0f, 20.0f};

void make_shaderProgram();
void make_vertexShaders();
void make_fragmentShaders();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboarddown(unsigned char key, int x, int y);
GLvoid Keyboardup(unsigned char key, int x, int y);
GLvoid TimerFunction(int value);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Motion(int x, int y);
GLvoid DragMotion(int x, int y);
char* filetobuf(const char*);

POINT mousept;
Object* stage;
vector<Object*> objs;

bool drag = false;
float Ypos = -5.0f;

bool R_mode = false;
bool M_mode = true;
bool S_mode = false;
bool Edit_mode = true;
Object* target = nullptr;
float dist_camera = 150.0f;

float yaw = 0.0f;
float pitch = 0.0f;

bool compareObjects(Object*& a, Object*& b) 
{
	// ����� ���
	glm::mat4 inverseViewMatrix = glm::inverse(view);

	// ȸ�� ����� cameraPos�� ����
	glm::vec4 rotatedCameraPos = inverseViewMatrix * glm::vec4(cameraPos, 1.0f);
	glm::vec4 cP = rotatedCameraPos;

	float distanceA = glm::length(glm::vec3(a->GetT().x - cP[0], a->GetT().y - cP[1], a->GetT().z - cP[2]));
	float distanceB = glm::length(glm::vec3(b->GetT().x - cP[0], b->GetT().y - cP[1], b->GetT().z - cP[2]));

	// ����� ������ ����
	return distanceA < distanceB;
}

bool Save(string filepath)
{
	ofstream out{ "resources/" + filepath};

	if (out.is_open()) 
	{
		if (stage != nullptr)
			out << *stage << endl;
		for (Object*& obj : objs) 
		{
			out << *obj << std::endl;
		}
		out.close();
		std::cout << "�ش� ��ο� ����Ǿ����ϴ�: " << "resources/" + filepath << std::endl;
	}
	else 
	{
		std::cerr << "������ ã�� �� �����ϴ�: " << "resources/" + filepath << std::endl;
	}

	return true;
}

bool Load(string filepath)
{
	std::ifstream in("resources/" + filepath);
	if (in.is_open()) {
		objs.clear();
		delete stage;
		stage = nullptr;

		Object t{};
		while (in >> t) 
		{
			objs.push_back(new Object(shaderProgramID, t.GetS(), t.GetRot(), t.GetT(), { 1.0f, 1.0f, 1.0f, 1.0f }, t.shape));
		}
		in.close();
		std::cout << "�ش� ����� ������ �ҷ��Խ��ϴ�: " << "resources/" + filepath << std::endl;
	}
	else {
		std::cerr << "������ ã�� �� �����ϴ�: " << "resources/" + filepath << std::endl;
	}

	return true;

}

pair<bool, glm::vec3> rayXZPlaneIntersection(glm::vec3 rayStart, glm::vec3 rayDirection) 
{
	pair<bool, glm::vec3> intersection;

	if (rayDirection.y == 0) {
		intersection.first = false;
		return intersection;
	}

	float t = (-1 * (rayStart.y + -Ypos)) / rayDirection.y;

	if (t < 0) {
		intersection.first = false;
		return intersection;
	}

	intersection.first = true;
	intersection.second = rayStart + t * rayDirection;

	return intersection;
}

void Reset()
{
	stage = new Object("resources/brick.obj", shaderProgramID, { 5.0f, 5.0f, 5.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -2.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 1);
	stage->InitTexture("resources/brick_base.png");
	cout << "�ʱ� ���� - ������Ʈ �̵� ���. ����� �׷��� ��ġ�� ĳ������ ���� ��ġ�Դϴ�." << endl;
	
	cameraPos = glm::vec3(0.0f, 0.0f, 100.0f); //--- ī�޶� ��ġ
	cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f); //--- ī�޶� �ٶ󺸴� ����
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- ī�޶� ���� ����
	view = glm::mat4(1.0f);
	cameraAngle = { 0.0f, 0.0f, 0.0f };

	proj = glm::mat4(1.0f);
	proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 120.0f); //--- ���� ���� ����: fovy, aspect, near, far
	proj = glm::translate(proj, glm::vec3(0.0, 0.0, 30.0f));

	view = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp);
	view = glm::rotate(view, glm::radians(cameraAngle.z), { 0.0f, 0.0f, 1.0f });
	view = glm::rotate(view, glm::radians(cameraAngle.y), { 0.0f, 1.0f, 0.0f });
	view = glm::rotate(view, glm::radians(cameraAngle.x), { 1.0f, 0.0f, 0.0f });

	//Load("w");
}

void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Example1");
	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	glewInit();
	make_shaderProgram();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutMotionFunc(DragMotion);
	glutPassiveMotionFunc(Motion);
	glutKeyboardFunc(Keyboarddown);
	glutKeyboardUpFunc(Keyboardup);
	Reset();
	glutTimerFunc(50, TimerFunction, 1);
	glutMainLoop();
}

GLvoid drawScene()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);
	
	glEnable(GL_DEPTH_TEST); 


	if (stage != nullptr)
		stage->Render();

	for (Object*& o : objs)
		o->Render();

	glutSwapBuffers(); //--- ȭ�鿡 ����ϱ�
}
//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboarddown(unsigned char key, int x, int y)
{

	if (Edit_mode) // �� ���� ���� ����.
	{
		switch (key)
		{
		case '1':
			cout << "1�� ��ü: ���� ���Դϴ�." << endl;
			objs.push_back(new Object("resources/brick.obj", shaderProgramID, { 5.0f, 5.0f, 5.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, {1.0f, 1.0f, 1.0f, 1.0f}, 1));
			objs.back()->InitTexture("resources/brick_base.png");
			target = objs.back();
			break;

		case '2':
			cout << "2�� ��ü: ����ǥ ���Դϴ�." << endl;
			objs.push_back(new Object("resources/brick2.obj", shaderProgramID, { 5.0f, 5.0f, 5.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 2));
			objs.back()->InitTexture("resources/brick2_base.png");
			target = objs.back();
			break;

		case '3':
			cout << "3�� ��ü: ���Դϴ�." << endl;
			objs.push_back(new Object("resources/star.obj", shaderProgramID, { 5.0f, 5.0f, 5.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 3));
			objs.back()->InitTexture("resources/star_base.png");
			target = objs.back();
			break;

		case '4':
			cout << "4�� ��ü: �����Դϴ�." << endl;
			objs.push_back(new Object("resources/coin.obj", shaderProgramID, { 5.0f, 5.0f, 5.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 3));
			objs.back()->InitTexture("resources/coin_base.png");
			target = objs.back();
			break;

		case '5':
			cout << "5�� ��ü: �������Դϴ�." << endl;
			objs.push_back(new Object("resources/pipe.obj", shaderProgramID, { 5.0f, 5.0f, 5.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 3));
			objs.back()->InitTexture("resources/pipe_base.png");
			target = objs.back();
			break;

		case '0':
		{
			system("cls");
			cout << "�ùķ��̼��� �����մϴ�." << endl;
			Edit_mode = false;

			R_mode = false;
			M_mode = false;
			S_mode = false;

			cameraPos = glm::vec3(0.0f, 10.0f, 0.0f); //--- ī�޶� ��ġ
			cameraDirection = glm::vec3(0.0f, 0.0f, 1.0f); //--- ī�޶� �ٶ󺸴� ����
			cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- ī�޶� ���� ����
			view = glm::mat4(1.0f);
			cameraAngle = { 0.0f, 0.0f, 0.0f };

			view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
			view = glm::rotate(view, glm::radians(cameraAngle.z), { 0.0f, 0.0f, 1.0f });
			view = glm::rotate(view, glm::radians(cameraAngle.y), { 0.0f, 1.0f, 0.0f });
			view = glm::rotate(view, glm::radians(cameraAngle.x), { 1.0f, 0.0f, 0.0f });
			break;
		}

		case 'r':
		case 'R':
			cout << "������Ʈ ȸ�� ���� ��ȯ�մϴ�." << endl;
			R_mode = true;
			M_mode = false;
			S_mode = false;
			break;
			
		case 'm':
		case 'M':
			cout << "������Ʈ �̵� ���� ��ȯ�մϴ�." << endl;
			R_mode = false;
			M_mode = true;
			S_mode = false;
			break;

		case 'c':
		case 'C':
			cout << "������Ʈ ũ�� ���� ���� ��ȯ�մϴ�." << endl;
			R_mode = false;
			M_mode = false;
			S_mode = true;
			break;

		case '+':
			Ypos += 5.0f;
			cout << "���� y�� ����: " << Ypos << endl;
			break;

		case '-':
			Ypos -= 5.0f;
			if (Ypos < -20.0f)
			{
				cout << "�ּ� �����Դϴ�." << endl;
				Ypos = -20.0f;
			}
			cout << "���� y�� ����: " << Ypos << endl;
			break;

		case 's':
		case 'S': // ���̺�
		{
			string path;
			cout << "������ ������ �̸��� �Է��ϼ���: ";
			cin >> path;

			Save(path);

			break;
		}

		case 'l':
		case 'L': // �ε�
		{
			string path;
			cout << "�ҷ��� ������ ��θ� �Է��ϼ���: ";
			cin >> path;

			Load(path);

			break;
		}

		case 'q': // ���α׷� ����
			exit(0);
			break;
		}
	}
	else // �÷��� ��� ����.
	{
		switch (key)
		{
		case 'w':
			cameraPos += 2.0f * cameraDirection;

			break;

		case 'a':
			cameraPos -= glm::normalize(glm::cross(cameraDirection, cameraUp)) * 2.0f;
			break;

		case 's':
			cameraPos -= 2.0f * cameraDirection;
			break;

		case 'd':
			cameraPos += glm::normalize(glm::cross(cameraDirection, cameraUp)) * 2.0f;
			break;

		case 'q': // ���α׷� ����
			exit(0);
			break;
		}
	}
	
	glutPostRedisplay();
}

GLvoid Keyboardup(unsigned char key, int x, int y)
{
	switch (key)
	{
	}
	glutPostRedisplay();
}

GLvoid TimerFunction(int value)
{

	if (stage != nullptr)
		stage->Update();
	for (Object*& o : objs)
		o->Update();

	glutPostRedisplay();
	glutTimerFunc(50, TimerFunction, 1);
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if(Edit_mode)
		{
			if (target != nullptr)
			{
				target = nullptr;
				return;
			}

			glm::vec3 ray_origin = glm::unProject(glm::vec3(x, HEIGHT - y, 0.0f), view, proj, glm::vec4(0, 0, WIDTH, HEIGHT));
			glm::vec3 ray_direction = glm::normalize(glm::unProject(glm::vec3(x, HEIGHT - y, 1.0f), view, proj, glm::vec4(0, 0, WIDTH, HEIGHT)) - ray_origin);

			// Ray Picking ����
			for (Object*& o : objs)
			{
				if (obb_ray(*o, ray_origin, ray_direction))
				{
					target = o;
					break;
				}
				else
					target = nullptr;
			}
		}


	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		if(Edit_mode)
		{
			glm::vec3 ray_origin = glm::unProject(glm::vec3(x, HEIGHT - y, 0.0f), view, proj, glm::vec4(0, 0, WIDTH, HEIGHT));
			glm::vec3 ray_direction = glm::normalize(glm::unProject(glm::vec3(x, HEIGHT - y, 1.0f), view, proj, glm::vec4(0, 0, WIDTH, HEIGHT)) - ray_origin);

			// Ray Picking ����
			auto it = std::remove_if(objs.begin(), objs.end(), [&](Object* obj) {
				return obb_ray(*obj, ray_origin, ray_direction);
				});

			// erase�� ����Ͽ� ����
			objs.erase(it, objs.end());
		}
	}
	return GLvoid();
}

GLvoid Motion(int x, int y)
{
	if (Edit_mode)
	{
		if (M_mode)
		{
			if (target != nullptr)
			{
				glm::vec3 ray_origin = glm::unProject(glm::vec3(x, HEIGHT - y, 0.0f), view, proj, glm::vec4(0, 0, WIDTH, HEIGHT));

				glm::vec3 ray_direction = glm::normalize(glm::unProject(glm::vec3(x, HEIGHT - y, 1.0f), view, proj, glm::vec4(0, 0, WIDTH, HEIGHT)) - ray_origin);

				pair<bool, glm::vec3> intersection = rayXZPlaneIntersection(ray_origin, ray_direction);
			

				if (intersection.first)
				{
					target->SetMove(0, intersection.second.x);
					target->SetMove(1, intersection.second.y + target->GetS().y / 2.0f);
					target->SetMove(2, intersection.second.z);
				}
			}
		}
		mousept.x = x;
		mousept.y = y;
	}
	else
	{
		float xoffset = mousept.x - x;
		float yoffset = y - mousept.y;

		float sensitivity = 0.05;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw -= xoffset;
		pitch -= yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraDirection = glm::normalize(front);

		glutWarpPointer(400, 400);
		mousept.x = 400;
		mousept.y = 400;

	}

	
	glutPostRedisplay();
}

GLvoid DragMotion(int x, int y)
{
	if(Edit_mode)
	{
		if (R_mode)
		{
			if (target != nullptr)
			{
				int deltaX = x - mousept.x;
				int deltaY = y - mousept.y;

				target->Rot(0, deltaY);
				target->Rot(1, deltaX);
			}
			else
			{
				int deltaX = x - mousept.x;
				int deltaY = y - mousept.y;

				cameraAngle.x += deltaY / 2.0f;
				cameraAngle.y += deltaX / 2.0f;

				sort(objs.begin(), objs.end(), compareObjects);
			}
		}
		else if (S_mode)
		{
			if (target != nullptr)
			{
				int deltaX = x - mousept.x;
				int deltaY = y - mousept.y;

				target->Scale(0, deltaX / 10.0f);
				target->Scale(1, -deltaY / 10.0f);
			}
		}
		else
		{
			if (target != nullptr)
			{
			}
			else
			{
				int deltaX = x - mousept.x;
				int deltaY = y - mousept.y;

				cameraPos.x += -deltaX / 10.;
				cameraPos.y += deltaY / 10.;

				/*cameraDirection.x += -deltaX / 10.;
				cameraDirection.y += deltaY / 10.;*/

				sort(objs.begin(), objs.end(), compareObjects);
			}
		}
	}
	
	mousept.x = x;
	mousept.y = y;
	glutPostRedisplay();
}

void make_shaderProgram()
{
	make_vertexShaders(); //--- ���ؽ� ���̴� �����
	make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
	//-- shader Program
	shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);
	glLinkProgram(shaderProgramID);
	//--- ���̴� �����ϱ�
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	//--- Shader Program ����ϱ�
	glUseProgram(shaderProgramID);
}

void make_vertexShaders()
{
	vertexSource = filetobuf("vertex.glsl");
	//--- ���ؽ� ���̴� ��ü �����
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
	//--- ���ؽ� ���̴� �������ϱ�
	glCompileShader(vertexShader);
	//--- �������� ����� ���� ���� ���: ���� üũ
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cout << "ERROR: vertex shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}

void make_fragmentShaders()
{
	fragmentSource = filetobuf("fragment.glsl");
	//--- �����׸�Ʈ ���̴� ��ü �����
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
	//--- �����׸�Ʈ ���̴� ������
	glCompileShader(fragmentShader);
	//--- �������� ����� ���� ���� ���: ������ ���� üũ
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cout << "ERROR: fragment shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading 
	if (!fptr) // Return NULL on failure 
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file 
	length = ftell(fptr); // Find out how many bytes into the file we are 
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator 
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file 
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer 
	fclose(fptr); // Close the file 
	buf[length] = 0; // Null terminator 
	return buf; // Return the buffer 
}