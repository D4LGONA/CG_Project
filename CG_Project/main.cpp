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
float Ypos = -20.0f;

bool R_mode = false;
bool M_mode = true;
bool Edit_mode = true;
Object* target = nullptr;

void howto()
{

}

bool Save(string filepath)
{
	ofstream out{ "resources/" + filepath};

	if (out.is_open()) 
	{
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
	objs.clear();

	std::ifstream in("resources/" + filepath);
	if (in.is_open()) {
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
	int tx, ty;
	cout << "�� ũ��: ";
	cin >> tx >> ty;
	stage = new Object("resources/plane.obj", shaderProgramID, { tx, 0.5f, ty}, { 0.0f, 0.0f, 0.0f }, { 0.0f, -20.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 0);
	stage->InitTexture("resources/stage_base.png");
	cout << "�ʱ� ���� - ������Ʈ �̵� ���." << endl;
	
	cameraPos = glm::vec3(0.0f, 0.0f, 120.0f); //--- ī�޶� ��ġ
	cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- ī�޶� �ٶ󺸴� ����
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- ī�޶� ���� ����
	view = glm::mat4(1.0f);
	cameraAngle = { 0.0f, 0.0f, 0.0f };

	proj = glm::mat4(1.0f);
	proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 200.0f); //--- ���� ���� ����: fovy, aspect, near, far
	proj = glm::translate(proj, glm::vec3(0.0, 0.0, 30.0f));

	view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	view = glm::rotate(view, glm::radians(cameraAngle.z), { 0.0f, 0.0f, 1.0f });
	view = glm::rotate(view, glm::radians(cameraAngle.y), { 0.0f, 1.0f, 0.0f });
	view = glm::rotate(view, glm::radians(cameraAngle.x), { 1.0f, 0.0f, 0.0f });
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
			objs.push_back(new Object("resources/brick.obj", shaderProgramID, { 5.0f, 5.0f, 5.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 2));
			objs.back()->InitTexture("resources/brick_base.png");
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


		case 'r':
		case 'R':
			cout << "������Ʈ ȸ�� ���� ��ȯ�մϴ�." << endl;
			R_mode = true;
			M_mode = false;
			break;

		case 'm':
		case 'M':
			cout << "������Ʈ �̵� ���� ��ȯ�մϴ�." << endl;
			R_mode = false;
			M_mode = true;
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
				target = o;
			else
				target = nullptr;
			drag = true;
		}
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		//drag = false;
	}
	return GLvoid();
}

GLvoid Motion(int x, int y)
{
	if (M_mode)
	{
		if (target != nullptr)
		{
			glm::vec3 ray_origin = glm::unProject(glm::vec3(x, HEIGHT - y, 0.0f), view, proj, glm::vec4(0, 0, WIDTH, HEIGHT));

			glm::vec3 ray_direction = glm::normalize(glm::unProject(glm::vec3(x, HEIGHT - y, 1.0f), view, proj, glm::vec4(0, 0, WIDTH, HEIGHT)) - ray_origin);

			auto intersection = rayXZPlaneIntersection(ray_origin, ray_direction).second;

			target->SetMove(0, intersection.x);
			target->SetMove(1, intersection.y + target->GetS().y / 2.0f);
			target->SetMove(2, intersection.z);
		}
	}

	mousept.x = x;
	mousept.y = y;
	glutPostRedisplay();
}

GLvoid DragMotion(int x, int y)
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
		
			cameraAngle.x += deltaY;
			cameraAngle.y += deltaX;

		}
	}
	else
	{
		if (target != nullptr)
		{
			int deltaX = x - mousept.x;
			int deltaY = y - mousept.y;

			target->Scale(0, deltaX / 10.0f);
			target->Scale(1, -deltaY / 10.0f);
		}
		else
		{
			int deltaX = x - mousept.x;
			int deltaY = y - mousept.y;

			cameraPos.x += -deltaX / 10.;
			cameraPos.y += deltaY / 10.;

			cameraDirection.x += -deltaX / 10.;
			cameraDirection.y += deltaY / 10.;
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