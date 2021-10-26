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
	/*
	float d = c.r;
	float txt = step(0.3,d)*step(d,0.7);
	float4 outlineColor = float4(0,0,0,1);
	float4 txtColor = float4(1,1,1,1);
	float4 col = lerp(txtColor,outlineColor,txt);
	return col*smoothstep(0.2,0.4,d);
	*/
	float d = c.r-0.5;
	float dd = d/fwidth(d)+0.5;
	//txt*=step(txt,0.9);
	//float txt = smoothstep(0.35,0.55,d/fwidth(d));
	float txt = clamp(dd+0.5,0.0,1.0);
	//float outline = clamp(d/
	float4 txtCol = float4(1,1,1,1);
	return txt*txtCol;
}
