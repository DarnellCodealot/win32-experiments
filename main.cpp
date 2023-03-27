
#include <chrono>
#include <vector>
#include <thread>
#include <d2d1.h>
#include <wincodec.h>
#include <iostream>
#include <math.h>
#include <fstream>

struct Vec2 {
    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}
    float x;
    float y;
};

struct TexVertex {
    TexVertex() = default;
    TexVertex(Vec2 p, Vec2 t) : p(p), t(t) {}
    Vec2 p;
    Vec2 t;
};

struct Color {
    Color() = default;
    Color(float r, float g, float b) : r(r), g(g), b(b) {}
    float r;
    float g;
    float b;
};

struct Image {
    Image(int width, int height) : width(width), height(height) {
        pixel = new char[width * height * 3];
    };
    void FlipX(){
        char* pixelcopy = new char[width*height*3];
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                pixelcopy[(y * width + x) * 3] = pixel[(y * width + x) * 3];
                pixelcopy[(y * width + x) * 3 + 1] = pixel[(y * width + x) * 3 + 1];
                pixelcopy[(y * width + x) * 3 + 2] = pixel[(y * width + x) * 3 + 2];
            }
        }
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                pixel[(y * width + x) * 3] = pixelcopy[(y * width + width - x - 1) * 3];
                pixel[(y * width + x) * 3 + 1] = pixelcopy[(y * width + width - x - 1) * 3 + 1];
                pixel[(y * width + x) * 3 + 2] = pixelcopy[(y * width + width - x - 1) * 3 + 2];
            }
        }
    }
    void FlipY() {
        char* pixelcopy = new char[width * height * 3];
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                pixelcopy[(y * width + x) * 3] = pixel[(y * width + x) * 3];
                pixelcopy[(y * width + x) * 3 + 1] = pixel[(y * width + x) * 3 + 1];
                pixelcopy[(y * width + x) * 3 + 2] = pixel[(y * width + x) * 3 + 2];
            }
        }
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                pixel[(y * width + x) * 3] = pixelcopy[((height - y - 1) * width + x) * 3];
                pixel[(y * width + x) * 3 + 1] = pixelcopy[((height - y - 1) * width + x) * 3 + 1];
                pixel[(y * width + x) * 3 + 2] = pixelcopy[((height - y - 1) * width + x) * 3 + 2];
            }
        }
    }

    char* pixel;
    const int width;
    const int height;
};

int screenwidth;
int screenheight;
void* BitmapMemory;

void PutPixel(int x, int y, char r, char g, char b) {
    char* R = (char*)BitmapMemory;
    char* G = (char*)BitmapMemory;
    char* B = (char*)BitmapMemory;
    B += (y * screenwidth + x) * 4;
    G += (y * screenwidth + x) * 4 + 1;
    R += (y * screenwidth + x) * 4 + 2;
    *R = r;
    *G = g;
    *B = b;
}

void drawtrigpriv(float startY,float endY,float startX,float endX,float sXslope,float eXslope, char r, char g, char b) {
    for (int y = std::ceil(startY); y < std::ceil(endY); y++) {
        for (int x = std::ceil(startX); x < std::ceil(endX); x++) {
            PutPixel(x, y, r, g, b);
        }
        startX += sXslope;
        endX += eXslope;
    }
}

void drawtrigprivtex(float startY, float endY, float startX, float startXtx, float startXty, float endX, float endXtx, float endXty, float sXslope, float eXslope, float stexX, float stexY, float etexX, float etexY, Image texture) {
    for (int y = std::ceil(startY); y < std::ceil(endY); y++) {
        float xstepx = (endXtx - startXtx)/(endX - startX);
        float xstepy = (endXty - startXty)/(endX - startX);
        float sxc = startXtx;
        float syc = startXty;
        for (int x = std::ceil(startX); x < std::ceil(endX); x++) {
            char r = texture.pixel[(int(syc * texture.height) * texture.width + int(sxc * texture.width)) * 3];
            char g = texture.pixel[(int(syc * texture.height) * texture.width + int(sxc * texture.width)) * 3 + 1];
            char b = texture.pixel[(int(syc * texture.height) * texture.width + int(sxc * texture.width)) * 3 + 2];
            PutPixel(x, y, r, g, b);
            sxc += xstepx;
            syc += xstepy;
        }
        startX += sXslope;
        endX += eXslope;
        startXtx += stexX;
        startXty += stexY;
        endXtx += etexX;
        endXty += etexY;
    }
}

