#include <string>
#include "Src/Foundation/Window.h"
#include "Src/Foundation/Core.h"
#include "Src/Foundation/ShaderManager.h"
#include "Src/Foundation/PSOManager.h"
#include "Src/Foundation/Timer.h"

#include "Src/Polygon/Plane.h"
#include "Src/Polygon/ScreenSpaceTriangle.h"
#include "Src/Polygon/Cube.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    Window win;
    win.create(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "My Window");

    Core core;
    core.init(win.hwnd, WINDOW_WIDTH, WINDOW_HEIGHT);

    ShaderManager shaderManager;
    PSOManager psoManager;
    Timer timer;
    float t = 0;

	// polygon
	ScreenSpaceTriangle triangle;
	triangle.init(&core, &psoManager, &shaderManager);
	Plane plane;
	plane.init(&core, &psoManager, &shaderManager);
	Cube cube;
	cube.init(&core, &psoManager, &shaderManager);
	int option = 3;
	// render
	while (true) {
		core.beginFrame();
		float dt = timer.dt();
		win.processMessages();
		if (win.keys[VK_ESCAPE] == 1) break;
		t += dt;

		switch (option) {
		case 1: {
			core.beginRenderPass();
			triangle.draw(&core, &psoManager, &shaderManager, t);
			break;
		}
		case 2: {
			core.beginRenderPass();
			plane.draw(&core, &psoManager, &shaderManager, t);
			break;
		}
		case 3: {
			core.beginRenderPass();
			cube.draw(&core, &psoManager, &shaderManager, t);
			break;
		}
		default:
			break;
		}
		core.finishFrame();

	}
    core.flushGraphicsQueue();
    return 0;
}
