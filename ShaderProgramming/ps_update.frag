#version 330 core
out vec4 FragColor;


void main()
{
	//uint counter = atomicCounter(ac);
	//if (counter < 6)
		FragColor = vec4(1.0f, 1.0f, 0.0f, 0.0f); //fragCol;//vec4(1.0f, 1.0f, 0.0f, 0.0f);//aColor;   

		//switch ( counter ) {
		//case 1:
		//	FragColor = vec4(1.0f,0.0f,0.0f,1.0f);
		//break;
		//case 2:
		//	FragColor = vec4(0.0f,0.0f,0.0f,1.0f);
		//break;
		//default:
		//	FragColor = vec4(0.0f,0.0f,0.0f,1.0f);
		//}

	//atomicCounterIncrement(ac);
}