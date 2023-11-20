#version 330 core

//--- out_Color: ���ؽ� ���̴����� �Է¹޴� ���� ��
//--- FragColor: ����� ������ ������ ������ ���۷� ���� ��.

in vec3 out_Color; //--- ���ؽ� ���̴����Լ� ���� ����
out vec4 FragColor; //--- ���� ���
uniform bool isSelected;

void main(void) 
{
	if (isSelected)
		FragColor = mix(FragColor, vec4(1.0f, 0.0f, 0.0f, 0.5f), 0.5f);
	else
		FragColor = vec4 (out_Color, 1.0);
}
