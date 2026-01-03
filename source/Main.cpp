
#include "../include/scenes.h"
#include <stdlib.h>
#include <queue>

// class handles average FPS calculation
class FPSCounter
{
private:
	std::queue<double> frameTimes;
	double totalTime;
	size_t maxSamples;
public:
	FPSCounter(size_t maxSamples = 100) : totalTime(0), maxSamples(maxSamples) {}
	void UpdateFrame(double frameTime) {
		frameTimes.push(frameTime);
		totalTime += frameTime;
		if (frameTimes.size() > maxSamples) {
			totalTime -= frameTimes.front();
			frameTimes.pop();
		}
	}
	double Get() const {
		if (frameTimes.empty()) return 0.0;
		return static_cast<double>(frameTimes.size()) / (totalTime / 1000.0);
	}
};

void startScene()
{
	Renderer renderer;
	Scene* scene = new Scene2(&renderer);
	FPSCounter fpsCounter(10);

	bool running = true;

	double totalTime = 0;
	double lastInputTime = 0;
	ChronoTimer timer;

	// Main rendering loop
	while (running)
	{
		scene->PreUpdate(); // Pre-update scene (clear canvas, handle input)

		if (renderer.canvas.keyPressed(VK_ESCAPE) || renderer.canvas.IsQuit()) break;
		if (renderer.canvas.keyPressed(VK_TAB) &&
			totalTime - lastInputTime > 400)
		{
			scene->CycleRenderMode();
			lastInputTime = totalTime;
		}

		timer.reset();
		scene->UpdateScene(0.f); // deltatime is not used in this scene
		scene->RenderScene();
		auto deltaTime = timer.elapsed();
		totalTime += deltaTime;

		// Update window title with Debug information
		fpsCounter.UpdateFrame(deltaTime);
		renderer.canvas.setWindowTitle(
			"Rasterizer ( " + ToString(scene->getRenderMode()) +
			") FPS ( " + std::to_string(fpsCounter.Get()) +
			") Time ( " + std::to_string(deltaTime) + " ) ");
	}

	delete scene;
}

// Entry point of the application
// No input variables
int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
	//int main() 
{

	startScene();

	// std::queue<int> q;
	// q.push(10);
	// q.push(3);
	// 
	// std::cout << q.front() << std::endl;

	return 0;
}