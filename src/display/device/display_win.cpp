/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <pangolin/platform.h>
#include <pangolin/gl/glinclude.h>
#include <pangolin/display/display.h>
#include <pangolin/display/display_internal.h>

#include <windowsx.h>

namespace pangolin
{

extern __thread PangolinGl* context;

const char *className = "Pangolin";

WNDCLASS wndClass;
HWND hWnd;
HDC hDC;
HGLRC hGLRC;
HPALETTE hPalette;

void
setupPixelFormat(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),  /* size */
        1,                              /* version */
        PFD_SUPPORT_OPENGL |
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,               /* support double-buffering */
        PFD_TYPE_RGBA,                  /* color type */
        24,                             /* prefered color depth */
        0, 0, 0, 0, 0, 0,               /* color bits (ignored) */
        8,                              /* alpha bits */
        0,                              /* alpha shift (ignored) */
        0,                              /* no accumulation buffer */
        0, 0, 0, 0,                     /* accum bits (ignored) */
        32,                             /* depth buffer */
        0,                              /* no stencil buffer */
        0,                              /* no auxiliary buffers */
        PFD_MAIN_PLANE,                 /* main layer */
        0,                              /* reserved */
        0, 0, 0,                        /* no layer, visible, damage masks */
    };
    int pixelFormat;

    pixelFormat = ChoosePixelFormat(hDC, &pfd);
    if (pixelFormat == 0) {
        MessageBox(WindowFromDC(hDC), "ChoosePixelFormat failed.", "Error",
            MB_ICONERROR | MB_OK);
        exit(1);
    }

    if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE) {
        MessageBox(WindowFromDC(hDC), "SetPixelFormat failed.", "Error",
            MB_ICONERROR | MB_OK);
        exit(1);
    }
}

void
setupPalette(HDC hDC)
{
    int pixelFormat = GetPixelFormat(hDC);
    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE* pPal;
    int paletteSize;

    DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    if (pfd.dwFlags & PFD_NEED_PALETTE) {
        paletteSize = 1 << pfd.cColorBits;
    }
    else {
        return;
    }

    pPal = (LOGPALETTE*)
        malloc(sizeof(LOGPALETTE) + paletteSize * sizeof(PALETTEENTRY));
    pPal->palVersion = 0x300;
    pPal->palNumEntries = paletteSize;

    /* build a simple RGB color palette */
    {
        int redMask = (1 << pfd.cRedBits) - 1;
        int greenMask = (1 << pfd.cGreenBits) - 1;
        int blueMask = (1 << pfd.cBlueBits) - 1;
        int i;

        for (i = 0; i<paletteSize; ++i) {
            pPal->palPalEntry[i].peRed =
                (((i >> pfd.cRedShift) & redMask) * 255) / redMask;
            pPal->palPalEntry[i].peGreen =
                (((i >> pfd.cGreenShift) & greenMask) * 255) / greenMask;
            pPal->palPalEntry[i].peBlue =
                (((i >> pfd.cBlueShift) & blueMask) * 255) / blueMask;
            pPal->palPalEntry[i].peFlags = 0;
        }
    }

    hPalette = CreatePalette(pPal);
    free(pPal);

    if (hPalette) {
        SelectPalette(hDC, hPalette, FALSE);
        RealizePalette(hDC);
    }
}

unsigned char GetPangoKey(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case VK_F1: return PANGO_SPECIAL + PANGO_KEY_F1;
    case VK_F2: return PANGO_SPECIAL + PANGO_KEY_F2;
    case VK_F3: return PANGO_SPECIAL + PANGO_KEY_F3;
    case VK_F4: return PANGO_SPECIAL + PANGO_KEY_F4;
    case VK_F5: return PANGO_SPECIAL + PANGO_KEY_F5;
    case VK_F6: return PANGO_SPECIAL + PANGO_KEY_F6;
    case VK_F7: return PANGO_SPECIAL + PANGO_KEY_F7;
    case VK_F8: return PANGO_SPECIAL + PANGO_KEY_F8;
    case VK_F9: return PANGO_SPECIAL + PANGO_KEY_F9;
    case VK_F10: return PANGO_SPECIAL + PANGO_KEY_F10;
    case VK_F11: return PANGO_SPECIAL + PANGO_KEY_F11;
    case VK_F12: return PANGO_SPECIAL + PANGO_KEY_F12;
    case VK_LEFT: return PANGO_SPECIAL + PANGO_KEY_LEFT;
    case VK_UP: return PANGO_SPECIAL + PANGO_KEY_UP;
    case VK_RIGHT: return PANGO_SPECIAL + PANGO_KEY_RIGHT;
    case VK_DOWN: return PANGO_SPECIAL + PANGO_KEY_DOWN;
    case VK_HOME: return PANGO_SPECIAL + PANGO_KEY_HOME;
    case VK_END: return PANGO_SPECIAL + PANGO_KEY_END;
    case VK_INSERT: return PANGO_SPECIAL + PANGO_KEY_INSERT;
    case VK_DELETE: return 127;
    default:
        const int lBufferSize = 2;
        WCHAR lBuffer[lBufferSize];

        BYTE State[256];
        GetKeyboardState(State);

        const UINT scanCode = (lParam >> 8) & 0xFFFFFF00;
        if( ToUnicode(wParam, scanCode, State, lBuffer, lBufferSize, 0) >=1 ) {
            return lBuffer[0];
        }
    }
    return 0;
}

