#pragma once

class Object
{
	GLuint vbo[2];
	GLuint ebo;
	GLuint vao;

	vector<glm::vec3> v; // 정점 정보 + 바운딩 박스 정보, size - 8 해서 원래 객체의 정점만 가져오기
	vector<glm::vec3> c; // 색 정보 
	vector<glm::vec3> n; // 노말 값
	vector<glm::vec3> uv; // 텍스쳐 uv
	vector<unsigned int> i; // 인덱스 정보, size - 

	glm::mat4 matrix = glm::mat4(1.0f); // 전체 행렬 녀석
	glm::vec3 S{ 1.0f,1.0f,1.0f }; // 크기
	glm::vec3 R{ 0.0f,0.0f,0.0f }; // 회전 ([0] - x, [1] - y, [2] - z) 축으로 얼마나 회전 할지
	glm::vec3 T{ 0.0f,0.0f,0.0f }; // 이동

	glm::vec3 origin{ 0.0f, 0.0f, 0.0f }; // 물체가 기본도형 여러 개 붙은 녀석일 때, 부속 도형들의 원점을 정해주는 것 
	glm::vec3 rotBy{ 0.0f, 0.0f, 0.0f }; // 공전 등을 구현할 때 한 점을 기준으로 회전하는 것
	glm::vec3 rotByAngle{ 0.0f, 0.0f, 0.0f }; // rotTo 기준 회전 각도

public:
	// obb에서 사용하는 녀석들 // 
	glm::vec3 vCenterPos = { 0.0f, 0.0f, 0.0f }; // 상자 중앙의 좌표
	glm::vec3 vAxisDir[3] = { {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} }; //상자에 평행한 세 축의 단위벡터
	float fAxisLen[3]; // 상자의 평행한 세 축의 길이 fAxisLen[n]은 vAxisDir[n]에 각각 대응한다.
	void UpdateBB();
	// // // // // // // // //

	bool isSelected = false;
	int id = 0;

	// 여기부터
	Object(const char*, int, glm::vec3, glm::vec3, glm::vec3);
	Object() {}
	~Object() {}
	void InitBuffer(); // 생성자 안에 무조건 넣습니다
	void Remove();
	void Render(GLuint shaderProgramID, GLenum eMode);
	void Readobj(const char* s); // 파일 읽는 녀석
	void UpdateMatrix(); // render 전에 행렬변환 수정하는 녀석, update 함수 안에 넣어요
	virtual void Update(); // 물체별로 함수 다르게 실행시키기 위해 virtual로 뺐음 굳이 이긴 해요
	// 여기까지는 건드리지 말도록

	void Move(int n, float value) { T[n] += value; origin[n] += value; } // T[n]의 값을 value 만큼 변경
	void Rot(int n, float value) { R[n] += value; } // 객체 자체의 회전, R[n]의 값을 value 만큼 변경
	void RotByPoint(int n, bool b, glm::vec3 o);
	void SetRot(int n, float value) { R[n] = value; } // 객체 자체의 회전을 주어진 값 을 변경
	void SetMove(int n, float value) { T[n] = value; }
	glm::vec3 GetRot() { return R; }
};
