/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011-2018 Steven Lovegrove, Andrey Mnatsakanov
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

#include <pangolin/factory/factory_registry.h>
#include <pangolin/platform.h>
#include <pangolin/gl/glinclude.h>
#include <pangolin/windowing/WinWindow.h>
#include <memory>

#define CheckWGLDieOnError() pangolin::_CheckWLDieOnError( __FILE__, __LINE__ );
namespace pangolin {
inline void _CheckWLDieOnError( const char *sFile, const int nLine )
{
    DWORD errorCode = GetLastError();
    if(errorCode!=0) {
        LPVOID lpMsgBuf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL);
        // MessageBox( NULL, (LPCTSTR)lpMsgBuf, ("Error "+std::to_string(errorCode)).c_str(), MB_OK | MB_ICONINFORMATION );
        pango_print_error("Error %i: %ws", errorCode, (WCHAR *)lpMsgBuf);
        pango_print_error("In: %s, line %d\n", sFile, nLine);
        // exit(EXIT_FAILURE);
    }
}
}

namespace pangolin
{

const char *className = "Pangolin";

////////////////////////////////////////////////////////////////////////
// Utils
////////////////////////////////////////////////////////////////////////

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
        if( ToUnicode((UINT)wParam, scanCode, State, lBuffer, lBufferSize, 0) >=1 ) {
            return (unsigned char)lBuffer[0];
        }
    }
    return 0;
}

KeyModifierBitmask GetMouseModifierKey(WPARAM wParam)
{
    KeyModifierBitmask mask;
    if (wParam & MK_SHIFT) mask |= KeyModifierShift;
    if (wParam & MK_CONTROL) mask |= KeyModifierCtrl;
    if (HIBYTE(GetKeyState(VK_MENU))) mask |= KeyModifierCmd;
    return mask;
}

////////////////////////////////////////////////////////////////////////
// WinWindow Implementation
////////////////////////////////////////////////////////////////////////

void WinWindow::SetupPixelFormat(HDC hDC)
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
        MessageBoxA(WindowFromDC(hDC), "ChoosePixelFormat failed.", "Error", MB_ICONERROR | MB_OK);
        CheckWGLDieOnError();
        exit(1);
    }

    if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE) {
        MessageBoxA(WindowFromDC(hDC), "SetPixelFormat failed.", "Error", MB_ICONERROR | MB_OK);
        CheckWGLDieOnError();
        exit(1);
    }
}

void WinWindow::SetupPalette(HDC hDC)
{
    int pixelFormat = GetPixelFormat(hDC);
    if(!pixelFormat) {
        std::cerr << "GetPixelFormat() failed" << std::endl;
        CheckWGLDieOnError();
    }

    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE* pPal;
    int paletteSize;

    if(!DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd)) {
        std::cerr << "DescribePixelFormat() failed" << std::endl;
        CheckWGLDieOnError();
    }

    if (pfd.dwFlags & PFD_NEED_PALETTE) {
        paletteSize = 1 << pfd.cColorBits;
    }
    else {
        return;
    }

    pPal = (LOGPALETTE*) malloc(sizeof(LOGPALETTE) + paletteSize * sizeof(PALETTEENTRY));
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
    else {
        std::cerr << "CreatePalette() failed" << std::endl;
    }
}

WinWindow::WinWindow(
    const std::string& window_title, int width, int height
) : hWnd(0), bIsFullscreen(false)
{
    const HMODULE hCurrentInst = GetModuleHandleA(nullptr);
    if(hCurrentInst==NULL) {
        std::cerr << "GetModuleHandle() failed" << std::endl;
        CheckWGLDieOnError();
    }
    RegisterThisClass(hCurrentInst);

    HWND thishwnd = CreateWindowA(
        className, window_title.c_str(),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, width, height,
        NULL, NULL, hCurrentInst, this);
    if(thishwnd==NULL) {
        std::cerr << "CreateWindow() failed" << std::endl;
        CheckWGLDieOnError();
    }

    if( thishwnd != hWnd ) {
        throw std::runtime_error("Pangolin Window Creation Failed.");
    }

    // Display Window
    ShowWindow(hWnd, SW_SHOW);
}

