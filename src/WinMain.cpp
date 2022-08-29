#include <windows.h>
#include <glad/glad.h>
#include <stdio.h>

#include "Game.h"

#if _APP_DEBUG
    #pragma comment ( linker, "/subsystem:console" )
    int main(int argc, const char **argv) {
        return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWDEFAULT);
    }
#else
    #pragma comment ( linker, "/subsystem:windows" )
#endif


#define WGL_CONTEXT_MAJOR_VERSION_ARB    0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB    0x2092
#define WGL_CONTEXT_FLAGS_ARB            0x2094
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB     0x9126
typedef HGLRC(WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)
             (HDC, HGLRC, const int *);

typedef const char *(WINAPI *PFNWGLGETEXTENSIONSSTRINGEXTPROC) (void); 
typedef BOOL (WINAPI *PFNWGLSWAPINTERVALEXTPROC) (int);
typedef int (WINAPI *PFNWGLGETSWAPINTERVALEXTPROC) (void);

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
    switch(iMsg) {
        case WM_CLOSE: {
            DestroyWindow(hwnd);
        }break;
        case WM_DESTROY: {
            PostQuitMessage(0);
        }break;
    }
    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
	WNDCLASSEX wndclass = {};
	wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wndclass.lpszMenuName = 0;
    wndclass.lpszClassName = "Win32 Game Window";
    RegisterClassEx(&wndclass);
	
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int clientWidth = 800;
    int clientHeight = 600;
    RECT windowRect = {};
    SetRect(&windowRect,
            (screenWidth / 2) - (clientWidth / 2),
            (screenHeight / 2) - (clientHeight / 2),
            (screenWidth / 2) + (clientWidth / 2),
            (screenHeight / 2) + (clientHeight / 2));

    DWORD style = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX; // WS_THICKFRAME to resize

    AdjustWindowRectEx(&windowRect, style, FALSE, 0);
    HWND hwnd = CreateWindowEx(0, wndclass.lpszClassName,
                "Game Window", style, 
                windowRect.left, windowRect.top,
                windowRect.right - windowRect.left,
                windowRect.bottom - windowRect.top,
                NULL, NULL, hInstance, szCmdLine);
    HDC hdc = GetDC(hwnd);
    
    PIXELFORMATDESCRIPTOR pfd = {};
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);

    HGLRC tempRC = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempRC);
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    const int attribList[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB, 0,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    HGLRC hglrc = wglCreateContextAttribsARB(hdc, 0, attribList);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempRC);
    wglMakeCurrent(hdc, hglrc);

    if(!gladLoadGL()) {
        printf("Could not initialize GLAD\n");
    } else {
        printf("OpenGL Version %d.%d\n", GLVersion.major, GLVersion.minor);
    }

    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
    bool swapControlSupported = strstr(_wglGetExtensionStringEXT(), "WGL_EXT_swap_control") != 0;

    int vsynch = 0;
    if(swapControlSupported) {
        PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
        if(wglSwapIntervalEXT(1)) {
            printf("Enabled vsynch\n");
            vsynch = wglGetSwapIntervalEXT();
        } else {
            printf("Could not enable vsynch\n");
        }
    } else {
        printf("WGL_EXT_swap_control not supported\n");
    }
	
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    clientWidth = clientRect.right - clientRect.left;
    clientHeight = clientRect.bottom - clientRect.top;

    glViewport(0, 0, clientWidth, clientHeight);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    glPointSize(5.0f);

    LARGE_INTEGER frequency = {};
    QueryPerformanceFrequency(&frequency);

    Game game = {};

    game.Initialize();

    LARGE_INTEGER lastCounter = {};
    QueryPerformanceCounter(&lastCounter);

    MSG msg;
    while(true) {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if(msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        LARGE_INTEGER currentCounter = {};
        QueryPerformanceCounter(&currentCounter);

        double fps = (double)frequency.QuadPart / (double)(currentCounter.QuadPart - lastCounter.QuadPart);
        float dt = (float)((double)(currentCounter.QuadPart - lastCounter.QuadPart) / (double)frequency.QuadPart);
        
        printf("FPS: %lf\n", fps);

        game.Update(dt);

        glClearColor(1.0f, 0.6f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

        game.Render();

        SwapBuffers(hdc);
        if(vsynch != 0) {
            glFinish();
        }

        lastCounter = currentCounter;
    }

    game.Shutdown();

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);
    
    return (int)msg.wParam;
}
