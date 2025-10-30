#pragma once
#include "triangle.h"
#include <vector>
#include <thread>
#include "sentinelQueue.h"

// store temporary data for triangle rendering
struct triangleData
{
	triangle tri;	// triangle
	color a;		// ambient light
	color d;		// diffuse light

	triangleData() = default;
	triangleData(triangle _tri, color _a, color _d) :tri(_tri), a(_a), d(_d) {
	}
};

class Scene
{
	std::atomic<int> triCounter;		// atomic triangle index counter for threads
	std::atomic<int> meshCounter;		// atomic mesh index counter for threads
	std::atomic<bool> meshProcessed;	// indicator for triangle threads to join

	SentinelQueue<triangleData> queue;

protected:
	Renderer* renderer;
	std::vector<Mesh*> sceneObjects;
	matrix sceneCamera;
	Light sceneLight;

	// protected constructor to prevent instantiation
	// abstract class
	Scene(Renderer* renderer)
	{
		this->renderer = renderer;
		triCounter = 0;
		meshCounter = 0;
		meshProcessed = false;
	}

	// process vertex for triangle
	// Input Variables:
	// - p : projection matrix
	// - w : world matrix of mesh
	// - mv	: mesh vertex
	// - width : width of canvas
	// - height : height of canvas
	inline void processVertex(const matrix& p, const matrix& w, const Vertex& mv,
		const unsigned int& width, const unsigned int& height, Vertex& out)
	{
		out.p = p * mv.p;					// Apply transformations
		out.p.divideW();					// Perspective division to normalize coordinates

		// Transform normals into world space for accurate lighting
		// no need for perspective correction as no shearing or non-uniform scaling
		out.normal = w * mv.normal;
		out.normal.normalise();

		// Map normalized device coordinates to screen space
		out.p[0] = (out.p[0] + 1.f) * 0.5f * width;
		out.p[1] = (out.p[1] + 1.f) * 0.5f * height;
		out.p[1] = height - out.p[1];		// Invert y-axis

		// Copy vertex colours
		out.rgb = mv.rgb;
	}

	// Method to draw triangles with multi threading
	// Input Variables:
	// - tris		: pointer to triangle array 
	// - total		: size of triangle array 
	// - renderer	: reference to renderer 
	// - L			: reference to Light
	void drawTriangles(triangleData* tris, int total, Renderer& renderer, vec4 lightDir)
	{
		int i;
		while ((i = triCounter.fetch_add(1)) < total)
			tris[i].tri.draw(renderer, lightDir, tris[i].a, tris[i].d);
	}