void DrawTriangle(float v0x, float v0y, float v1x, float v1y, float v2x, float v2y, char r, char g, char b) {
    if (v0y > v1y) {
        std::swap(v0y, v1y);
        std::swap(v0x, v1x);
    }
    if (v1y > v2y) {
        std::swap(v1y, v2y);
        std::swap(v1x, v2x);
    }
    if (v0y > v1y) {
        std::swap(v0y, v1y);
        std::swap(v0x, v1x);
    }
    if (v0y == v1y) {
        if (v0x > v1x) {
            std::swap(v0y, v1y);
            std::swap(v0x, v1x);
        }
        drawtrigpriv(v0y, v2y, v0x, v1x, (v2x - v0x) / (v2y - v0y), (v2x - v1x) / (v2y - v1y), r, g, b);
    }
    else if (v1y == v2y) {
        if (v1x > v2x) {
            std::swap(v1y, v2y);
            std::swap(v1x, v2x);
        }
        drawtrigpriv(v0y, v1y, v0x, v0x,  (v1x - v0x) / (v1y - v0y), (v2x - v0x) / (v2y - v0y), r, g, b);
    }
    else {
        float splitpx = v0x + (v2x-v0x)*(v1y-v0y)/(v2y-v0y);
        if (splitpx > v1x) {
            drawtrigpriv(v0y, v1y, v0x, v0x, (v1x - v0x)/(v1y - v0y), (splitpx - v0x) / (v1y - v0y), r, g, b);
            drawtrigpriv(v1y, v2y, v1x, splitpx, (v2x - v1x) / (v2y - v1y), (v2x - splitpx) / (v2y - v1y), r, g, b);
        }
        else {
            drawtrigpriv(v0y, v1y, v0x, v0x, (splitpx - v0x) / (v1y - v0y), (v1x - v0x) / (v1y - v0y), r, g, b);
            drawtrigpriv(v1y, v2y, splitpx, v1x, (v2x - splitpx) / (v2y - v1y), (v2x - v1x) / (v2y - v1y), r, g, b);
        }
    }
}

void DrawTriangleTex(float v0x, float v0y, float v0tx, float v0ty, float v1x, float v1y, float v1tx, float v1ty, float v2x, float v2y, float v2tx, float v2ty, Image texture) {
    if (v0y > v1y) {
        std::swap(v0y, v1y);
        std::swap(v0x, v1x);
        std::swap(v0ty, v1ty);
        std::swap(v0tx, v1tx);
    }
    if (v1y > v2y) {
        std::swap(v1y, v2y);
        std::swap(v1x, v2x);
        std::swap(v1ty, v2ty);
        std::swap(v1tx, v2tx);
    }
    if (v0y > v1y) {
        std::swap(v0y, v1y);
        std::swap(v0x, v1x);
        std::swap(v0ty, v1ty);
        std::swap(v0tx, v1tx);
    }
    if (v0y == v1y) {
        if (v0x > v1x) {
            std::swap(v0y, v1y);
            std::swap(v0x, v1x);
            std::swap(v0ty, v1ty);
            std::swap(v0tx, v1tx);
        }
        drawtrigprivtex(v0y, v2y, v0x, v0tx, v0ty, v1x, v1tx, v1ty, (v2x - v0x) / (v2y - v0y), (v2x - v1x) / (v2y - v1y), (v2tx-v0tx) / (v2y - v0y), (v2ty - v0ty) / (v2y - v0y), (v2tx - v1tx) / (v2y - v1y), (v2ty - v1ty) / (v2y - v1y), texture);
    }
    else if (v1y == v2y) {
        if (v1x > v2x) {
            std::swap(v1y, v2y);
            std::swap(v1x, v2x);
            std::swap(v1ty, v2ty);
            std::swap(v1tx, v2tx);
        }
        drawtrigprivtex(v0y, v1y, v0x, v0tx, v0ty, v0x, v0tx, v0ty, (v1x - v0x) / (v1y - v0y), (v2x - v0x) / (v2y - v0y), (v1tx - v0tx) / (v1y - v0y), (v1ty - v0ty) / (v1y - v0y), (v2tx - v0tx) / (v2y - v0y), (v2ty - v0ty) / (v2y - v0y), texture);
    }
    else {
        float splitpx = v0x + (v2x - v0x) * (v1y - v0y) / (v2y - v0y);
        float splitptx = v0tx + (v2tx - v0tx) * (v1y - v0y) / (v2y - v0y);
        float splitpty = v0ty + (v2ty - v0ty) * (v1y - v0y) / (v2y - v0y);
        if (splitpx > v1x) {
            drawtrigprivtex(v0y, v1y, v0x, v0tx, v0ty, v0x, v0tx, v0ty, (v1x - v0x) / (v1y - v0y), (splitpx - v0x) / (v1y - v0y), (v1tx - v0tx) / (v1y - v0y), (v1ty - v0ty) / (v1y - v0y), (splitptx - v0tx) / (v1y - v0y), (splitpty - v0ty) / (v1y - v0y), texture);
            drawtrigprivtex(v1y, v2y, v1x, v1tx, v1ty, splitpx, splitptx, splitpty, (v2x - v1x) / (v2y - v1y), (v2x - splitpx) / (v2y - v1y), (v2tx - v1tx) / (v2y - v1y), (v2ty - v1ty) / (v2y - v1y), (v2tx - splitptx) / (v2y - v1y), (v2ty - splitpty) / (v2y - v1y), texture);
        }
        else {
            drawtrigprivtex(v0y, v1y, v0x, v0tx, v0ty, v0x, v0tx, v0ty, (splitpx - v0x) / (v1y - v0y), (v1x - v0x) / (v1y - v0y), (splitptx - v0tx) / (v1y - v0y), (splitpty - v0ty) / (v1y - v0y), (v1tx - v0tx) / (v1y - v0y), (v1ty - v0ty) / (v1y - v0y), texture);
            drawtrigprivtex(v1y, v2y, splitpx, splitptx, splitpty, v1x, v1tx, v1ty, (v2x - splitpx) / (v2y - v1y), (v2x - v1x) / (v2y - v1y), (v2tx - splitptx) / (v2y - v1y), (v2ty - splitpty) / (v2y - v1y), (v2tx - v1tx) / (v2y - v1y), (v2ty - v1ty) / (v2y - v1y), texture);
        }
    }
}