WinWindow::~WinWindow()
{
    if(IsWindow(hWnd) && !DestroyWindow(hWnd)) {
        std::cerr << "DestroyWindow() failed" << std::endl;
        CheckWGLDieOnError();
    }
}

void WinWindow::RegisterThisClass(HMODULE hCurrentInst)
{
    WNDCLASSA wndClass;
    wndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WinWindow::WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hCurrentInst;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = className;
    if(!RegisterClassA(&wndClass)) {
        std::cerr << "RegisterClass() failed" << std::endl;
        CheckWGLDieOnError();
    }
}

LRESULT APIENTRY
WinWindow::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WinWindow* self = 0;

    if (uMsg == WM_NCCREATE) {
        auto lpcs = reinterpret_cast<LPCREATESTRUCTA>(lParam);
        self = reinterpret_cast<WinWindow*>(lpcs->lpCreateParams);
        if(self) {
            self->hWnd = hwnd;
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(self));
        }
    } else {
        self = reinterpret_cast<WinWindow*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->HandleWinMessages(uMsg, wParam, lParam);
    } else {
        return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
}

LRESULT WinWindow::HandleWinMessages(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        /* initialize OpenGL rendering */
        hDC = GetDC(hWnd);
        if(hDC==NULL) {
            std::cerr << "WM_CREATE GetDC() failed" << std::endl;
        }
        SetupPixelFormat(hDC);
        SetupPalette(hDC);
        hGLRC = wglCreateContext(hDC);
        if(!hGLRC) {
            std::cerr << "WM_CREATE wglCreateContext() failed" << std::endl;
            CheckWGLDieOnError();
        }
        if(!wglMakeCurrent(hDC, hGLRC)) {
            std::cerr << "WM_CREATE wglMakeCurrent() failed" << std::endl;
            CheckWGLDieOnError();
        }
        return 0;
    case WM_CLOSE:
    case WM_QUIT:
        if(!DestroyWindow(hWnd)) {
            std::cerr << "DestroyWindow() failed" << std::endl;
            CheckWGLDieOnError();
        }
        return 0;
    case WM_DESTROY:
        /* finish OpenGL rendering */
        if (hGLRC) {
            if(!wglMakeCurrent(NULL, NULL)) {
                std::cerr << "WM_DESTROY wglMakeCurrent() failed" << std::endl;
                CheckWGLDieOnError();
            }
            if(!wglDeleteContext(hGLRC)) {
                std::cerr << "WM_DESTROY wglDeleteContext() failed" << std::endl;
                CheckWGLDieOnError();
            }
        }
        if (hPalette) {
            DeleteObject(hPalette);
        }
        ReleaseDC(hWnd, hDC);
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        ResizeSignal(WindowResizeEvent({(int)LOWORD(lParam), (int)HIWORD(lParam)}));
        return 0;
    case WM_PALETTECHANGED:
        /* realize palette if this is *not* the current window */
        if (hGLRC && hPalette && (HWND)wParam != hWnd) {
            if(!UnrealizeObject(hPalette)) {
                std::cerr << "WM_PALETTECHANGED UnrealizeObject() failed" << std::endl;
            }
            if(!SelectPalette(hDC, hPalette, FALSE)) {
                std::cerr << "WM_PALETTECHANGED SelectPalette() failed" << std::endl;
            }
            if(RealizePalette(hDC)==GDI_ERROR) {
                std::cerr << "WM_PALETTECHANGED RealizePalette() failed" << std::endl;
            }
            //redraw();
            break;
        }
        break;
    case WM_QUERYNEWPALETTE:
        /* realize palette if this is the current window */
        if (hGLRC && hPalette) {
            if(!UnrealizeObject(hPalette)) {
                std::cerr << "WM_QUERYNEWPALETTE UnrealizeObject() failed" << std::endl;
            }
            if(!SelectPalette(hDC, hPalette, FALSE)) {
                std::cerr << "WM_QUERYNEWPALETTE SelectPalette() failed" << std::endl;
            }
            if(RealizePalette(hDC)==GDI_ERROR) {
                std::cerr << "WM_QUERYNEWPALETTE RealizePalette() failed" << std::endl;
            }
            return TRUE;
        }
        break;
    case WM_PAINT:
        break;
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
        unsigned char key = GetPangoKey(wParam, lParam);
        if(key > 0) {
            KeyboardSignal(KeyboardEvent({
                afLastMousePos[0], afLastMousePos[1],
                KeyModifierBitmask(),
                (unsigned char)key, message == WM_KEYDOWN
            }));
        }
        return 0;
    }
    case WM_LBUTTONDOWN:
        MouseSignal(MouseEvent({
            (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam),
            GetMouseModifierKey(wParam), 0, true
        }));
        return 0;
    case WM_MBUTTONDOWN:
        MouseSignal(MouseEvent({
            (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam),
            GetMouseModifierKey(wParam), 1, true
        }));
        return 0;
    case WM_RBUTTONDOWN:
        MouseSignal(MouseEvent({
            (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam),
            GetMouseModifierKey(wParam), 2, true
        }));
        return 0;
    case WM_LBUTTONUP:
        MouseSignal(MouseEvent({
            (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam),
            GetMouseModifierKey(wParam), 0, false
        }));
        return 0;
    case WM_MBUTTONUP:
        MouseSignal(MouseEvent({
            (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam),
            GetMouseModifierKey(wParam), 1, false
        }));
        return 0;
    case WM_RBUTTONUP:
        MouseSignal(MouseEvent({
            (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam),
            GetMouseModifierKey(wParam), 2, false
        }));
        return 0;
    case WM_MOUSEMOVE:
        if (wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON) ) {
            MouseMotionSignal(MouseMotionEvent({
                (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam),
                GetMouseModifierKey(wParam),
            }));
        } else{
            PassiveMouseMotionSignal(MouseMotionEvent({
                (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam),
                GetMouseModifierKey(wParam),
            }));
        }
        afLastMousePos[0] = (float)GET_X_LPARAM(lParam);
        afLastMousePos[1] = (float)GET_Y_LPARAM(lParam); 
        return 0;
    case WM_MOUSEWHEEL:
        SpecialInputSignal(SpecialInputEvent({
            afLastMousePos[0], afLastMousePos[1],
            GetMouseModifierKey(wParam), InputSpecialScroll,
            {0.0f, GET_WHEEL_DELTA_WPARAM(wParam) / 5.0f, 0.0f, 0.0f }
        }));
        return 0;
    case WM_MOUSEHWHEEL:
        SpecialInputSignal(SpecialInputEvent({
            afLastMousePos[0], afLastMousePos[1],
            GetMouseModifierKey(wParam), InputSpecialScroll,
            {GET_WHEEL_DELTA_WPARAM(wParam) / 5.0f, 0.0f, 0.0f, 0.0f }
        }));
        return 0;
    default:
        break;
    }
    return DefWindowProcA(hWnd, message, wParam, lParam);
}

