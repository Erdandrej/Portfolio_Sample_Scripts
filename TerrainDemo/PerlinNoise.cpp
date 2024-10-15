#include "PerlinNoise.h"

glm::vec2 PerlinNoise::randomGradient(int ix, int iy) {
	const unsigned w = 8 * sizeof(unsigned);
	const unsigned s = w / 2;
	unsigned a = ix, b = iy;
	a *= 3284157443;

	b ^= a << s | a >> w - s;
	b *= 1911520717;

	a ^= b << s | b >> w - s;
	a *= 2048419325;
	float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

	// Create the vector from the angle
	glm::vec2 v;
	v.x = sin(random);
	v.y = cos(random);

	return v;
}

float PerlinNoise::dotGridGradient(int ix, int iy, float x, float y) {
	// Get gradient from integer coordinates
	glm::vec2 gradient = randomGradient(ix, iy);

	// Compute the distance vector
	float dx = x - (float)ix;
	float dy = y - (float)iy;

	// Compute the dot-product
	return (dx * gradient.x + dy * gradient.y);
}

float PerlinNoise::interpolate(float a0, float a1, float w)
{
	return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;
}


float PerlinNoise::perlin(float x, float y) {

	// Determine grid cell corner coordinates
	int x0 = (int)x;
	int y0 = (int)y;
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	// Compute Interpolation weights
	float sx = x - (float)x0;
	float sy = y - (float)y0;

	// Compute and interpolate top two corners
	float n0 = dotGridGradient(x0, y0, x, y);
	float n1 = dotGridGradient(x1, y0, x, y);
	float ix0 = interpolate(n0, n1, sx);

	// Compute and interpolate bottom two corners
	n0 = dotGridGradient(x0, y1, x, y);
	n1 = dotGridGradient(x1, y1, x, y);
	float ix1 = interpolate(n0, n1, sx);

	// Final step: interpolate between the two previously interpolated values, now in y
	float value = interpolate(ix0, ix1, sy);

	return value;
}

float PerlinNoise::fullForcePerlin(int u, int v, int gridSize, int octaves, float displacementFactor, bool normalize) {
	float frequency = 1;
	float amplitude = 1;
	
	// Add all octaves of perlin noise value
	frequency = 1;
	amplitude = 1;
	float value = 0;
	for (int i = 0; i < octaves; i++)
	{
		value += perlin(u * frequency / gridSize, v * frequency / gridSize) * amplitude;

		frequency *= 2;
		amplitude /= 2;
	}

	// Adds contrast
	value *= displacementFactor;

	// Clip value between 1 and -1
	if (value > 1.0f) value = 1.0f;
	else if (value < -1.0f) value = -1.0f;

	if (normalize) {
		value = (value + 1) * 0.5;
	}
		

	return value;
}
