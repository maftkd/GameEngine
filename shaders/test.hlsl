struct vs_in{
	float2 position : POS;//expecting NDC
	float2 uv : TEX;
};

struct vs_out {
	float4 position_clip : SV_POSITION; //required output of vs
	float2 uv : UV;
};

Texture2D textureMap : register(t0);
SamplerState sampleState : register(s0);

vs_out vs_main(vs_in input){
	vs_out output = (vs_out)0;
	output.position_clip =float4(input.position,0.0,1.0);
	output.uv=input.uv;
	return output;
}

float4 ps_main(vs_out input) : SV_TARGET {
	return textureMap.Sample(sampleState,input.uv);
	return float4(input.uv.x,input.uv.y,0.0,1);
	//return float4(0.1,0.1,0.1,1);
}
