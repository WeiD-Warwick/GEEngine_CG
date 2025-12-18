#include <string>
#include "Src/Foundation/Window.h"
#include "Src/Foundation/Core.h"
#include "Src/Foundation/ShaderManager.h"
#include "Src/Foundation/PSOManager.h"
#include "Src/Foundation/Timer.h"

#include "Src/Polygon/Plane.h"
#include "Src/Polygon/ScreenSpaceTriangle.h"
#include "Src/Polygon/Cube.h"
#include "Src/Polygon/Sphere.h"
#include "Src/Polygon/Tree.h"
#include "Src/Polygon/AnimatedModel.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    Window win;
    win.create(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "My Window");

    Core core;
    core.init(win.hwnd, WINDOW_WIDTH, WINDOW_HEIGHT);

    ShaderManager shaderManager;
    PSOManager psoManager;
	TextureManager textureManager;
    Timer timer;
    float t = 0;

	// polygon
	ScreenSpaceTriangle triangle;
	triangle.init(&core, &psoManager, &shaderManager);
	Plane plane;
	plane.init(&core, &psoManager, &shaderManager);
	Cube cube;
	cube.init(&core, &psoManager, &shaderManager);
	Sphere sphere;
	sphere.init(&core, &psoManager, &shaderManager, 100, 100, 10);
	
	Tree tree;
	tree.init(&core, &psoManager, &shaderManager, "Src/Assets/Models/acacia_003.gem");

	AnimatedModel animatedModel;
	animatedModel.load(&core, &psoManager, &shaderManager, &textureManager, "Src/Assets/Models/TRex.gem");

	int option = 6;
	// render
	while (true) {
		core.beginFrame();
		float dt = timer.dt();
		t += dt;

		win.processMessages();
		if (win.keys[VK_ESCAPE] == 1) break;
		if (win.keys['1'] == 1) option = 1;
		if (win.keys['2'] == 1) option = 2;
		if (win.keys['3'] == 1) option = 3;
		if (win.keys['4'] == 1) option = 4;
		if (win.keys['5'] == 1) option = 5;
		if (win.keys['6'] == 1) option = 6;
		if (win.keys['7'] == 1) option = 7;

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
			Matrix W;
			cube.draw(&core, &psoManager, &shaderManager, t, W);
			break;
		}
		case 4: {
			core.beginRenderPass();
			sphere.draw(&core, &psoManager, &shaderManager, t);
			break;
		}
		case 5: {
			core.beginRenderPass();
			Matrix W;
			cube.draw(&core, &psoManager, &shaderManager, t, W);
			W = Matrix::translation(Vec3(5.0f, 0, 0));
			cube.draw(&core, &psoManager, &shaderManager, t, W);
			break;
		}

		case 6: {
			core.beginRenderPass();
			tree.draw(&core, &psoManager, &shaderManager, t);
			break;
		}
		case 7: {
			core.beginRenderPass();
			Matrix W = Matrix::scaling(Vec3(0.01f, 0.01f, 0.01f));
			animatedModel.update(dt);
			animatedModel.draw(&core, &psoManager, &shaderManager, &textureManager, t, W);
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
