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

#pragma once

#include <string>
#include <pangolin/platform.h>
#include <pangolin/windowing/window.h>
#include <windowsx.h>

namespace pangolin
{

struct WinWindow : public WindowInterface
{
    WinWindow(
        const std::string& title, int width, int height
    );

    ~WinWindow();

    void Move(int x, int y) override;

    void Resize(unsigned int w, unsigned int h) override;

    void ShowFullscreen(const TrueFalseToggle on_off) override;

    void MakeCurrent() override;

    void RemoveCurrent() override;

    void ProcessEvents() override;

    void SwapBuffers() override;

    HGLRC GetGLRenderContext()
    {
        return hGLRC;
    }

private:
    void StartFullScreen();
    void StopFullScreen();

    static LRESULT APIENTRY WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    LRESULT HandleWinMessages(UINT message, WPARAM wParam, LPARAM lParam);

    void RegisterThisClass(HMODULE hCurrentInst);

    void SetupPixelFormat(HDC hdc);

    void SetupPalette(HDC hDC);

    // Owns the Window
    HWND hWnd;
    HDC hDC;
    HGLRC hGLRC;
    HPALETTE hPalette;
    bool bIsFullscreen;
    RECT cWindowedRect;
    float afLastMousePos[2];
};

}