	// method processes and draws triangles using caching
	// - meshes	: array of meshes
	// - renderer : reference to the renderer
	// - L : light
	// default value set to 3 works best for this value
	void renderCaching(const std::vector<Mesh*>& meshes, Renderer& renderer, Light& L)
	{
		L.omega_i.normalise(); // normalize light before rendering

		// cache canvas width and height
		unsigned int width = renderer.canvas.getWidth();
		unsigned int height = renderer.canvas.getHeight();

		for (auto& mesh : meshes)
		{
			matrix p = renderer.vp * mesh->world;	// calculate projection matrix for the mesh

			// calculate diffuse and ambient lights for mesh
			color ambient = L.ambient * mesh->ka;
			color diffuse = L.L * mesh->kd;

			// process all triangles of mesh
			for (int i = 0; i < mesh->triangles.size(); i++)
			{
				Vertex t[3];						// Temporary array to store transformed triangle vertices

				// process all 3 vertices of triangles (loop unrolling)
				processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[0]], width, height, t[0]);
				processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[1]], width, height, t[1]);
				processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[2]], width, height, t[2]);

				// Clip triangles with Z-values outside [-1, 1]
				if (fabs(t[0].p[2]) > 1.0f || fabs(t[1].p[2]) > 1.0f || fabs(t[2].p[2]) > 1.0f) break;

				// Create and render triangle object 
				triangle(t[0], t[1], t[2]).draw(renderer, L.omega_i, ambient, diffuse);
			}
		}
	}

	// method processes and draws triangles using shared counter
	// - meshes	: array of meshes
	// - renderer : reference to the renderer
	// - L : light
	// - totalThreads : number of threads to use for multithreading
	// default value set to 3 works best for this value
	void renderSharedCounter(const std::vector<Mesh*>& meshes, Renderer& renderer, Light& L, unsigned int totalThreads = 3)
	{
		L.omega_i.normalise(); // normalize light before rendering

		// cache canvas width and height
		unsigned int width = renderer.canvas.getWidth();
		unsigned int height = renderer.canvas.getHeight();

		std::vector<triangleData> triangles;

		for (auto& mesh : meshes)
		{
			matrix p = renderer.vp * mesh->world; // calculate projection matrix for the mesh

			// calculate diffuse and ambient lights for mesh
			color ambient = L.ambient * mesh->ka;
			color diffuse = L.L * mesh->kd;

			// process all triangles of mesh
			for (int i = 0; i < mesh->triangles.size(); i++)
			{
				Vertex t[3]; // Temporary array to store transformed triangle vertices

				// process all 3 vertices of triangles (loop unrolling)
				processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[0]], width, height, t[0]);
				processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[1]], width, height, t[1]);
				processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[2]], width, height, t[2]);

				// Clip triangles with Z-values outside [-1, 1]
				if (fabs(t[0].p[2]) > 1.0f || fabs(t[1].p[2]) > 1.0f || fabs(t[2].p[2]) > 1.0f) break;

				// add triangle to triangle list
				triangles.emplace_back(triangleData(triangle(t[0], t[1], t[2]), ambient, diffuse));
			}
		}

		unsigned int size = triangles.size(); // total triangles count

		triCounter.store(0); // reset triangle counter

		// render triangle using multiple threads
		std::vector<std::thread> threads; // threads array
		for (int i = 0; i < totalThreads; i++)
			threads.emplace_back(std::thread(&Scene::drawTriangles, this, &triangles[0], size, std::ref(renderer), L.omega_i));

		for (auto& t : threads)
			t.join();
	}

	void processMesh(const std::vector<Mesh*>& meshes, int total,
		const unsigned int& width, const unsigned int& height, matrix vp, Light L)
	{
		int i;
		while ((i = meshCounter.fetch_add(1)) < total)
		{
			Mesh* mesh = meshes[i];
			matrix p = vp * mesh->world; // calculate projection matrix for the mesh

			// calculate diffuse and ambient lights for mesh
			color ambient = L.ambient * mesh->ka;
			color diffuse = L.L * mesh->kd;

			// process all triangles of mesh
			for (int i = 0; i < mesh->triangles.size(); i++)
			{
				Vertex t[3]; // Temporary array to store transformed triangle vertices

				// process all 3 vertices of triangles (loop unrolling)
				processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[0]], width, height, t[0]);
				processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[1]], width, height, t[1]);
				processVertex(p, mesh->world, mesh->vertices[mesh->triangles[i].v[2]], width, height, t[2]);

				// Clip triangles with Z-values outside [-1, 1]
				if (fabs(t[0].p[2]) > 1.0f || fabs(t[1].p[2]) > 1.0f || fabs(t[2].p[2]) > 1.0f) break;

				// add triangle to triangle list
				queue.enqueue(triangleData(triangle(t[0], t[1], t[2]), ambient, diffuse));
			}
		}
	}

	void processTriangles(Renderer& renderer, const vec4& dir)
	{
		triangleData data;	// to store triangle data when dequeue
		bool process;		// to check if dequeue is successfull
		while ((process = queue.dequeue(data)) || !meshProcessed) // check for dequeue or mesh processed by meshProcess threads
		{
			if (process)
				data.tri.draw(renderer, dir, data.a, data.d);
		}
	}

	// method processes and draws triangles using shared counter
	// - meshes	: array of meshes
	// - renderer : reference to the renderer
	// - L : light
	// - totalThreads : number of threads to use for multithreading
	// default value set to 3 works best for this value
	void renderSentinelQueue(const std::vector<Mesh*>& meshes, Renderer& renderer, Light& L,
		unsigned int meshThreadCount = 3, unsigned int triThreadCount = 3)
	{
		L.omega_i.normalise(); // normalize light before rendering

		// cache canvas width and height
		unsigned int width = renderer.canvas.getWidth();
		unsigned int height = renderer.canvas.getHeight();

		triCounter.store(0);
		meshCounter.store(0);
		meshProcessed.store(0);

		std::vector<std::thread> meshThreads;	// mesh threads array
		std::vector<std::thread> triThreads;	// triangles threads array

		for (int i = 0; i < meshThreadCount; i++)
			meshThreads.emplace_back(std::thread(&Scene::processMesh, this, std::ref(meshes), meshes.size(), width, height, renderer.vp, L));

		for (int i = 0; i < triThreadCount; i++)
			triThreads.emplace_back(std::thread(&Scene::processTriangles, this, std::ref(renderer), L.omega_i));

		for (auto& t : meshThreads)
			t.join();

		meshProcessed.store(1);

		for (auto& t : triThreads)
			t.join();
	}

	void render(const std::vector<Mesh*>& meshes, Renderer& renderer, Light& L)
	{
		renderCaching(meshes, renderer, L);
		//renderSharedCounter(meshes, renderer, L,4);
		//renderSentinelQueue(meshes, renderer, L);
	}

public:
	virtual ~Scene()
	{
		for (auto& m : sceneObjects)
			delete m;
		std::cout << "Scene destroyed, all meshes deleted." << std::endl;
	}

	void PreUpdate()
	{
		if (renderer == nullptr)
			return;

		// clear the canvas and zbuffer before rendering
		renderer->clear();
		// check for input events
		renderer->canvas.checkInput();
	}

	void RenderScene()
	{
		if (renderer == nullptr)
			return;

		// update view projection matrix before rendering
		renderer->updateVP(sceneCamera);
		// render all objects in a scene
		render(sceneObjects, *renderer, sceneLight);
		// present the rendered frame
		renderer->present();
	}

	virtual void UpdateScene(float deltaTime) = 0;
};