#include <windows.h>
#include <Windowsx.h>
#include <xinput.h>
#include <glad/glad.h>
#include <stdio.h>

#include "Defines.h"
#include "Game.h"
#include "Input.h"


static bool gRunning;
static Input gInput;
static WORD XInputButtons[] = 
{
    XINPUT_GAMEPAD_DPAD_UP,
    XINPUT_GAMEPAD_DPAD_DOWN,
    XINPUT_GAMEPAD_DPAD_LEFT,
    XINPUT_GAMEPAD_DPAD_RIGHT,
    XINPUT_GAMEPAD_START,
    XINPUT_GAMEPAD_BACK,
    XINPUT_GAMEPAD_A,
    XINPUT_GAMEPAD_B,
    XINPUT_GAMEPAD_X,
    XINPUT_GAMEPAD_Y
};


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

static float ProcessXInputStick(WORD value, int deadZoneValue)
{
    float result = 0;
    if(value < -deadZoneValue)
    {
        result = (float)(value + deadZoneValue) / (32768.0f - deadZoneValue);
    }
    else if(value > deadZoneValue)
    {
        result = (float)(value - deadZoneValue) / (32767.0f - deadZoneValue);
    }
    return result;
}

static void ProcessInputAndMessages(Input *lastInput) {
    MSG msg = {};
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        switch(msg.message)
        {
            case WM_QUIT: {
                gRunning = false;      
            } break;
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP: { 
                bool wasDown = ((msg.lParam & (1 << 30)) != 0);
                bool isDown = ((msg.lParam & (1 << 31)) == 0);
                if(isDown != wasDown) {
                    DWORD vkCode = (DWORD)msg.wParam;
                    gInput.mKeys[vkCode].mIsDown = isDown;
                }
            }break;
            case WM_MOUSEMOVE: {
                gInput.mMouseX = (int)GET_X_LPARAM(msg.lParam); 
                gInput.mMouseY = (int)GET_Y_LPARAM(msg.lParam); 
            }break;
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP: {
                gInput.mMouseLeft.mIsDown = ((msg.wParam & MK_LBUTTON) != 0);
                gInput.mMouseMiddle.mIsDown = ((msg.wParam & MK_MBUTTON) != 0);
                gInput.mMouseRight.mIsDown = ((msg.wParam & MK_RBUTTON) != 0);
            }break;
            default: {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }break;
        } 
    }

    XINPUT_STATE state = {};
    if(XInputGetState(0, &state) == ERROR_SUCCESS)
    {
        XINPUT_GAMEPAD *pad = &state.Gamepad;
        for(int i = 0; i < ArrayCount(gInput.mJoyButtons); ++i)
        {
            gInput.mJoyButtons[i].mIsDown = pad->wButtons & XInputButtons[i];
        }
        gInput.mLeftStickX =  ProcessXInputStick(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        gInput.mLeftStickY =  ProcessXInputStick(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        gInput.mRightStickX = ProcessXInputStick(pad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        gInput.mRightStickY = ProcessXInputStick(pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    }
    else
    {
        for(int i = 0; i < ArrayCount(gInput.mJoyButtons); ++i)
        {
            gInput.mJoyButtons[i].mIsDown = false;
        }
        gInput.mLeftStickX = 0.0f; 
        gInput.mLeftStickY = 0.0f;
        gInput.mRightStickX = 0.0f;
        gInput.mRightStickY = 0.0f;
    }

    
    for(int i = 0; i < ArrayCount(gInput.mKeys); ++i) {
        if(lastInput->mKeys[i].mIsDown) {
            gInput.mKeys[i].mWasDown = true;
        }
        else {
            gInput.mKeys[i].mWasDown = false; 
        }
    }
    for(int i = 0; i < ArrayCount(gInput.mMouseButtons); ++i) {
        if(lastInput->mMouseButtons[i].mIsDown) {
            gInput.mMouseButtons[i].mWasDown = true;
        }
        else {
            gInput.mMouseButtons[i].mWasDown = false; 
        }
    }
    for(int i = 0; i < ArrayCount(gInput.mJoyButtons); ++i) {
        if(lastInput->mJoyButtons[i].mIsDown) {
            gInput.mJoyButtons[i].mWasDown = true;
        }
        else {
            gInput.mJoyButtons[i].mWasDown = false; 
        }
    }

    if(gInput.mJoyA.mIsDown != gInput.mJoyA.mWasDown) {
        if(gInput.mJoyA.mIsDown) {
            printf("A press\n");
        }
    }

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
    
    gRunning = true;
   
    Input lastInput = {};

    while(gRunning) {

        ProcessInputAndMessages(&lastInput);

        LARGE_INTEGER currentCounter = {};
        QueryPerformanceCounter(&currentCounter);

        //double fps = (double)frequency.QuadPart / (double)(currentCounter.QuadPart - lastCounter.QuadPart);
        float dt = (float)((double)(currentCounter.QuadPart - lastCounter.QuadPart) / (double)frequency.QuadPart);
        
        //printf("FPS: %lf\n", fps);

        game.Update(dt);

        glClearColor(1.0f, 0.6f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

        game.Render();

        SwapBuffers(hdc);
        if(vsynch != 0) {
            glFinish();
        }

        lastCounter = currentCounter;
        lastInput = gInput;
    }

    game.Shutdown();

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);
    
    return 0;
}
