#version 330 core

//--- out_Color: 버텍스 세이더에서 입력받는 색상 값
//--- FragColor: 출력할 색상의 값으로 프레임 버퍼로 전달 됨.

in vec3 FragPos; //--- 버텍스 세이더에게서 전달 받음
in vec3 Normal;
in vec2 TexCoord;
out vec4 FragColor; //--- 색상 출력
uniform vec3 lightColor; // 조명 색
uniform vec3 lightPos; // 조명 위치
uniform vec4 objectColor; // 물체색
uniform vec3 viewPos; // 추가: 관찰자의 위치
uniform bool isLight;
uniform float ambientLight;
uniform sampler2D outTexture;

void main(void) 
{
	if(isLight)
	{
		vec3 ambient = ambientLight * lightColor; //--- 주변 조명 값

		vec3 normalVector = normalize(Normal);
		vec3 lightDir = normalize(lightPos - FragPos.xyz); // 수정: 벡터 뺄셈 구문 수정
		float diffuseLight = max(dot(normalVector, lightDir), 0.0f); //--- N과 L의 내적 값으로 강도 조절: 음수 방지
		vec3 diffuse = diffuseLight * lightColor; //--- 산란 반사 조명값: 산란반사값 * 조명색상값
		
		int shininess = 128; //--- 광택 계수
		vec3 viewDir = normalize(viewPos - FragPos.xyz); //--- 관찰자의 방향
		vec3 reflectDir = reflect (-lightDir, normalVector); //--- 반사 방향: reflect 함수 - 입사 벡터의 반사 방향 계산
		float specularLight = max (dot (viewDir, reflectDir), 0.0); //--- V와 R의 내적값으로 강도 조절: 음수 방지
		specularLight = pow(specularLight, shininess); //--- shininess 승을 해주어 하이라이트를 만들어준다.
		vec3 specular = specularLight * lightColor; //--- 거울 반사 조명값: 거울반사값 * 조명색상값
		vec3 result = (ambient + diffuse + specular) * objectColor.rgb; //--- 최종 조명 설정된 픽셀 색상: (주변+산란반사+거울반사조명)*객체 색상
		FragColor = vec4 (result, objectColor.a);
	}
	else
	{
		float amb = 0.0f;
		vec3 ambient = amb * lightColor; //--- 주변 조명 값

		vec3 result = ambient * objectColor.rgb; //--- 최종 조명 설정된 픽셀 색상: (주변+산란반사+거울반사조명)*객체 색상
		FragColor = vec4 (result, 1.0);
	}

	FragColor = texture(outTexture, TexCoord) * objectColor.a;
}