#pragma once

class Object
{
	GLuint ShaderProgram;
	unsigned int texture = -1;

	GLuint vbo;
	GLuint vao;

	vector<glm::vec3> v; // ���� �븻�� �ؽ�ó �� ������ ��� ����
	glm::vec4 objColor = { 0.0f, 0.0f, 0.0f, 1.0f }; // RGBA

	glm::mat4 matrix = glm::mat4(1.0f); // ��ü ��� �༮
	glm::vec3 S{ 1.0f,1.0f,1.0f }; // ũ��
	glm::vec3 R{ 0.0f,0.0f,0.0f }; // ȸ�� ([0] - x, [1] - y, [2] - z) ������ �󸶳� ȸ�� ����
	glm::vec3 T{ 0.0f,0.0f,0.0f }; // �̵�

	glm::vec3 origin{ 0.0f, 0.0f, 0.0f }; // ��ü�� �⺻���� ���� �� ���� �༮�� ��, �μ� �������� ������ �����ִ� �� 
	glm::vec3 rotBy{ 0.0f, 0.0f, 0.0f }; // ���� ���� ������ �� �� ���� �������� ȸ���ϴ� ��
	glm::vec3 rotByAngle{ 0.0f, 0.0f, 0.0f }; // rotTo ���� ȸ�� ����

public:

	// obb���� ����ϴ� �༮�� // 
	glm::vec3 vCenterPos = { 0.0f, 0.0f, 0.0f }; // ���� �߾��� ��ǥ
	glm::vec3 vAxisDir[3] = { {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} }; //���ڿ� ������ �� ���� ��������
	float fAxisLen[3]; // ������ ������ �� ���� ���� fAxisLen[n]�� vAxisDir[n]�� ���� �����Ѵ�.
	void UpdateBB();
	// // // // // // // // //

	// ��ŷ //

	
	// �������
	Object(const char*, GLuint, glm::vec3, glm::vec3, glm::vec3, glm::vec4);
	Object() {}
	~Object() {}
	void InitBuffer(); // ������ �ȿ� ������ �ֽ��ϴ�
	void Remove();
	void Render();
	void Readobj(const char* s); // ���� �д� �༮
	void UpdateMatrix(); // render ���� ��ĺ�ȯ �����ϴ� �༮, update �Լ� �ȿ� �־��
	virtual void Update(); // ��ü���� �Լ� �ٸ��� �����Ű�� ���� virtual�� ���� ���� �̱� �ؿ�
	void InitTexture(const char*);
	// ��������� �ǵ帮�� ������

	void Move(int n, float value) { T[n] += value; origin[n] += value; } // T[n]�� ���� value ��ŭ ����
	void Rot(int n, float value) { R[n] += value; } // ��ü ��ü�� ȸ��, R[n]�� ���� value ��ŭ ����
	void RotByPoint(int n, bool b, glm::vec3 o);
	void SetRot(int n, float value) { R[n] = value; } // ��ü ��ü�� ȸ���� �־��� �� �� ����
	void SetMove(int n, float value) { T[n] = value; }
	glm::vec3 GetRot() { return R; }
};