void DrawTriangle(Vec2 v0, Vec2 v1, Vec2 v2, Color color) {
    DrawTriangle(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y, color.r, color.g, color.b);
}

void DrawTriangleTex(TexVertex v0, TexVertex v1, TexVertex v2, Image texture) {
    DrawTriangleTex(v0.p.x, v0.p.y, v0.t.x, v0.t.y, v1.p.x, v1.p.y, v1.t.x, v1.t.y, v2.p.x, v2.p.y, v2.t.x, v2.t.y, texture);
}

Image loadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);

    BITMAPFILEHEADER bmFileHeader;
    file.read(reinterpret_cast<char*>(&bmFileHeader), sizeof(bmFileHeader));

    BITMAPINFOHEADER bmInfoHeader;
    file.read(reinterpret_cast<char*>(&bmInfoHeader), sizeof(bmInfoHeader));

    const bool is32b = bmInfoHeader.biBitCount == 32;

    int width = bmInfoHeader.biWidth;
    int height;

    // test for reverse row order and control
    // y loop accordingly
    int yStart;
    int yEnd;
    int dy;
    if (bmInfoHeader.biHeight < 0)
    {
        height = -bmInfoHeader.biHeight;
        yStart = 0;
        yEnd = height;
        dy = 1;
    }
    else
    {
        height = bmInfoHeader.biHeight;
        yStart = height - 1;
        yEnd = -1;
        dy = -1;
    }

    Image imagedata(width, height);

    file.seekg(bmFileHeader.bfOffBits);
    // padding is for the case of of 24 bit depth only
    const int padding = (4 - (width * 3) % 4) % 4;

    for (int y = yStart; y != yEnd; y += dy)
    {
        for (int x = 0; x < width; x++)
        {
            imagedata.pixel[(y * width + x)*3 + 2] = file.get();
            imagedata.pixel[(y * width + x) * 3 + 1] = file.get();
            imagedata.pixel[(y * width + x) * 3 + 0] = file.get();
            if (is32b)
            {
                file.seekg(1, std::ios::cur);
            }
        }
        if (!is32b)
        {
            file.seekg(padding, std::ios::cur);
        }
    }
    return imagedata;
}

LRESULT CALLBACK myWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    case WM_PAINT:
        //BeginPaint(hWnd, )
        TextOutA(GetDC(hWnd), 0,0, "FoodMorning", 11);
        break;
    }
    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

int main() {
    //FreeConsole();
    auto hInstance = GetModuleHandle(NULL);
    WNDCLASSEXA ds = {
        sizeof(WNDCLASSEXA),
        CS_OWNDC,
        myWndProc,
        0,
        0,
        hInstance,
        NULL,
        NULL,
        NULL,
        NULL,
        "winclass",
        NULL
    };
    RegisterClassExA(&ds);
    HWND hwnd = CreateWindowA("winclass", "ALL", WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, 100, 100, 1000,800, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, SW_SHOW);

    HDC hdc = GetDC(hwnd);
    RECT clientrect;
    GetClientRect(hwnd, &clientrect);
    screenwidth = clientrect.right - clientrect.left;
    screenheight = clientrect.bottom - clientrect.top;
    BITMAPINFO BitmapInfo;
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = screenwidth;
    // Negative height makes top left as the coordinate system origin for the DrawPixel function, otherwise its bottom left
    BitmapInfo.bmiHeader.biHeight = -screenheight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;
    
    BitmapMemory = new char[screenwidth * screenheight * 4];

    std::chrono::steady_clock::time_point timeneverwaits = std::chrono::steady_clock::now();

    MSG msg{};
    msg.message = WM_NULL;
    auto next_frame = std::chrono::steady_clock::now();
    while (true) {
        next_frame += std::chrono::milliseconds(1000 / 60);
        //taking care of window messages
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                break;
            }
        }
        auto tp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timeneverwaits).count();
        
        for (int x = 0; x < screenwidth; x++) {
            for (int y = 0; y < screenheight; y++) {
                PutPixel(x, y, 0, 0, 0);
            }
        }

        StretchDIBits(hdc, 0, 0, screenwidth, screenheight, 0, 0, screenwidth, screenheight, BitmapMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        std::this_thread::sleep_until(next_frame);
    }
    return msg.wParam;
}