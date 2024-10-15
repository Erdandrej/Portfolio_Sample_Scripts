#ifdef _WIN32
extern "C" _declspec(dllexport) unsigned int NvOptimusEnablement = 0x00000001;
#endif

#include <GL/glew.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <chrono>

#include <labhelper.h>
#include <imgui.h>
#include <imgui_impl_sdl_gl3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#include <Model.h>
#include "hdr.h"
#include "HeightField.h"
#include "Clouds.h"
#include "Water.h"
#include "Orb.h"

using std::min;
using std::max;

///////////////////////////////////////////////////////////////////////////////
// Various globals
///////////////////////////////////////////////////////////////////////////////
SDL_Window* g_window = nullptr;
float currentTime = 0.0f;
float previousTime = 0.0f;
float deltaTime = 0.0f;
bool showUI = false;
int windowWidth, windowHeight;

// Mouse input
ivec2 g_prevMouseCoords = { -1, -1 };
bool g_isMouseDragging = false;

///////////////////////////////////////////////////////////////////////////////
// Shader programs
///////////////////////////////////////////////////////////////////////////////
GLuint shaderProgram;       // Shader for rendering the final image
GLuint backgroundProgram;
GLuint heightFieldShaderProgram; // Shader for HeightField
GLuint waterShaderProgram; // shader for the water
GLuint waterEffectShaderProgram; // shader for the water effect
GLuint cloudsShaderProgram; // shader for the clouds
GLuint orangeOrbShaderProgram; // shader program for the orange orb
GLuint purpleOrbShaderProgram; // shader program for the purple orb
GLuint greenOrbShaderProgram; // shader program for the green orb
GLuint vegetationShaderProgram; // shader program for the vegetation

///////////////////////////////////////////////////////////////////////////////
// Environment
///////////////////////////////////////////////////////////////////////////////
float environment_multiplier = 1.5f;
GLuint environmentMap, irradianceMap, reflectionMap;
const std::string envmap_base_name = "001";
const std::string envmap_base_name_extra = "003";
const std::string envmap_base_name_extra_nintendo = "004";
bool drawNintendoGuy = false;

///////////////////////////////////////////////////////////////////////////////
// Camera parameters.
///////////////////////////////////////////////////////////////////////////////
vec3 cameraPosition(-70.0f, 50.0f, 70.0f);
vec3 cameraDirection = normalize(vec3(0.0f) - cameraPosition);
float cameraSpeed = 250.f;

vec3 worldUp(0.0f, 1.0f, 0.0f);

///////////////////////////////////////////////////////////////////////////////
// HeightField Parameters
///////////////////////////////////////////////////////////////////////////////
HeightField terrain;
bool drawHeightField = true;
bool isFlat = true;
bool drawFog = false;
bool wireFrameMode = false;
bool easeNoise = true;
int hfSize = 1024;
int hfTesselation = hfSize/4;
int hfGridSize = hfTesselation/4;
int hfOctaves = 6;
float hfDisplacementFactor = 1.2f;
float hfMaxHeight = 100.0f;
float hfWaterHeight = 0.1f;

//grass parameters
int grassFrequency = 3;
float grassMinAltitude = 0.15f;
float grassMaxAltitude = 0.4f ;


///////////////////////////////////////////////////////////////////////////////
// Clouds Parameters
///////////////////////////////////////////////////////////////////////////////
Clouds clouds, waterEffect;
bool drawClouds = true;
float cloudIntensity = 1.0f;
int cloudSize = 1024;
int cloudGridSize = 25;
int cloudTesselation = 100;
int cloudOctaves = 6;

///////////////////////////////////////////////////////////////////////////////
// Water Parameters
///////////////////////////////////////////////////////////////////////////////
Water water;
bool drawWater = true;
bool drawWaterEffect = true;
float waterIntensity = 1.0f;
int waterSize = 1024;
int waterGridSize = 25;
int waterTesselation = 100;
int waterOctaves = 6;

