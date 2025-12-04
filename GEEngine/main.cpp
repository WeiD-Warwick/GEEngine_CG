#include "Src/Foundation/Window.h"
#include <string>
#include "Src/Foundation/Core.h"
#include "Src/Foundation/ShaderManager.h"
#include "Src/Foundation/PSOManager.h"
#include "Src/Foundation/Plane.h"
#include "Src/Foundation/Timer.h"
#include <iostream>
#include "Src/Foundation/ScreenSpaceTriangle.h"
//#include "Src/Foundation/ScreenSpaceTriangle.h"
//#include "Src/Foundation/Plane.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int width = 1024, height = 1024;

    Window win;
    win.create(0, 0, width, height, "My Window");

    Core core;
    core.init(win.hwnd, width, height);

    ShaderManager shaderManager;
    PSOManager psoManager;
    Timer timer;
    float t = 0;

	// polygon
	ScreenSpaceTriangle triangle;
	triangle.init(&core, &psoManager, &shaderManager);
	Plane plane;
	plane.init(&core, &psoManager, &shaderManager);
	int option = 1;
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
		default:
			break;
		}
		core.finishFrame();

	}
    core.flushGraphicsQueue();
    return 0;
}
