#include "Src/Foundation/Window.h"
#include <string>
#include "Src/Foundation/Core.h"
#include "Src/Foundation/ShaderManager.h"
#include "Src/Foundation/PSOManager.h"
#include "Src/Foundation/Plane.h"
#include "Src/Foundation/Timer.h"
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

    Plane plane;
    plane.init(&core, &psoManager, &shaderManager);

    while (true) {
		core.beginFrame();
		float dt = timer.dt();
		win.processMessages();
		if (win.keys[VK_ESCAPE] == 1)
		{
			break;
		}

		t += dt;
		Matrix vp;
		Matrix p = Matrix::perspective(0.01f, 10000.0f, 1024.0f / 1024.0f, 60.0f);
		Vec3 from = Vec3(11 * cos(t), 5, 11 * sinf(t));
		Matrix v = Matrix::lookAt(from, Vec3(0, 0, 0), Vec3(0, 1, 0));
		vp =  v * p;

		shaderManager.updateConstantVS("StaticModelUntextured", "staticMeshBuffer", "VP", &vp);
		
		core.beginRenderPass();

		plane.draw(&core, &psoManager, &shaderManager);

		core.finishFrame();
    }

    core.flushGraphicsQueue();
    return 0;
}