void WinWindow::StartFullScreen() {
    LONG dwExStyle = GetWindowLongA(hWnd, GWL_EXSTYLE)
        & ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
    if(dwExStyle==0) {
        std::cerr << "GetWindowLongA() failed" << std::endl;
        CheckWGLDieOnError();
    }
    LONG dwStyle = GetWindowLongA(hWnd, GWL_STYLE)
        & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    if(dwStyle==0) {
        std::cerr << "GetWindowLongA() failed" << std::endl;
        CheckWGLDieOnError();
    }

    if(!SetWindowLongA(hWnd, GWL_EXSTYLE, dwExStyle)) {
        std::cerr << "SetWindowLongA() failed" << std::endl;
        CheckWGLDieOnError();
    }
    if(!SetWindowLongA(hWnd, GWL_STYLE, dwStyle)) {
        std::cerr << "SetWindowLongA() failed" << std::endl;
        CheckWGLDieOnError();
    }

    // Save windowed size so that we can un-fullscreen to
    // to this dimension
    GetWindowRect(hWnd, &cWindowedRect);
    ShowWindow(hWnd, SW_SHOWMAXIMIZED);
}

void WinWindow::StopFullScreen() {
    ChangeDisplaySettings(NULL, 0);
    ShowCursor(TRUE);

    LONG dwExStyle = GetWindowLongA(hWnd, GWL_EXSTYLE) | WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    LONG dwStyle = GetWindowLongA(hWnd, GWL_STYLE) | WS_OVERLAPPEDWINDOW;

    if(dwExStyle==0) {
        std::cerr << "GetWindowLongA() failed" << std::endl;
        CheckWGLDieOnError();
    }
    if(dwStyle==0) {
        std::cerr << "GetWindowLongA() failed" << std::endl;
        CheckWGLDieOnError();
    }

    if(!SetWindowLongA(hWnd, GWL_EXSTYLE, dwExStyle)) {
        std::cerr << "SetWindowLongA() failed" << std::endl;
        CheckWGLDieOnError();
    }
    if(!SetWindowLongA(hWnd, GWL_STYLE, dwStyle)) {
        std::cerr << "SetWindowLongA() failed" << std::endl;
        CheckWGLDieOnError();
    }

    // Restore previous size
    Resize(cWindowedRect.right-cWindowedRect.left, cWindowedRect.bottom-cWindowedRect.top);
    Move(cWindowedRect.left, cWindowedRect.top);
}

