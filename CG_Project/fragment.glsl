#version 330 core

//--- out_Color: 버텍스 세이더에서 입력받는 색상 값
//--- FragColor: 출력할 색상의 값으로 프레임 버퍼로 전달 됨.

in vec3 out_Color; //--- 버텍스 세이더에게서 전달 받음
out vec4 FragColor; //--- 색상 출력
uniform bool isSelected;

void main(void) 
{
	if (isSelected)
		FragColor = mix(FragColor, vec4(1.0f, 0.0f, 0.0f, 0.5f), 0.5f);
	else
		FragColor = vec4 (out_Color, 1.0);
}
