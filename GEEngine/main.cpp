#include "Src/Foundation/Window.h"
#include <string>
#include "Src/Foundation/Core.h"
#include "Src/Foundation/ScreenSpaceTriangle.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int width = 1024, height = 1024;

    Window win;
    win.create(0, 0, width, height, "My Window");

    Core core(win.hwnd, width, height);

    ScreenSpaceTriangle tri(&core);

    while (true) {
        core.beginFrame();
        win.processMessages();
        tri.draw(&core);

        if (win.keys[VK_ESCAPE]) break;

        core.finishFrame();
    }

    core.flushGraphicsQueue();
    return 0;
}
