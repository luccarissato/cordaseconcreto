#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define Rectangle Win32Rectangle
#define CloseWindow Win32CloseWindow
#define ShowCursor Win32ShowCursor
#include <windows.h>
#undef Rectangle
#undef CloseWindow
#undef ShowCursor
#undef DrawText
#undef DrawTextEx
#undef LoadImage

#include "window_icon.h"
#include <stdlib.h>

static HICON taskbarIcon = NULL;

void applyWindowTaskbarIcon(Image icon) {
    if (icon.data == NULL || icon.width <= 0 || icon.height <= 0) {
        return;
    }

    HWND windowHandle = (HWND)GetWindowHandle();
    if (windowHandle == NULL) {
        return;
    }

    Color* pixels = LoadImageColors(icon);
    if (pixels == NULL) {
        return;
    }

    int width = icon.width;
    int height = icon.height;
    int xorSize = width * height * 4;
    int andStride = ((width + 31) / 32) * 4;
    int andSize = andStride * height;
    unsigned char* xorMask = (unsigned char*)malloc((size_t)xorSize);
    unsigned char* andMask = (unsigned char*)calloc((size_t)andSize, 1);

    if (xorMask == NULL || andMask == NULL) {
        free(xorMask);
        free(andMask);
        UnloadImageColors(pixels);
        return;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Color pixel = pixels[(y * width) + x];
            int dst = (((height - 1 - y) * width) + x) * 4;
            xorMask[dst + 0] = pixel.b;
            xorMask[dst + 1] = pixel.g;
            xorMask[dst + 2] = pixel.r;
            xorMask[dst + 3] = pixel.a;
        }
    }

    HICON newIcon = CreateIcon(GetModuleHandle(NULL), width, height, 1, 32, andMask, xorMask);
    if (newIcon != NULL) {
        HICON oldBigIcon = (HICON)SendMessage(windowHandle, WM_SETICON, ICON_BIG, (LPARAM)newIcon);
        HICON oldSmallIcon = (HICON)SendMessage(windowHandle, WM_SETICON, ICON_SMALL, (LPARAM)newIcon);

        if (oldBigIcon != NULL && oldBigIcon != taskbarIcon) {
            DestroyIcon(oldBigIcon);
        }
        if (oldSmallIcon != NULL && oldSmallIcon != taskbarIcon && oldSmallIcon != oldBigIcon) {
            DestroyIcon(oldSmallIcon);
        }
        if (taskbarIcon != NULL) {
            DestroyIcon(taskbarIcon);
        }
        taskbarIcon = newIcon;
    }

    free(xorMask);
    free(andMask);
    UnloadImageColors(pixels);
}
#else
#include "window_icon.h"

void applyWindowTaskbarIcon(Image icon) {
    (void)icon;
}
#endif
