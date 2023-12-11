#version 330 core

//--- out_Color: ���ؽ� ���̴����� �Է¹޴� ���� ��
//--- FragColor: ����� ������ ������ ������ ���۷� ���� ��.

in vec3 FragPos; //--- ���ؽ� ���̴����Լ� ���� ����
in vec3 Normal;
in vec2 TexCoord;
out vec4 FragColor; //--- ���� ���
uniform vec3 lightColor; // ���� ��
uniform vec3 lightPos; // ���� ��ġ
uniform vec4 objectColor; // ��ü��
uniform vec3 viewPos; // �߰�: �������� ��ġ
uniform bool isLight;
uniform float ambientLight;
uniform sampler2D outTexture;

void main(void) 
{
	if(isLight)
	{
		vec3 ambient = ambientLight * lightColor; //--- �ֺ� ���� ��

		vec3 normalVector = normalize(Normal);
		vec3 lightDir = normalize(lightPos - FragPos.xyz); // ����: ���� ���� ���� ����
		float diffuseLight = max(dot(normalVector, lightDir), 0.0f); //--- N�� L�� ���� ������ ���� ����: ���� ����
		vec3 diffuse = diffuseLight * lightColor; //--- ��� �ݻ� ����: ����ݻ簪 * �������
		
		int shininess = 128; //--- ���� ���
		vec3 viewDir = normalize(viewPos - FragPos.xyz); //--- �������� ����
		vec3 reflectDir = reflect (-lightDir, normalVector); //--- �ݻ� ����: reflect �Լ� - �Ի� ������ �ݻ� ���� ���
		float specularLight = max (dot (viewDir, reflectDir), 0.0); //--- V�� R�� ���������� ���� ����: ���� ����
		specularLight = pow(specularLight, shininess); //--- shininess ���� ���־� ���̶���Ʈ�� ������ش�.
		vec3 specular = specularLight * lightColor; //--- �ſ� �ݻ� ����: �ſ�ݻ簪 * �������
		vec3 result = (ambient + diffuse + specular) * objectColor.rgb; //--- ���� ���� ������ �ȼ� ����: (�ֺ�+����ݻ�+�ſ�ݻ�����)*��ü ����
		FragColor = vec4 (result, objectColor.a);
	}
	else
	{
		float amb = 0.0f;
		vec3 ambient = amb * lightColor; //--- �ֺ� ���� ��

		vec3 result = ambient * objectColor.rgb; //--- ���� ���� ������ �ȼ� ����: (�ֺ�+����ݻ�+�ſ�ݻ�����)*��ü ����
		FragColor = vec4 (result, 1.0);
	}

	FragColor = texture(outTexture, TexCoord) * objectColor.a;
}