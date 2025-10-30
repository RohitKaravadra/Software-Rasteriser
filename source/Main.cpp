
#include "../include/scenes.h"

// Entry point of the application
// No input variables
int main() {
	Renderer renderer;
	Scene* scene = new Scene2(&renderer);

	bool running = true;

	ChronoTimer timer;

	// Main rendering loop
	while (running)
	{
		scene->PreUpdate(); // Pre-update scene (clear canvas, handle input)

		if (renderer.canvas.keyPressed(VK_ESCAPE) || renderer.canvas.IsQuit()) break;

		timer.reset();
		scene->UpdateScene(0.f); // deltatime is not used in this scene
		scene->RenderScene();
		std::cout << timer.elapsed() << std::endl;
	}

	delete scene;
	return 0;
}