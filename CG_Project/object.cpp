#include "stdafx.h"
#include "Object.h"
#include "stb_image.h"

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

Object::Object(const char* c, GLuint sp, glm::vec3 s = { 1.0f, 1.0f, 1.0f }, glm::vec3 r = { 0.0f, 0.0f, 0.0f }, glm::vec3 t = { 0.0f, 0.0f, 0.0f }, glm::vec4 cc = {1.0f, 1.0f, 1.0f, 1.0f}, int shp = 0)
	: S{s}, R{r}, T{t}, objColor{cc}, ShaderProgram{sp}, shape{shp}
{
	Readobj(c);
}

Object::Object(GLuint sp, glm::vec3 s, glm::vec3 r, glm::vec3 t, glm::vec4 cc, int shp) // Todo
	: S{ s }, R{ r }, T{ t }, objColor{ cc }, ShaderProgram{ sp }, shape{ shp }
{
	switch (shape)
	{
	case 1:
		Readobj("resources/brick.obj");
		InitTexture("resources/brick_base.png");
		break;

	case 2:
		Readobj("resources/brick2.obj");
		InitTexture("resources/brick2_base.png");
		break;

	case 3:
		Readobj("resources/star.obj");
		InitTexture("resources/star_base.png");
		break;

	case 4:
		Readobj("resources/coin.obj");
		InitTexture("resources/coin_base.png");
		break;

	case 5:
		Readobj("resources/pipe.obj");
		InitTexture("resources/pipe_base.png");
		break;
	}

}

void Object::Update()
{
	UpdateMatrix();

	vCenterPos = T + origin;
	vAxisDir[0] = T + origin + glm::vec3{ 1.0f, 0.0f, 0.0f };
	vAxisDir[1] = T + origin + glm::vec3{ 0.0f, 1.0f, 0.0f };
	vAxisDir[2] = T + origin + glm::vec3{ 0.0f, 0.0f, 1.0f };
	UpdateBB();
	cout << T.x << ", " << T.y << ", " << T.z << endl;
}

void Object::Render()
{
	glBindVertexArray(vao);
	glBindTexture(GL_TEXTURE_2D, texture);

	view = glm::lookAt(cameraPos, cameraDirection + cameraPos, cameraUp);
	view = glm::rotate(view, glm::radians(cameraAngle.z), { 0.0f, 0.0f, 1.0f });
	view = glm::rotate(view, glm::radians(cameraAngle.y), { 0.0f, 1.0f, 0.0f });
	view = glm::rotate(view, glm::radians(cameraAngle.x), { 1.0f, 0.0f, 0.0f });

	unsigned int lightPosLocation = glGetUniformLocation(ShaderProgram, "lightPos"); 
	glUniform3f(lightPosLocation, lightPos.x, lightPos.y, lightPos.z);
	int lightColorLocation = glGetUniformLocation(ShaderProgram, "lightColor"); 
	glUniform3f(lightColorLocation, lightColor.r, lightColor.g, lightColor.b); 
	int objColorLocation = glGetUniformLocation(ShaderProgram, "objectColor"); 
	glUniform4f(objColorLocation, objColor.r, objColor.g, objColor.b, objColor.a);
	unsigned int viewPosLocation = glGetUniformLocation(ShaderProgram, "viewPos"); //--- viewPos 값 전달: 카메라 위치
	glUniform3f(viewPosLocation, cameraPos.x, cameraPos.y, cameraPos.z);

	// Location 번호 저장
	unsigned int PosLocation = glGetAttribLocation(ShaderProgram, "in_Position"); //	: 0  Shader의 'layout (location = 0)' 부분
	unsigned int NormalLocation = glGetAttribLocation(ShaderProgram, "in_Normal"); //	: 1
	unsigned int TextLocation = glGetAttribLocation(ShaderProgram, "vTexCoord"); //	: 1
	unsigned int modelLocation = glGetUniformLocation(ShaderProgram, "transform");
	unsigned int viewLocation = glGetUniformLocation(ShaderProgram, "view");
	unsigned int projLocation = glGetUniformLocation(ShaderProgram, "projection");
	unsigned int isLightLocation = glGetUniformLocation(ShaderProgram, "isLight");
	unsigned int LightLocation = glGetUniformLocation(ShaderProgram, "ambientLight");
	glUniform1i(isLightLocation, light_onf);
	glUniform1f(LightLocation, light_hardness);

	glEnableVertexAttribArray(PosLocation); // Enable 필수! 사용하겠단 의미
	glEnableVertexAttribArray(NormalLocation);
	glEnableVertexAttribArray(TextLocation);

	glBindBuffer(GL_ARRAY_BUFFER, vbo); // 정점 좌표용 VBO 바인딩
	glVertexAttribPointer(PosLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 3, 0);
	glVertexAttribPointer(NormalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 3, (void*)(sizeof(glm::vec3)));
	glVertexAttribPointer(TextLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 3, (void*)(sizeof(glm::vec3) * 2));

	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(matrix));
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &proj[0][0]);

	// 여기서 그리기 //
	glDrawArrays(GL_TRIANGLES, 0, v.size());

	glDisableVertexAttribArray(PosLocation); // Disable 필수!
	glDisableVertexAttribArray(NormalLocation);
	glDisableVertexAttribArray(TextLocation);
}