LRESULT APIENTRY
WndProc(
HWND hWnd,
UINT message,
WPARAM wParam,
LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        /* initialize OpenGL rendering */
        hDC = GetDC(hWnd);
        setupPixelFormat(hDC);
        setupPalette(hDC);
        hGLRC = wglCreateContext(hDC);
        wglMakeCurrent(hDC, hGLRC);
        //init();
        return 0;
    case WM_DESTROY:
        /* finish OpenGL rendering */
        if (hGLRC) {
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(hGLRC);
        }
        if (hPalette) {
            DeleteObject(hPalette);
        }
        ReleaseDC(hWnd, hDC);
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        /* track window size changes */
        if (hGLRC) {
            process::Resize((int)LOWORD(lParam), (int)HIWORD(lParam));
            return 0;
        }
    case WM_PALETTECHANGED:
        /* realize palette if this is *not* the current window */
        if (hGLRC && hPalette && (HWND)wParam != hWnd) {
            UnrealizeObject(hPalette);
            SelectPalette(hDC, hPalette, FALSE);
            RealizePalette(hDC);
            //redraw();
            break;
        }
        break;
    case WM_QUERYNEWPALETTE:
        /* realize palette if this is the current window */
        if (hGLRC && hPalette) {
            UnrealizeObject(hPalette);
            SelectPalette(hDC, hPalette, FALSE);
            RealizePalette(hDC);
            //redraw();
            return TRUE;
        }
        break;
    case WM_PAINT:
    {
        //PAINTSTRUCT ps;
        //BeginPaint(hWnd, &ps);
        //if (hGLRC) {
        //    redraw();
        //}
        //EndPaint(hWnd, &ps);
        //return 0;
    }
        break;
    case WM_KEYDOWN:
    {
        unsigned char key = GetPangoKey(wParam, lParam);
        if(key>0) process::Keyboard(key, 1, 1);
        return 0;
    }
    case WM_KEYUP:
    {
        unsigned char key = GetPangoKey(wParam, lParam);
        if (key>0) process::KeyboardUp(key, 1, 1);
        return 0;
    }
    case WM_LBUTTONDOWN:
        process::Mouse(0, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    case WM_MBUTTONDOWN:
        process::Mouse(1, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    case WM_RBUTTONDOWN:
        process::Mouse(2, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_LBUTTONUP:
        process::Mouse(0, 1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    case WM_MBUTTONUP:
        process::Mouse(1, 1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    case WM_RBUTTONUP:
        process::Mouse(2, 1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_MOUSEMOVE:
        if (wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON) ) {
            process::MouseMotion(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        } else{
            process::PassiveMouseMotion(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        return 0;

    case WM_MOUSEWHEEL:
        process::Scroll(0.0f, GET_WHEEL_DELTA_WPARAM(wParam) / 5.0 );
        return 0;
    case WM_MOUSEHWHEEL:
        process::Scroll(GET_WHEEL_DELTA_WPARAM(wParam) / 5.0, 0.0f);
        return 0;
    default:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void ProcessQueuedWinMessages()
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            pangolin::Quit();
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void FinishFrame()
{
    RenderViews();
    PostRender();
    SwapBuffers(hDC);
    ProcessQueuedWinMessages();
}

void CreateWindowAndBind(std::string window_title, int w, int h, const Params& /*params*/ )
{
    // Create Pangolin GL Context
    BindToContext(window_title);
    context->is_double_buffered = true;

    // Create Window
    const HMODULE hCurrentInst = GetModuleHandle(0);
    const int nCmdShow = SW_SHOW;

    /* register window class */
    wndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hCurrentInst;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = className;
    RegisterClass(&wndClass);

    hWnd = CreateWindow(
        className, window_title.c_str(),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, w, h,
        NULL, NULL, hCurrentInst, NULL);

    // Display Window
    ShowWindow(hWnd, nCmdShow);
    //UpdateWindow(hWnd);

    // Process Messages
    ProcessQueuedWinMessages();
    glewInit();
}

void StartFullScreen() {
    LONG dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE) 
        & ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
    LONG dwStyle = GetWindowLong(hWnd, GWL_STYLE) 
        & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);

    SetWindowLong(hWnd, GWL_EXSTYLE, dwExStyle);
    SetWindowLong(hWnd, GWL_STYLE, dwStyle);

    GLint prev[2];
    std::memcpy(prev, context->windowed_size, sizeof(prev));
    ShowWindow(hWnd, SW_SHOWMAXIMIZED);
    std::memcpy(context->windowed_size, prev, sizeof(prev));
}

void StopFullScreen() {

    ChangeDisplaySettings(NULL, 0);
    ShowCursor(TRUE);

    LONG dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    LONG dwStyle = GetWindowLong(hWnd, GWL_STYLE) | WS_OVERLAPPEDWINDOW;

    SetWindowLong(hWnd, GWL_EXSTYLE, dwExStyle);
    SetWindowLong(hWnd, GWL_STYLE, dwStyle);

    SetWindowPos(hWnd,
        HWND_TOP,
        0, 0,
        context->windowed_size[0], context->windowed_size[1],
        SWP_FRAMECHANGED);
}

void SetFullscreen(bool fullscreen)
{
    if( fullscreen != context->is_fullscreen )
    {
        if(fullscreen) {
            StartFullScreen();
        }else{
            StopFullScreen();
        }
        context->is_fullscreen = fullscreen;
    }
}

void PangolinPlatformInit(PangolinGl& /*context*/)
{
}

void PangolinPlatformDeinit(PangolinGl& /*context*/)
{
}

}
