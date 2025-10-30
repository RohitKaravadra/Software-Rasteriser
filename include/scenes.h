
#define _USE_MATH_DEFINES
#include "ChronoTimer.h"
#include "utilities.h"
#include "scene.h"

struct rRot { float x; float y; float z; }; // Structure to store random rotation parameters

class Scene1 : public Scene
{
	float zoffset = 8.0f;
	float step = -0.1f;

public:
	Scene1(Renderer* renderer) :Scene(renderer)
	{
		sceneLight = Light{ vec4(0.f, 1.f, 1.f, 0.f), color(1.0f, 1.0f, 1.0f), color(0.1f, 0.1f, 0.1f) };

		// Create a scene of 40 cubes with random rotations
		for (unsigned int i = 0; i < 20; i++) {
			Mesh* m = new Mesh();
			*m = Mesh::makeCube(1.f);
			m->world = matrix::makeTranslation(-2.0f, 0.0f, (-3 * static_cast<float>(i))) * makeRandomRotation();
			sceneObjects.push_back(m);
			m = new Mesh();
			*m = Mesh::makeCube(1.f);
			m->world = matrix::makeTranslation(2.0f, 0.0f, (-3 * static_cast<float>(i))) * makeRandomRotation();
			sceneObjects.push_back(m);
		}

		float zoffset = 8.0f; // Initial camera Z-offset
		float step = -0.1f;  // Step size for camera movement
	}

	void UpdateScene(float deltaTime) override
	{
		sceneCamera = matrix::makeTranslation(0, 0, -zoffset); // Update camera position

		// Rotate the first two cubes in the scene
		sceneObjects[0]->world = sceneObjects[0]->world * matrix::makeRotateXYZ(0.1f, 0.1f, 0.0f);
		sceneObjects[1]->world = sceneObjects[1]->world * matrix::makeRotateXYZ(0.0f, 0.1f, 0.2f);

		zoffset += step;
		if (zoffset < -60.f || zoffset > 8.f) {
			step *= -1.f;
		}
	}
};

class Scene2 : public Scene
{
	Mesh* sphere = nullptr;
	std::vector<rRot> rotations;
	float sphereOffset = -6.f;
	float sphereStep = 0.1f;

public:
	Scene2(Renderer* renderer) :Scene(renderer)
	{
		sceneLight = Light{ vec4(0.f, 1.f, 1.f, 0.f), color(1.0f, 1.0f, 1.0f), color(0.1f, 0.1f, 0.1f) };

		RandomNumberGenerator& rng = RandomNumberGenerator::getInstance();

		// Create a grid of cubes with random rotations
		for (unsigned int y = 0; y < 6; y++) {
			for (unsigned int x = 0; x < 8; x++) {
				Mesh* m = new Mesh();
				*m = Mesh::makeCube(1.f);
				sceneObjects.push_back(m);
				m->world = matrix::makeTranslation(-7.0f + (static_cast<float>(x) * 2.f), 5.0f - (static_cast<float>(y) * 2.f), -8.f);
				rRot r{ rng.getRandomFloat(-.1f, .1f), rng.getRandomFloat(-.1f, .1f), rng.getRandomFloat(-.1f, .1f) };
				rotations.push_back(r);
			}
		}

		// Create a sphere and add it to the scene
		sphere = Mesh::makeSphere(1.0f, 10, 20);
		sceneObjects.push_back(sphere);
		sphere->world = matrix::makeTranslation(sphereOffset, 0.f, -6.f);
	}

	void UpdateScene(float deltaTime) override
	{
		// Rotate each cube in the grid
		for (int i = 0; i < rotations.size(); i++)
			sceneObjects[i]->world = sceneObjects[i]->world * matrix::makeRotateXYZ(rotations[i].x, rotations[i].y, rotations[i].z);


		// Move the sphere back and forth
		sphereOffset += sphereStep;
		sphere->world = matrix::makeTranslation(sphereOffset, 0.f, -6.f);
		if (sphereOffset > 6.0f || sphereOffset < -6.0f) {
			sphereStep *= -1.f;
		}
	}
};

class Scene3 : public Scene
{
	std::vector<rRot> rotations;
	float x = 0.0f, y = 0.0f, z = -4.0f; // Initial translation parameters

public:

	Scene3(Renderer* renderer) :Scene(renderer)
	{
		// Initialize camera to identity matrix
		sceneCamera = matrix::makeIdentity();

		// create light source {direction, diffuse intensity, ambient intensity}
		sceneLight = Light{ vec4(0.f, 1.f, 1.f, 0.f), color(1.0f, 1.0f, 1.0f), color(0.1f, 0.1f, 0.1f) };

		RandomNumberGenerator& rng = RandomNumberGenerator::getInstance();

		// Create a sphere and a rectangle mesh
		int totalX = 20, totalY = 20, totalZ = 20, space = 2;
		for (int i = 0; i < totalX; i++)
		{
			for (int j = 0; j < totalY; j++)
			{
				for (int k = 0; k < totalZ; k++)
				{
					//Mesh mesh = Mesh::makeCube(2);
					Mesh* mesh = new Mesh();
					mesh = Mesh::makeSphere(1.f, 10, 10);
					//mesh = Mesh::makeCube(1);
					mesh->world = matrix::makeTranslation((i - totalX / 2) * space, (j - totalY / 2) * space, -k * space - 4);
					sceneObjects.push_back(mesh);
					rRot r{ rng.getRandomFloat(-.1f, .1f), rng.getRandomFloat(-.1f, .1f), rng.getRandomFloat(-.1f, .1f) };
					rotations.push_back(r);
				}
			}
		}
	}

	void UpdateScene(float deltaTime) override
	{
		// Handle user inputs for transformations
		if (renderer->canvas.keyPressed('A')) x += 0.1f;
		if (renderer->canvas.keyPressed('D')) x += -0.1f;
		if (renderer->canvas.keyPressed('W')) z += 0.1f;
		if (renderer->canvas.keyPressed('S')) z += -0.1f;
		if (renderer->canvas.keyPressed('Q')) y += 0.1f;
		if (renderer->canvas.keyPressed('E')) y += -0.1f;

		// Apply transformations to the camera
		sceneCamera = matrix::makeTranslation(x, y, z);

		// Rotate each cube in the grid
		for (unsigned int i = 0; i < rotations.size(); i++)
			sceneObjects[i]->world = sceneObjects[i]->world *
			matrix::makeRotateXYZ(rotations[i].x, rotations[i].y, rotations[i].z);

	}
};