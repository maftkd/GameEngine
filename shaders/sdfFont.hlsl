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
	float4 c = textureMap.Sample(sampleState,input.uv);
	//good for fancy script
	/*
	float d = c.r;
	float dd = smoothstep(0.35,0.55,d);
	*/
	//good for simpler fonts
	float d = c.r-0.47;
	clip(d);
	float dd = smoothstep(0,0.03,d);
	//float dd = d/fwidth(d)+0.5;
	float4 txtCol = dd*float4(1,1,1,1);
	//return dd*txtCol;
	return txtCol;
}
