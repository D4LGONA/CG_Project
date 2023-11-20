#include "stdafx.h"
#include "Object.h"

void Object::UpdateBB()
{
	// centerpos 업데이트
	vCenterPos = glm::vec3(matrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	vAxisDir[0] = { 0.5f, 0.0f, 0.0f };
	vAxisDir[1] = { 0.0f, 0.5f, 0.0f };
	vAxisDir[2] = { 0.0f, 0.0f, 0.5f };

	// 각 방향벡터 업데이트
	for (int i = 0; i < 3; ++i)
	{
		vAxisDir[i] = glm::vec3(matrix * glm::vec4(vAxisDir[i], 1.0f)) - vCenterPos;
		fAxisLen[i] = glm::length(vAxisDir[i]);
		if (abs(fAxisLen[i]) < FLT_EPSILON)
			vAxisDir[i] = { 0.0f, 0.0f, 0.0f };
		else
			vAxisDir[i] /= fAxisLen[i];
	}
}

Object::Object(const char* c, int id, glm::vec3 s = { 1.0f, 1.0f, 1.0f }, glm::vec3 r = { 0.0f, 0.0f, 0.0f }, glm::vec3 t = { 0.0f, 0.0f, 0.0f })
	: id{id}, S{s}, R{r}, T{t}
{
	Readobj(c);
}

void Object::Update()
{
	UpdateMatrix();

	vCenterPos = T + origin;
	vAxisDir[0] = T + origin + glm::vec3{ 1.0f, 0.0f, 0.0f };
	vAxisDir[1] = T + origin + glm::vec3{ 0.0f, 1.0f, 0.0f };
	vAxisDir[2] = T + origin + glm::vec3{ 0.0f, 0.0f, 1.0f };
	UpdateBB();
}

void Object::Render(GLuint shaderProgramID, GLenum eMode)
{
	if(eMode == GL_SELECT)
		glLoadName(id);
	glBindVertexArray(vao);

	// Location 번호 저장
	unsigned int PosLocation = glGetAttribLocation(shaderProgramID, "in_Position"); //	: 0  Shader의 'layout (location = 0)' 부분
	unsigned int ColorLocation = glGetAttribLocation(shaderProgramID, "in_Color"); //	: 1
	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "transform");
	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "view");
	unsigned int projLocation = glGetUniformLocation(shaderProgramID, "projection");
	unsigned int selected = glGetUniformLocation(shaderProgramID, "isSelected");

	glEnableVertexAttribArray(PosLocation); // Enable 필수! 사용하겠단 의미
	glEnableVertexAttribArray(ColorLocation);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // 정점 좌표용 VBO 바인딩
	glVertexAttribPointer(PosLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // 색상 데이터용 VBO 바인딩
	glVertexAttribPointer(ColorLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(matrix));
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &proj[0][0]);
	glUniform1i(selected, isSelected);

	// 여기서 그리기
	if(eMode == GL_SELECT)
		glDrawElements(GL_TRIANGLES, i.size() - 36, GL_UNSIGNED_INT, 0);
	else
	{
		glDrawElements(GL_TRIANGLES, i.size() - 36, GL_UNSIGNED_INT, 0);
		glDrawElements(GL_LINES, 36, GL_UNSIGNED_INT, (void*)((i.size() - 36) * sizeof(unsigned int)));
	}

	glDisableVertexAttribArray(PosLocation); // Disable 필수!
	glDisableVertexAttribArray(ColorLocation);
}

void Object::Remove()
{
	i.clear();
	v.clear();
	c.clear();
}

