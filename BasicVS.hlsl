float4 main( float4 pos : POSITION ) : SV_POSITION
{
	//Scale
	pos = pos * float4(1.0f, 1.0f, 1.0f, 1);
	//Trans
	pos = pos + float4(0.0f, 0.0f, 0.0f, 0);

	return pos;
}