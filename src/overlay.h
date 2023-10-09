#pragma once
#include <d2d1.h>
#include <dwmapi.h>
#include <dwrite.h>
#include <string>
#include <vector>
#include <windows.h>

#define WM_INPUT 0x00FF


class Item {
public:
    const char* Id = "";
    int Number = 0;
    D2D_RECT_F Pos;
    DWORD Delay = 200;
    DWORD TickCount = 0;
    class Overlay* OverlayPtr = 0x0;
};


struct DeVec2 {
    ULONG x, y;
};




class Overlay
{
public:
    bool Quit = false;
    bool LockWindow = false;
    int Menu = 0;
    tagPOINT MousePosF = { 0, 0 };

private:
    HWND window;

    ID2D1Factory*          d2d_factory;
    ID2D1HwndRenderTarget* tar;
    IDWriteFactory*        write_factory;
    ID2D1SolidColorBrush*  brush;
    IDWriteTextFormat*     format;
    IDWriteTextFormat*     formatOther;

    const wchar_t* font;
    float          size;
    float          sizeOther;

    float WindowWidht = 600;
    float WindowHeight = 420;

    int ItemCount = 0;
    int NumbCurrentItem = 0;
    DeVec2 WindowPos = { 500,500 };

    const char* MainMenuText = "Asphalt 8 Cheeto";

    const int MainMenuTextLenght = strnlen_s(MainMenuText, 100);

    std::vector<Item> ItemList;


public:
    Overlay(const wchar_t* Font, float Font_Size, float Font_SizeOther)
    {
        this->font = Font;
        this->size = Font_Size;
        this->sizeOther = Font_SizeOther;
    }

    ~Overlay()
    {
        begin_scene();
        clear_scene();
        end_scene();

        tar->Release();
        write_factory->Release();
        brush->Release();
        d2d_factory->Release();

        printf("tar: %p, write: %p, brush: %p, factory: %p", tar, write_factory, brush, d2d_factory);
    }

    bool init();
    bool startup_d2d();

    void begin_scene();
    void end_scene();
    void clear_scene();

    /* create helper functions here */
    void draw_text(int x, int y, const char* text, D2D1::ColorF color, ...);
    void DrawMenu(D2D1::ColorF color);
    bool AddItem(D2D1_RECT_F Rect, const char* id);
    bool Button(const char* Name);
    void ButtonBehaviour(bool* Pressed, bool* hovered, const char* id);
    bool CheckBox(const char* Name, bool *Value);
    bool SliderFloat(const char* Name, float* Value, float min, float max);
    //LPMSG GetMessagesC();
};