///////////////////////////////////////////////////////////////////////////////
// Orb Parameters
///////////////////////////////////////////////////////////////////////////////
Orb orangeOrb, purpleOrb, greenOrb;
bool drawOrb = true;
float orbIntensity = 1.0f;
int orbSize = 1024;
int orbGridSize = 25;
int orbTesselation = 100;
int orbOctaves = 6;
vec3 orangeOrbTranslation = vec3(0, 350, 100);
vec3 purpleOrbTranslation = vec3(140, 190, -150);
vec3 greenOrbTranslation = vec3(-260, 320, -150);

glm::mat4 createRotationMatrix(float angle, glm::vec3 axis) {
	return glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
}

void loadShaders(bool is_reload)
{
	GLuint shader = labhelper::loadShaderProgram("../project/fullscreenQuad.vert", "../project/background.frag",
	                                      is_reload);
	if(shader != 0)
	{
		backgroundProgram = shader;
	}

	shader = labhelper::loadShaderProgram("../project/shading.vert", "../project/shading.frag", is_reload);
	if(shader != 0)
	{
		shaderProgram = shader;
	}

	shader = labhelper::loadShaderProgram("../project/heightfield.vert", "../project/heightfield.frag", false);
	if (shader != 0)
	{
		heightFieldShaderProgram = shader;
	}

	shader = labhelper::loadShaderProgram("../project/cloud.vert", "../project/cloud.frag", false);
	if (shader != 0)
	{
		cloudsShaderProgram = shader;
	}

	shader = labhelper::loadShaderProgram("../project/water.vert", "../project/water.frag", false);
	if (shader != 0)
	{
		waterShaderProgram = shader;
	}

	shader = labhelper::loadShaderProgram("../project/waterEffect.vert", "../project/waterEffect.frag", false);
	if (shader != 0)
	{
		waterEffectShaderProgram = shader;
	}

	shader = labhelper::loadShaderProgram("../project/orb.vert", "../project/orangeOrb.frag", false);
	if (shader != 0)
	{
		orangeOrbShaderProgram = shader;
	}

	shader = labhelper::loadShaderProgram("../project/orb.vert", "../project/purpleOrb.frag", false);
	if (shader != 0)
	{
		purpleOrbShaderProgram = shader;
	}

	shader = labhelper::loadShaderProgram("../project/orb.vert", "../project/greenOrb.frag", false);
	if (shader != 0)
	{
		greenOrbShaderProgram = shader;
	}

	shader = labhelper::loadShaderProgram("../project/vegetation.vert", "../project/vegetation.frag", false);
	if (shader != 0)
	{
		vegetationShaderProgram = shader;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// This function is called once at the start of the program and never again
///////////////////////////////////////////////////////////////////////////////
void initialize()
{
	ENSURE_INITIALIZE_ONLY_ONCE();

	///////////////////////////////////////////////////////////////////////
	//		Load Shaders
	///////////////////////////////////////////////////////////////////////
	loadShaders(false);
	
	///////////////////////////////////////////////////////////////////////
	// Load HeightField
	///////////////////////////////////////////////////////////////////////
	terrain.generate_mesh(hfSize, hfTesselation, hfGridSize, hfOctaves, hfDisplacementFactor, hfMaxHeight, hfWaterHeight, easeNoise, grassFrequency, grassMinAltitude, grassMaxAltitude);

	///////////////////////////////////////////////////////////////////////
	// Load Clouds
	///////////////////////////////////////////////////////////////////////
	clouds.generateCloudMesh(cloudTesselation, 500.0f);
	clouds.generateCloudTexture(cloudSize, cloudSize, cloudGridSize, cloudOctaves, cloudIntensity);

	///////////////////////////////////////////////////////////////////////
	// Load Water
	///////////////////////////////////////////////////////////////////////
	water.generateWaterMesh(waterTesselation, hfWaterHeight * hfMaxHeight);
	water.generateWaterTexture(waterSize, waterSize, waterGridSize, waterOctaves, waterIntensity);

	waterEffect.generateCloudMesh(waterTesselation, hfWaterHeight * hfMaxHeight + 0.1f);
	waterEffect.generateCloudTexture(waterSize, waterSize, waterGridSize, waterOctaves, 0.4f);

	///////////////////////////////////////////////////////////////////////
	// Load Orbs
	///////////////////////////////////////////////////////////////////////
	orangeOrb.generateOrbMesh(orbTesselation, 26.0f);
	orangeOrb.generateOrbTexture(orbSize, orbSize, orbGridSize, orbOctaves, orbIntensity);

	purpleOrb.generateOrbMesh(orbTesselation, 18.0f);
	purpleOrb.generateOrbTexture(orbSize, orbSize, orbGridSize, orbOctaves, orbIntensity);

	greenOrb.generateOrbMesh(orbTesselation, 35.0f);
	greenOrb.generateOrbTexture(orbSize, orbSize, orbGridSize, orbOctaves, orbIntensity);

	///////////////////////////////////////////////////////////////////////
	// Load environment map
	///////////////////////////////////////////////////////////////////////
	const int roughnesses = 8;
	std::vector<std::string> filenames;
	for(int i = 0; i < roughnesses; i++)
		filenames.push_back("../scenes/envmaps/" + envmap_base_name + "_dl_" + std::to_string(i) + ".hdr");

	environmentMap = labhelper::loadHdrTexture("../scenes/envmaps/" + envmap_base_name_extra + ".hdr");
	irradianceMap = labhelper::loadHdrTexture("../scenes/envmaps/" + envmap_base_name + "_irradiance.hdr");
	reflectionMap = labhelper::loadHdrMipmapTexture(filenames);

	glEnable(GL_DEPTH_TEST); // enable Z-buffering
	//glEnable(GL_CULL_FACE);  // enables backface culling
}

void drawBackground(const mat4& viewMatrix, const mat4& projectionMatrix)
{
	glUseProgram(backgroundProgram);
	labhelper::setUniformSlow(backgroundProgram, "environment_multiplier", environment_multiplier);
	labhelper::setUniformSlow(backgroundProgram, "inv_PV", inverse(projectionMatrix * viewMatrix));
	labhelper::setUniformSlow(backgroundProgram, "camera_pos", cameraPosition);
	labhelper::drawFullScreenQuad();
}

///////////////////////////////////////////////////////////////////////////////
/// This function is used to draw the main objects on the scene
///////////////////////////////////////////////////////////////////////////////
void drawScene(GLuint currentShaderProgram,
               const mat4& viewMatrix,
               const mat4& projectionMatrix,
               const mat4& lightViewMatrix,
               const mat4& lightProjectionMatrix)
{
	glUseProgram(currentShaderProgram);

	// Environment
	labhelper::setUniformSlow(currentShaderProgram, "environment_multiplier", environment_multiplier);

	// camera
	labhelper::setUniformSlow(currentShaderProgram, "viewInverse", inverse(viewMatrix));
}

///////////////////////////////////////////////////////////////////////////////
/// This function will be called once per frame, so the code to set up
/// the scene for rendering should go here
///////////////////////////////////////////////////////////////////////////////
void display(void)
{
	///////////////////////////////////////////////////////////////////////////
	// Check if window size has changed and resize buffers as needed
	///////////////////////////////////////////////////////////////////////////
	{
		int w, h;
		SDL_GetWindowSize(g_window, &w, &h);
		if(w != windowWidth || h != windowHeight)
		{
			windowWidth = w;
			windowHeight = h;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Setup matrices
	///////////////////////////////////////////////////////////////////////////
	mat4 projMatrix = perspective(radians(45.0f), float(windowWidth) / float(windowHeight), 5.0f, 2000.0f);
	mat4 viewMatrix = lookAt(cameraPosition, cameraPosition + cameraDirection, worldUp);

	///////////////////////////////////////////////////////////////////////////
	// Bind the environment map(s) to unused texture units
	///////////////////////////////////////////////////////////////////////////
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, environmentMap);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, irradianceMap);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, reflectionMap);
	glActiveTexture(GL_TEXTURE0);

	///////////////////////////////////////////////////////////////////////////
	// Setup transparency
	///////////////////////////////////////////////////////////////////////////
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	///////////////////////////////////////////////////////////////////////////
	// Draw from camera
	///////////////////////////////////////////////////////////////////////////
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
	glClearColor(0.2f, 0.2f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawBackground(viewMatrix, projMatrix);

	// HeightField 
	if (drawHeightField)
	{	
		// Turn on wireframe mode
		 if(wireFrameMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
		glUseProgram(heightFieldShaderProgram);

		// Light
		labhelper::setUniformSlow(heightFieldShaderProgram, "light.ambient", vec3(0.2, 0.2, 0.2));
		labhelper::setUniformSlow(heightFieldShaderProgram, "light.diffuse", vec3(0.3, 0.3, 0.3));
		labhelper::setUniformSlow(heightFieldShaderProgram, "light.specular", vec3(1.0, 1.0, 1.0));
		labhelper::setUniformSlow(heightFieldShaderProgram, "light.direction", vec3(-0.2f, -1.0f, -0.3f));

		// Matrices
		labhelper::setUniformSlow(heightFieldShaderProgram, "viewPosition", cameraPosition);
		labhelper::setUniformSlow(heightFieldShaderProgram, "modelViewProjectionMatrix", projMatrix * viewMatrix);
		labhelper::setUniformSlow(heightFieldShaderProgram, "modelViewMatrix", viewMatrix);

		labhelper::setUniformSlow(heightFieldShaderProgram, "isFlat", isFlat);
		labhelper::setUniformSlow(heightFieldShaderProgram, "drawFog", drawFog);

		terrain.render_triangles();

		glUseProgram(0);

		glUseProgram(vegetationShaderProgram);

		labhelper::setUniformSlow(vegetationShaderProgram, "modelViewProjectionMatrix",
			projMatrix * viewMatrix);
		labhelper::setUniformSlow(vegetationShaderProgram, "modelViewMatrix", viewMatrix);

		terrain.render_grass();

		glUseProgram(0);

		// Turn off wireframe mode
		if (wireFrameMode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (drawClouds)
	{
		// Turn on wireframe mode
		if (wireFrameMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glUseProgram(cloudsShaderProgram);

		// Environment
		labhelper::setUniformSlow(cloudsShaderProgram, "environment_multiplier", environment_multiplier);

		labhelper::setUniformSlow(cloudsShaderProgram, "modelViewProjectionMatrix",
			projMatrix * viewMatrix);
		labhelper::setUniformSlow(cloudsShaderProgram, "modelViewMatrix", viewMatrix);
		labhelper::setUniformSlow(cloudsShaderProgram, "normalMatrix",
			inverse(transpose(viewMatrix)));

		clouds.submitTriangles();

		glUseProgram(cloudsShaderProgram);
		labhelper::setUniformSlow(cloudsShaderProgram, "currentTime", currentTime);

		glUseProgram(0);

		// Turn off wireframe mode
		if (wireFrameMode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (drawWater)
	{
		// Turn on wireframe mode
		if (wireFrameMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glUseProgram(waterShaderProgram);

		// Environment
		labhelper::setUniformSlow(waterShaderProgram, "environment_multiplier", environment_multiplier);

		labhelper::setUniformSlow(waterShaderProgram, "modelViewProjectionMatrix",
			projMatrix * viewMatrix);
		labhelper::setUniformSlow(waterShaderProgram, "modelViewMatrix", viewMatrix);
		labhelper::setUniformSlow(waterShaderProgram, "normalMatrix",
			inverse(transpose(viewMatrix)));
		labhelper::setUniformSlow(waterShaderProgram, "drawFog", drawFog);
		labhelper::setUniformSlow(waterShaderProgram, "currentTime", currentTime);

		water.submitTriangles();

		glUseProgram(0);

		// Turn off wireframe mode
		if (wireFrameMode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (drawWaterEffect)
	{
		// Turn on wireframe mode
		if (wireFrameMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glUseProgram(waterEffectShaderProgram);

		// Environment
		labhelper::setUniformSlow(waterEffectShaderProgram, "environment_multiplier", environment_multiplier);

		labhelper::setUniformSlow(waterEffectShaderProgram, "modelViewProjectionMatrix",
			projMatrix * viewMatrix);
		labhelper::setUniformSlow(waterEffectShaderProgram, "modelViewMatrix", viewMatrix);
		labhelper::setUniformSlow(waterEffectShaderProgram, "normalMatrix",
			inverse(transpose(viewMatrix)));

		labhelper::setUniformSlow(waterEffectShaderProgram, "currentTime", currentTime);

		waterEffect.submitTriangles();

		glUseProgram(0);

		// Turn off wireframe mode
		if (wireFrameMode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (drawOrb)
	{
		// Turn on wireframe mode
		if (wireFrameMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glUseProgram(orangeOrbShaderProgram);

		// Environment
		labhelper::setUniformSlow(orangeOrbShaderProgram, "environment_multiplier", environment_multiplier);

		labhelper::setUniformSlow(orangeOrbShaderProgram, "modelViewProjectionMatrix",
			projMatrix * viewMatrix);
		labhelper::setUniformSlow(orangeOrbShaderProgram, "modelViewMatrix", viewMatrix);
		labhelper::setUniformSlow(orangeOrbShaderProgram, "normalMatrix",
			inverse(transpose(viewMatrix)));

		labhelper::setUniformSlow(orangeOrbShaderProgram, "currentTime", currentTime);
		labhelper::setUniformSlow(orangeOrbShaderProgram, "translation", orangeOrbTranslation);

		orangeOrb.submitTriangles();

		glUseProgram(0);

		glUseProgram(purpleOrbShaderProgram);

		// Environment
		labhelper::setUniformSlow(purpleOrbShaderProgram, "environment_multiplier", environment_multiplier);

		labhelper::setUniformSlow(purpleOrbShaderProgram, "modelViewProjectionMatrix",
			projMatrix* viewMatrix);
		labhelper::setUniformSlow(purpleOrbShaderProgram, "modelViewMatrix", viewMatrix);
		labhelper::setUniformSlow(purpleOrbShaderProgram, "normalMatrix",
			inverse(transpose(viewMatrix)));

		labhelper::setUniformSlow(purpleOrbShaderProgram, "currentTime", currentTime);
		labhelper::setUniformSlow(purpleOrbShaderProgram, "translation", purpleOrbTranslation);

		purpleOrb.submitTriangles();

		glUseProgram(0);

		glUseProgram(greenOrbShaderProgram);

		// Environment
		labhelper::setUniformSlow(greenOrbShaderProgram, "environment_multiplier", environment_multiplier);

		labhelper::setUniformSlow(greenOrbShaderProgram, "modelViewProjectionMatrix",
			projMatrix* viewMatrix);
		labhelper::setUniformSlow(greenOrbShaderProgram, "modelViewMatrix", viewMatrix);
		labhelper::setUniformSlow(greenOrbShaderProgram, "normalMatrix",
			inverse(transpose(viewMatrix)));

		labhelper::setUniformSlow(greenOrbShaderProgram, "currentTime", currentTime);
		labhelper::setUniformSlow(greenOrbShaderProgram, "translation", greenOrbTranslation);

		greenOrb.submitTriangles();

		glUseProgram(0);

		// Turn off wireframe mode
		if (wireFrameMode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

bool handleEvents(void)
{
	// Allow ImGui to capture events.
	ImGuiIO& io = ImGui::GetIO();

	// check events (keyboard among other)
	SDL_Event event;
	bool quitEvent = false;
	while(SDL_PollEvent(&event))
	{
		ImGui_ImplSdlGL3_ProcessEvent(&event);

		if(event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE))
		{
			quitEvent = true;
		}
		else if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_g)
		{
			showUI = !showUI;
		}
		else if(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_PRINTSCREEN)
		{
			labhelper::saveScreenshot();
		}
		if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT
		   && (!showUI || !io.WantCaptureMouse))
		{
			g_isMouseDragging = true;
			int x;
			int y;
			SDL_GetMouseState(&x, &y);
			g_prevMouseCoords.x = x;
			g_prevMouseCoords.y = y;
		}

		if(!(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)))
		{
			g_isMouseDragging = false;
		}

		if(event.type == SDL_MOUSEMOTION && g_isMouseDragging && !io.WantCaptureMouse)
		{
			// More info at https://wiki.libsdl.org/SDL_MouseMotionEvent
			int delta_x = event.motion.x - g_prevMouseCoords.x;
			int delta_y = event.motion.y - g_prevMouseCoords.y;
			float rotationSpeed = 0.4f;
			mat4 yaw = rotate(rotationSpeed * deltaTime * -delta_x, worldUp);
			mat4 pitch = rotate(rotationSpeed * deltaTime * -delta_y,
			                    normalize(cross(cameraDirection, worldUp)));
			cameraDirection = vec3(pitch * yaw * vec4(cameraDirection, 0.0f));
			g_prevMouseCoords.x = event.motion.x;
			g_prevMouseCoords.y = event.motion.y;
		}
	}

	// check keyboard state (which keys are still pressed)
	const uint8_t* state = SDL_GetKeyboardState(nullptr);

	static bool was_shift_pressed = state[SDL_SCANCODE_LSHIFT];
	if(was_shift_pressed && !state[SDL_SCANCODE_LSHIFT])
	{
		cameraSpeed /= 5;
	}
	if(!was_shift_pressed && state[SDL_SCANCODE_LSHIFT])
	{
		cameraSpeed *= 5;
	}
	was_shift_pressed = state[SDL_SCANCODE_LSHIFT];


	vec3 cameraRight = cross(cameraDirection, worldUp);

	if(state[SDL_SCANCODE_W])
	{
		cameraPosition += cameraSpeed * deltaTime * cameraDirection;
	}
	if(state[SDL_SCANCODE_S])
	{
		cameraPosition -= cameraSpeed * deltaTime * cameraDirection;
	}
	if(state[SDL_SCANCODE_A])
	{
		cameraPosition -= cameraSpeed * deltaTime * cameraRight;
	}
	if(state[SDL_SCANCODE_D])
	{
		cameraPosition += cameraSpeed * deltaTime * cameraRight;
	}
	if(state[SDL_SCANCODE_Q])
	{
		cameraPosition -= cameraSpeed * deltaTime * worldUp;
	}
	if(state[SDL_SCANCODE_E])
	{
		cameraPosition += cameraSpeed * deltaTime * worldUp;
	}
	return quitEvent;
}


///////////////////////////////////////////////////////////////////////////////
/// This function is to hold the general GUI logic
///////////////////////////////////////////////////////////////////////////////
void gui()
{
	// ----------------- Set variables --------------------------
	ImGui::Text("Specs");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
	            ImGui::GetIO().Framerate);
	ImGui::Text("Camera");
	ImGui::SliderFloat("Camera Speed", &cameraSpeed, 100.f, 500.f);
	ImGui::Checkbox("Wireframe", &wireFrameMode);
	ImGui::Text("HeightField");
	ImGui::Checkbox("Draw Height Field", &drawHeightField);
	ImGui::Checkbox("Color is Flat", &isFlat);
	ImGui::Checkbox("Draw Fog", &drawFog);
	if (ImGui::Button("Generate Height Field Mesh")) 
	{
		terrain.generate_mesh(hfSize, hfTesselation, hfGridSize, hfOctaves, hfDisplacementFactor, hfMaxHeight, hfWaterHeight, easeNoise, grassFrequency, grassMinAltitude, grassMaxAltitude);
	};
	ImGui::Checkbox("Ease noise values", &easeNoise);
	ImGui::SliderInt("Grid Size", &hfGridSize, 32, hfTesselation/2);
	ImGui::SliderInt("Octaves", &hfOctaves, 1, 12);
	ImGui::SliderFloat("Displacement Factor", &hfDisplacementFactor, 0.1f, 3.0f);
	ImGui::SliderFloat("Max Height", &hfMaxHeight, 10.0f, 200.0f);
	ImGui::SliderInt("Grass Frequency", &grassFrequency, 1, 15);
	ImGui::SliderFloat("Grass Min Altitude", &grassMinAltitude, 0.1f, 0.9f);
	ImGui::SliderFloat("Grass Max Altitude", &grassMaxAltitude, 0.1f, 0.9f);
	ImGui::Text("Clouds");
	ImGui::Checkbox("Draw Clouds", &drawClouds);
	if (ImGui::Button("Generate Clouds Texture"))
	{
		clouds.generateCloudTexture(cloudSize, cloudSize, cloudGridSize, cloudOctaves, cloudIntensity);
	};
	ImGui::SliderFloat("Cloud Intensity", &cloudIntensity, 0.5f, 3.0f);
	ImGui::SliderInt("Cloud Size", &cloudSize, 50, 1000);
	ImGui::SliderInt("Cloud Tesselation", &cloudTesselation, 50, 300);
	ImGui::SliderInt("Cloud Grid Size", &cloudGridSize, 10, 300);
	ImGui::SliderInt("Cloud Octaves", &cloudOctaves, 1, 12);
	if (ImGui::Button("Generate Water Texture"))
	{
		water.generateWaterTexture(waterSize, waterSize, waterGridSize, waterOctaves, waterIntensity);
	};
	ImGui::SliderFloat("Water Intensity", &waterIntensity, 0.5f, 3.0f);
	ImGui::SliderInt("Water Size", &waterSize, 50, 1000);
	ImGui::SliderInt("Water Tesselation", &waterTesselation, 50, 300);
	ImGui::SliderInt("Water Grid Size", &waterGridSize, 10, 300);
	ImGui::SliderInt("Water Octaves", &waterOctaves, 1, 12);
	if (ImGui::Button("Call GOD"))
	{
		environmentMap = labhelper::loadHdrTexture("../scenes/envmaps/" + envmap_base_name_extra_nintendo + ".hdr");
	}
	// ----------------------------------------------------------
}

int main(int argc, char* argv[])
{
	g_window = labhelper::init_window_SDL("OpenGL Project");

	initialize();

	bool stopRendering = false;
	auto startTime = std::chrono::system_clock::now();

	while(!stopRendering)
	{
		//update currentTime
		std::chrono::duration<float> timeSinceStart = std::chrono::system_clock::now() - startTime;
		previousTime = currentTime;
		currentTime = timeSinceStart.count();
		deltaTime = currentTime - previousTime;

		// Inform imgui of new frame
		ImGui_ImplSdlGL3_NewFrame(g_window);

		// check events (keyboard among other)
		stopRendering = handleEvents();

		// render to window
		display();

		// Render overlay GUI.
		if(showUI)
		{
			gui();
		}

		// Render the GUI.
		ImGui::Render();

		// Swap front and back buffer. This frame will now been displayed.
		SDL_GL_SwapWindow(g_window);
	}
	// Shut down everything. This includes the window and all other subsystems.
	labhelper::shutDown(g_window);
	return 0;
}
