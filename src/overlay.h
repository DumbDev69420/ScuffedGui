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
    int WidthId = 0;
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
    int SpacingAmount = 0;
    tagPOINT MousePosF = { 0, 0 };
    DeVec2 WindowSize;

    //Colors




    //Menu Colors
    D3DCOLORVALUE MenuInnerColor = { 0.05882352941, 0.07843137254, 0.19215686274, 1 };
    D3DCOLORVALUE MenuOuterColor = { 0.09411764705, 0.11372549019, 0.23137254901, 1 };

    //Colors Button
    D3DCOLORVALUE InnerColor = { 54 / 255.0f, 119 / 255.0f, 175 / 255.0f, 255 / 255.0f };
    D3DCOLORVALUE Outercolor = { 38 / 255.0f, 54 / 255.0f, 88 / 255.0f, 255 / 255.0f };




    D3DCOLORVALUE InnerColorSoft = { 157 / 255.0f, 193 / 255.0f, 225 / 255.0f, 255 / 255.0f };
    

private:
    HWND window;

    ID2D1Factory* d2d_factory;
    ID2D1HwndRenderTarget* tar;
    IDWriteFactory* write_factory;
    ID2D1SolidColorBrush* brush;
    IDWriteTextFormat* format;
    IDWriteTextFormat* formatOther;

    const wchar_t* font;
    float          size;
    float          sizeOther;

    float WindowWidht = 600;
    float WindowHeight = 420;

    int DelayForward = 0;
    int ItemCountGlobal = 0;
    int ItemCount = 0;
    int NumbCurrentItem = 0;
    DeVec2 WindowPos = { 500,500 };

    const char* MainMenuText = "Template";

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
    void draw_text(int x, int y, const char* text, D3DCOLORVALUE color, bool Rainbow, ...);
    void DrawMenu();
    bool AddItem(D2D1_RECT_F Rect, const char* id, bool IgnoreList = false);
    bool Button(const char* Name);
    void ButtonBehaviour(bool* Pressed, bool* hovered, const char* id);
    bool CheckBox(const char* Name, bool* Value);
    bool SliderFloat(const char* Name, float* Value, float min, float max);
    void Spacing();
    void DrawWaterMark();
    bool LoadingScreen();
    //LPMSG GetMessagesC();
};