void Object::Readobj(const char* s)
{
	FILE* path = fopen(s, "r");
	if (path == NULL)
		return;

	char count[128];
	int vertexnum = 0;
	int facenum = 0;
	int uvnum = 0;
	//--- 1. 전체 버텍스 개수 및 삼각형 개수 세기
	while (!feof(path))
	{
		fscanf(path, "%s", count);
		if (count[0] == 'v' && count[1] == '\0')
			vertexnum++;
		else if (count[0] == 'f' && count[1] == '\0')
			facenum++;
		else if (count[0] == 'v' && count[1] == 't' && count[3] == '\0')
			uvnum++;
		memset(count, '\0', sizeof(count));
	}
	rewind(path);
	int vertIndex = 0;
	int faceIndex = 0;
	int uvIndex = 0;
	//--- 2. 메모리 할당
	glm::vec3* vertex = new glm::vec3[vertexnum];
	glm::vec3* face = new glm::vec3[facenum];
	glm::vec3* uvdata = new glm::vec3[facenum];
	glm::vec2* uv = new glm::vec2[uvnum];
	char bind[128];

	while (!feof(path)) {
		fscanf(path, "%s", bind);
		if (bind[0] == 'v' && bind[1] == '\0') {
			fscanf(path, "%f %f %f\n",
				&vertex[vertIndex].x, &vertex[vertIndex].y, &vertex[vertIndex].z);
			vertIndex++;
		}
		else if (bind[0] == 'f' && bind[1] == '\0') {
			unsigned int temp_face[3], temp_uv[3], temp_normal[3];
			fscanf(path, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
				&temp_face[0], &temp_uv[0], &temp_normal[0],
				&temp_face[1], &temp_uv[1], &temp_normal[1],
				&temp_face[2], &temp_uv[2], &temp_normal[2]);
			fscanf(path, "%d//%d %d//%d %d//%d\n",
				&temp_face[0], &temp_normal[0],
				&temp_face[1], &temp_normal[1],
				&temp_face[2], &temp_normal[2]);
			face[faceIndex].x = temp_face[0] - 1;
			face[faceIndex].y = temp_face[1] - 1;
			face[faceIndex].z = temp_face[2] - 1;
			uvdata[faceIndex].x = temp_uv[0];
			uvdata[faceIndex].y = temp_uv[1];
			uvdata[faceIndex].z = temp_uv[2];
			faceIndex++;
		}
		else if (bind[0] == 'v' && bind[1] == 't' && bind[2] == '\0') {
			fscanf(path, "%f %f\n", &uv[uvIndex].x, &uv[uvIndex].y);
			uvIndex++;
		}
	}

	for (int i = 0; i < vertexnum; ++i)
	{
		v.emplace_back(vertex[i]);
		c.push_back({ uidC(dre),uidC(dre),uidC(dre) });
		n.emplace_back(face[i]);
		this->uv.emplace_back(uvdata[i]);
	}

	for (int j = 0; j < facenum; ++j)
	{
		i.push_back(face[j].x);
		i.push_back(face[j].y);
		i.push_back(face[j].z);
	}

	unsigned int tmpidx[] =
	{ 1, 5, 3, 4, 3, 8,
	  8, 7, 6, 6, 2, 8,
	  2, 1, 4, 6, 5, 2,
	  2, 5, 1, 6, 7, 5,
	  8, 2, 4, 3, 5, 7,
	  4, 1, 3, 8, 3, 7 };

	glm::vec3 BB[] = {
	{0.5f, 0.5f, -0.5f  },
	{0.5f, -0.5f, -0.5f },
	{0.5f, 0.5f, 0.5f   },
	{0.5f, -0.5f, 0.5f  },
	{-0.5f, 0.5f, -0.5f },
	{-0.5f, -0.5f, -0.5f},
	{-0.5f, 0.5f, 0.5f  },
	{-0.5f, -0.5f, 0.5f }
	};

	for (int j = 0; j < 36; ++j)
		i.push_back(tmpidx[j] + v.size());

	for (int i = 0; i < 8; ++i)
	{
		v.push_back(BB[i]);
		c.push_back({ 0.0f, 0.0f, 0.0f });
	}

	InitBuffer();
	UpdateMatrix();

	fclose(path);
}

void Object::UpdateMatrix()
{
	matrix = glm::mat4(1.0f);

	// 한 점을 기준으로 이동
	matrix = glm::translate(matrix, rotBy);
	matrix = glm::rotate(matrix, glm::radians(rotByAngle.z), { 0.0f, 0.0f, 1.0f });
	matrix = glm::rotate(matrix, glm::radians(rotByAngle.y), { 0.0f, 1.0f, 0.0f });
	matrix = glm::rotate(matrix, glm::radians(rotByAngle.x), { 1.0f, 0.0f, 0.0f });
	matrix = glm::translate(matrix, -rotBy);

	// 자체 이동
	matrix = glm::translate(matrix, T + rotBy);
	matrix = glm::rotate(matrix, glm::radians(R.x), { 1.0f, 0.0f, 0.0f });
	matrix = glm::rotate(matrix, glm::radians(R.y), { 0.0f, 1.0f, 0.0f });
	matrix = glm::rotate(matrix, glm::radians(R.z), { 0.0f, 0.0f, 1.0f });
	matrix = glm::scale(matrix, S);
}

void Object::RotByPoint(int n, bool b, glm::vec3 o)
{
	rotBy = o;
	if (b) rotByAngle[n] += 5.0f;
	else rotByAngle[n] -= 5.0f;
}

void Object::InitBuffer()
{
	glGenVertexArrays(1, &vao); // VAO를 생성하고 할당합니다.
	glBindVertexArray(vao); // VAO를 바인드합니다.
	glGenBuffers(2, vbo); // 2개의 VBO를 생성하고 할당합니다.

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * i.size(), i.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * v.size(), v.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * c.size(), c.data(), GL_DYNAMIC_DRAW);

	// 정점 좌표 데이터를 VAO에 바인딩하고 활성화합니다.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	// 색상 데이터를 VAO에 바인딩하고 활성화합니다.
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}