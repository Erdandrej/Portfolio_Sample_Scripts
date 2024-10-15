#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

///////////////////////////////////////////////////////////////////////////////
// Material
///////////////////////////////////////////////////////////////////////////////
uniform bool isFlat;
uniform bool drawFog;

///////////////////////////////////////////////////////////////////////////////
// In
///////////////////////////////////////////////////////////////////////////////
flat in vec3 flatColor;
in vec3 Color;
in vec3 viewSpacePosition;

///////////////////////////////////////////////////////////////////////////////
// Fog
///////////////////////////////////////////////////////////////////////////////
uniform vec3 fogColor = vec3(0.5, 0.5, 0.5);
uniform float fogDensity = 0.1;
uniform float fogHeightMin = 0.0; //minimum height for fog
uniform float fogHeightMax = 50.0; //maximum height for fog
uniform float fogDistanceMin = 10.0; //minimum distance for fog
uniform float fogDistanceMax = 100.0; //maximum distance for fog

///////////////////////////////////////////////////////////////////////////////
// Output color
///////////////////////////////////////////////////////////////////////////////
layout(location = 0) out vec4 fragmentColor;

void main()
{
    
	vec3 final_color = Color;
    if (isFlat) final_color = flatColor;

	//alternative fog calculations taking into account height and distance from the camera
	// Distance-based fog calculation
    //float distance = length(viewSpacePosition);
    //float distanceFogFactor = smoothstep(fogDistanceMin, fogDistanceMax, distance);

    // Height-based fog calculation
    //float heightFogFactor = smoothstep(fogHeightMin, fogHeightMax, viewSpacePosition.y);

    // Combine both factors
    //float fogFactor = heightFogFactor /** distanceFogFactor*/;

	//exponential calculations for fog
    if (drawFog) {
        float fogFactor = 1.0 - exp(-fogDensity * length(viewSpacePosition) / 25);
        final_color = mix(final_color, fogColor, fogFactor);
    }

	fragmentColor = vec4(final_color, 1.0);
}