void WinWindow::ShowFullscreen(const TrueFalseToggle on_off)
{
    const bool target = to_bool(on_off, bIsFullscreen);
    if(target != bIsFullscreen) {
        if(target) {
            StartFullScreen();
        }else{
            StopFullScreen();
        }
    }
    bIsFullscreen = target;
}

void WinWindow::Move(int x, int y)
{
    if( !SetWindowPos(hWnd, 0, x, y, 0, 0, SWP_NOSIZE) ) {
        std::cerr << "WinWindow::Move failed" << std::endl;
        CheckWGLDieOnError();
    }
}

void WinWindow::Resize(unsigned int w, unsigned int h)
{
    if( !SetWindowPos(hWnd, 0, 0, 0, w, h, SWP_NOMOVE) ) {
        std::cerr << "WinWindow::Resize failed" << std::endl;
        CheckWGLDieOnError();
    }
}

void WinWindow::MakeCurrent()
{
    if(wglMakeCurrent(hDC, hGLRC)==FALSE) {
        std::cerr << "wglMakeCurrent() failed" << std::endl;
        CheckWGLDieOnError();
    }

    RECT rect;
    if(!GetWindowRect(hWnd, &rect)) {
        std::cerr << "GetWindowRect() failed" << std::endl;
        CheckWGLDieOnError();
    }
    Resize(rect.right - rect.left, rect.bottom - rect.top);
}

void WinWindow::RemoveCurrent()
{
    if(wglMakeCurrent(NULL, NULL)==0) {
        std::cerr << "wglMakeCurrent() failed" << std::endl;
        CheckWGLDieOnError();
    }
}

void WinWindow::SwapBuffers()
{
    if(!::SwapBuffers(hDC)) {
        std::cerr << "SwapBuffers() failed" << std::endl;
        CheckWGLDieOnError();
    }
}

void WinWindow::ProcessEvents()
{
    static bool needs_init = true;
    if(needs_init) {
        RECT cRect;
        GetClientRect(hWnd, &cRect);
        ResizeSignal(WindowResizeEvent({cRect.right, cRect.bottom}));
        needs_init = false;
    }

    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            CloseSignal();
            break;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

std::unique_ptr<WindowInterface> CreateWinWindowAndBind(std::string window_title, int w, int h)
{
    WinWindow* win = new WinWindow(window_title, w, h);

    return std::unique_ptr<WindowInterface>(win);
}

PANGOLIN_REGISTER_FACTORY(WinWindow)
{
  struct WinWindowFactory : public TypedFactoryInterface<WindowInterface> {
      std::map<std::string,Precedence> Schemes() const override
      {
          return {{"winapi",10}, {"default",100}};
      }
      const char* Description() const override
      {
          return "Use Windows native window";
      }
      ParamSet Params() const override
      {
          return {{
              {"window_title","window","Title of application Window"},
              {"w","640","Requested window width"},
              {"h","480","Requested window height"}
          }};
      }
    std::unique_ptr<WindowInterface> Open(const Uri& uri) override {

      const std::string window_title = uri.Get<std::string>("window_title", "window");
      const int w = uri.Get<int>("w", 640);
      const int h = uri.Get<int>("h", 480);
      return std::unique_ptr<WindowInterface>(CreateWinWindowAndBind(window_title, w, h));
    }
  };

  return FactoryRegistry::I()->RegisterFactory<WindowInterface>(std::make_shared<WinWindowFactory>());
}

}