void Object::Remove()
{
	v.clear();
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
	int normalsnum = 0;
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
		else if (count[0] == 'v' && count[1] == 'n' && count[2] == '\0')
			normalsnum++;
		memset(count, '\0', sizeof(count));
	}
	rewind(path);
	int vertIndex = 0;
	int faceIndex = 0;
	int uvIndex = 0;
	int normalIndex = 0;

	//--- 2. 메모리 할당
	glm::vec3* vertex = new glm::vec3[vertexnum];
	glm::vec3* face = new glm::vec3[facenum];
	glm::vec3* uvdata = new glm::vec3[facenum];
	glm::vec3* normalidx = new glm::vec3[facenum];
	glm::vec2* uv = new glm::vec2[uvnum];
	glm::vec3* normals = new glm::vec3[normalsnum];
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
			face[faceIndex].x = temp_face[0] - 1;
			face[faceIndex].y = temp_face[1] - 1;
			face[faceIndex].z = temp_face[2] - 1;
			uvdata[faceIndex].x = temp_uv[0] - 1;
			uvdata[faceIndex].y = temp_uv[1] - 1;
			uvdata[faceIndex].z = temp_uv[2] - 1;
			normalidx[faceIndex].x = temp_normal[0] - 1;
			normalidx[faceIndex].y = temp_normal[1] - 1;
			normalidx[faceIndex].z = temp_normal[2] - 1;
			faceIndex++;
		}
		else if (bind[0] == 'v' && bind[1] == 't' && bind[2] == '\0') {
			fscanf(path, "%f %f\n", &uv[uvIndex].x, &uv[uvIndex].y);
			uvIndex++;
		}
		else if (bind[0] == 'v' && bind[1] == 'n' && bind[2] == '\0') {
			fscanf(path, "%f %f %f\n",
				&normals[normalIndex].x, &normals[normalIndex].y, &normals[normalIndex].z);
			normalIndex++;
		}
	}

	for (int i = 0; i < faceIndex; ++i)
	{
		v.push_back(vertex[int(face[i].x)]);
		v.push_back(normals[int(normalidx[i].x)]);
		v.push_back(glm::vec3{ uv[int(uvdata[i].x)], 0.0f });

		v.push_back(vertex[int(face[i].y)]);
		v.push_back(normals[int(normalidx[i].y)]);
		v.push_back(glm::vec3{ uv[int(uvdata[i].y)], 0.0f });

		v.push_back(vertex[int(face[i].z)]);
		v.push_back(normals[int(normalidx[i].z)]);
		v.push_back(glm::vec3{ uv[int(uvdata[i].z)], 0.0f });
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
	matrix = glm::translate(matrix, T + origin);
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

void Object::InitTexture(const char* s)
{
	int widthImage, heightImage, numberOfChannel;
	glGenTextures(1, &texture); //--- 텍스처 생성
	glBindTexture(GL_TEXTURE_2D, texture); //--- 텍스처 바인딩
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //--- 현재 바인딩된 텍스처의 파라미터 설정하기
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(s, &widthImage, &heightImage, &numberOfChannel, 0); //--- 텍스처로 사용할 비트맵 이미지 로드하기
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); //---텍스처 이미지 정의
	stbi_image_free(data);
}

void Object::InitBuffer()
{
	glGenVertexArrays(1, &vao); // VAO를 생성하고 할당합니다.
	glBindVertexArray(vao); // VAO를 바인드합니다.
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * v.size(), v.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), (void*)0); //--- 위치 속성
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), (void*)(sizeof(glm::vec3))); //--- 노말 속성
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), (void*)(2 * sizeof(glm::vec3))); //--- 노말 속성
	glEnableVertexAttribArray(2);
}

