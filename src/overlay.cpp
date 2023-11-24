#include "overlay.h"
#include <comdef.h>
#include <corecrt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

namespace NeededFuncs {

    unsigned long createRGB(int r, int g, int b)
    {
        return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
    }




    typedef struct
    {
        DWORD R;
        DWORD G;
        DWORD B;
        DWORD A;
    }RGBA;


    D3DCOLORVALUE ConvertRGBAToD3DCOLORVALUE(const RGBA& rgba)
    {
        D3DCOLORVALUE color;
        color.r = rgba.R / 255.0f;
        color.g = rgba.G / 255.0f;
        color.b = rgba.B / 255.0f;
        color.a = rgba.A / 255.0f;
        return color;
    }


    RGBA DefaultColRgba;
    RGBA Rainbowshit = { 0, 0,0,255 };
    D3DCOLORVALUE RainbowTextColor;


    int Rainbowify(RGBA* Color) {

        if (Color->B != 255 && Color->G == 0) {
            Color->B++;
        }

        if (Color->B == 255 && Color->R != 0) {
            Color->R--;

        }


        if (Color->B == 255 && Color->G != 255 && Color->R == 0) {
            Color->G++;

        }


        if (Color->G == 255 && Color->B != 0) {
            Color->B--;

        }

        if (Color->B == 0 && Color->R != 255) {
            Color->R++;

        }

        if (Color->R == 255 && Color->G != 0) {
            Color->G--;

        }

        return 1;
    }


    std::string wstringToString(const std::wstring& wstr)
    {
        std::string str;
        for (wchar_t c : wstr) {
            str += static_cast<char>(c);
        }
        return str;
    }

    std::wstring stringToWideString(const std::string& str)
    {
        std::wstring wstr(str.length(), L' ');
        std::copy(str.begin(), str.end(), wstr.begin());
        return wstr;
    }
}





bool Overlay::init()
{
    window = FindWindowA("CEF-OSC-WIDGET", "NVIDIA GeForce Overlay");
    

    volatile HWND FFF = window;

    if (!window)
        return false;

    auto info = GetWindowLongA(window, -20);

    if (!info)
        return false;

    auto attrchange = SetWindowLongPtrA(window, -20, (LONG_PTR)(info | 0x20));

    if (!attrchange)
        return false;

    MARGINS margin;
    margin.cyBottomHeight = margin.cyTopHeight = margin.cxLeftWidth = margin.cxRightWidth = -1;

    if (DwmExtendFrameIntoClientArea(window, &margin) != S_OK)
        return false;

    if (!SetLayeredWindowAttributes(window, 0x000000, 0xFF, 0x02))
        return false;

    if (!SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, 0x0002 | 0x0001))
        return false;

    ShowWindow(window, SW_SHOW);

    return true;
}

bool Overlay::startup_d2d()
{
    RECT rc;
    if (HRESULT ret = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory) == S_OK)
    {
        if (ret = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)(&write_factory)) == S_OK)
        {
            /* not adding more checks here, this should all succeed. Should be pretty safe. */
            write_factory->CreateTextFormat(font, NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size, L"en-us", &format);
            write_factory->CreateTextFormat(font, NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, sizeOther, L"en-us", &formatOther);

            GetClientRect(window, &rc);

            if (ret = d2d_factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)), D2D1::HwndRenderTargetProperties(window, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)), &tar) == S_OK)
            {
                D2D1_BRUSH_PROPERTIES properties = { 1.0f, D2D1::Matrix3x2F::Identity() };

                tar->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), properties, &brush);

                return true;
            }
        }
    }
    return false;
}


Overlay* PtrToOverlay = 0x0;
bool FirstRainbowUpdate = true;

void Overlay::begin_scene()
{
    tar->BeginDraw();
    FirstRainbowUpdate = true;
}

void Overlay::end_scene()
{
    tar->EndDraw();
    ItemCountGlobal = 0;
}

void Overlay::clear_scene()
{
    tar->Clear();
}


bool FormatOthers = false;


void Overlay::draw_text(int x, int y, const char* text, D3DCOLORVALUE color, bool Rainbow = false, ...)
{

    char    buffer[4096];
    int     length;
    wchar_t out[256];
    RECT    window_metrics;

    if (!GetWindowRect(window, &window_metrics))
        return;

    va_list arg_list;
    va_start(arg_list, text);
    vsnprintf(buffer, sizeof(buffer), text, arg_list);
    va_end(arg_list);

    length = strlen(buffer);


    size_t convertedChars;

    mbstowcs_s(&convertedChars, out, buffer, length);


    std::wstring Conv = NeededFuncs::stringToWideString(text);


    if(!Rainbow)
    brush->SetColor(color);
    else {
        if (FirstRainbowUpdate) {
            FirstRainbowUpdate = false;
            NeededFuncs::Rainbowify(&NeededFuncs::Rainbowshit);
            NeededFuncs::RainbowTextColor = NeededFuncs::ConvertRGBAToD3DCOLORVALUE(NeededFuncs::Rainbowshit);
        }
        brush->SetColor(NeededFuncs::RainbowTextColor);
    }




    if (FormatOthers) {
        tar->DrawTextW(Conv.c_str(), length, formatOther, D2D1::RectF(x, y, window_metrics.right - window_metrics.left, window_metrics.bottom - window_metrics.top), brush);
    }
    else
    {
        tar->DrawTextW(Conv.c_str(), length, format, D2D1::RectF(x, y, window_metrics.right - window_metrics.left, window_metrics.bottom - window_metrics.top), brush);
    }



}


bool InAreaOfButton(const Item& item);


constexpr int WindowThickness = 5;



void Overlay::DrawMenu() {


    RECT    window_metrics;

    if (!GetWindowRect(window, &window_metrics))
        return;

    PtrToOverlay = this;


    int x, y;

    x = WindowPos.x;
    y = WindowPos.y;

    D2D_POINT_2F Point1 = { x,y };

    D2D1_RECT_F Rect{ x, y, x + WindowWidht, y - WindowHeight };

    D2D1_ROUNDED_RECT RoundedRect = { Rect, 2.0f, 2.0f };


    D2D1_ROUNDED_RECT RoundedRectFF = RoundedRect;

    RoundedRectFF.rect.right -= WindowThickness;
    RoundedRectFF.rect.bottom += WindowThickness;
    RoundedRectFF.rect.top -= WindowThickness;
    RoundedRectFF.rect.left += WindowThickness;




    if (!LockWindow) {
        //Right
        if (GetAsyncKeyState('U')) {
            WindowPos.x += 15;
        }


        //Left
        if (GetAsyncKeyState('I')) {
            WindowPos.x -= 15;
        }


        //Down
        if (GetAsyncKeyState('O')) {
            WindowPos.y -= 15;
        }


        //Up
        if (GetAsyncKeyState('P')) {
            WindowPos.y += 15;
        }
    }



    brush->SetColor(MenuInnerColor);

    tar->FillRoundedRectangle(RoundedRect, brush);


    brush->SetColor(MenuOuterColor);
    tar->FillRectangle(RoundedRectFF.rect, brush);





    draw_text(Rect.left + WindowWidht / 2 - MainMenuTextLenght * 6, Rect.bottom + 10, MainMenuText, D2D1::ColorF(D2D1::ColorF::White));
}



void Overlay::DrawWaterMark() {

    draw_text(10, 10, "ScuffedGUI Made by Senpai42", NeededFuncs::RainbowTextColor, true);
}


bool Overlay::AddItem(D2D1_RECT_F Rect, const char* id, bool IgnoreList) {


    for (size_t i = 0; i < ItemList.size(); i++)
    {

        if (ItemList[i].Id == id) {
            if (ItemList[i].Number != ItemCountGlobal) {
                if (ItemCountGlobal == 0) {
                    for (size_t i = 0; i < ItemList.size(); i++)
                    {
                        ItemList[i].Number = i;
                    }
                }
                else {
                    DelayForward = 500;
                    ItemList.clear();
                    SpacingAmount = 0;
                }
                ItemCountGlobal++;
                return true;
            }

            ItemList[i].Pos = Rect;
            ItemCountGlobal++;
            return false;
        }
    }



    if (IgnoreList)SpacingAmount++;

    Item New{ id, ItemCount, Rect };
    ItemList.push_back(New);
    ItemCount++;
    ItemCountGlobal++;

    return true;
}

/* //Nah just using GetAsyncKeyState
*
LPMSG Overlay::GetMessagesC() {
    LPMSG Msg = nullptr;
    if (GetMessage(Msg, window, WM_INPUT, WM_INPUT) == false)Quit = false;
    return Msg;
}*/



bool InAreaOfButton(const Item& item) { //Check if Mouse is in the Area of the Rectangle figure

    double distanceX = std::abs((item.Pos.left + item.Pos.right) / 2 - PtrToOverlay->MousePosF.x);


    double distanceY = std::abs((item.Pos.top + item.Pos.bottom) / 2 - PtrToOverlay->MousePosF.y);


    return (distanceX <= (item.Pos.right - item.Pos.left) / 2) &&
        (distanceY <= (item.Pos.bottom - item.Pos.top) / 2);
}

bool Slider = false;

//Behaviour stuff like if pressed or hovered
void Overlay::ButtonBehaviour(bool* Pressed, bool* hovered, const char* id) {


    bool Found = false;

    for (size_t i = 0; i < ItemList.size(); i++)
    {
        if (ItemList[i].Id == id) {
            NumbCurrentItem = i;
            Found = true;
            break;
        }
    }





    if (Found) {
        if (GetAsyncKeyState(VK_LBUTTON)) {

            if (InAreaOfButton(ItemList[NumbCurrentItem]))*hovered = true;

            if (Slider || GetTickCount64() > ItemList[NumbCurrentItem].TickCount) {
                Slider = false;

                if (DelayForward == 0) {
                    ItemList[NumbCurrentItem].TickCount = GetTickCount64() + ItemList[NumbCurrentItem].Delay;
                    if (InAreaOfButton(ItemList[NumbCurrentItem]))*Pressed = true;
                    return;
                }

                for (size_t i = 0; i < ItemList.size(); i++)
                {
                    ItemList[i].TickCount = GetTickCount64() + ItemList[i].Delay + DelayForward;
                }
                DelayForward = 0;
                return;
            }
        }
        else
        {
            if (InAreaOfButton(ItemList[NumbCurrentItem]))*hovered = true;
        }

    }



}



bool Overlay::Button(const char* Name) {

    RECT    window_metrics;

    if (!GetWindowRect(window, &window_metrics))
        return false;


    int sizex = 0;

    sizex = strnlen_s(Name, 100);

    //sizex = (strnlen_s(Name, 100) * format->GetFontStretch());



    bool Pressed = false;
    bool Hovered = false;
    ButtonBehaviour(&Pressed, &Hovered, Name);

    int Spacing = 30;



    D2D_RECT_F RectButton;
    RectButton.left = WindowPos.x + 20;
    RectButton.top = WindowPos.y - WindowHeight + 50 + Spacing * NumbCurrentItem;
    RectButton.right = WindowPos.x + sizex * 10 + 4;
    RectButton.bottom = WindowPos.y - WindowHeight + 50 + Spacing * NumbCurrentItem + 25;


    D2D1_ROUNDED_RECT RoundedRect = { RectButton, 5.0f, 5.0f };


    if (AddItem(RectButton, Name)) {
        return false;
    }



    /*
    NeededFuncs::Rainbowify(&NeededFuncs::DefaultColRgba);

    D2D1::ColorF ColorF(D2D1::ColorF::White);


    ColorF.r = NeededFuncs::DefaultColRgba.R;
    ColorF.g = NeededFuncs::DefaultColRgba.G;
    ColorF.b = NeededFuncs::DefaultColRgba.B;
    ColorF.a = 255;*/

    D3DCOLORVALUE ColBlack = InnerColor;


    if (Hovered) {
        ColBlack.r += 0.1;
        ColBlack.g += 0.1;
        ColBlack.b += 0.1;
    }

    brush->SetColor(ColBlack);

    tar->FillRoundedRectangle(RoundedRect, brush);


    FormatOthers = true;
    draw_text(RectButton.left + 10, RectButton.top, Name, D2D1::ColorF(D2D1::ColorF::White));
    FormatOthers = false;

    return Pressed;

}


bool Overlay::CheckBox(const char* Name, bool* Value) {

    RECT    window_metrics;

    if (!GetWindowRect(window, &window_metrics))
        return false;

    int sizex = 0;

    sizex = strnlen_s(Name, 100);

    //sizex = (strnlen_s(Name, 100) * format->GetFontStretch());



    bool Pressed = false;
    bool Hovered = false;
    ButtonBehaviour(&Pressed, &Hovered, Name);

    int Spacing = 30;



    D2D_RECT_F RectButton;
    RectButton.left = WindowPos.x + 20;
    RectButton.top = WindowPos.y - WindowHeight + 50 + Spacing * NumbCurrentItem;
    RectButton.right = WindowPos.x + 40;
    RectButton.bottom = WindowPos.y - WindowHeight + 50 + Spacing * NumbCurrentItem + 20;




    if (AddItem(RectButton, Name)) {
        return false;
    }

    D3DCOLORVALUE ColBlack(0);


    ColBlack = Outercolor;


    if (Hovered) {
        ColBlack.r += 0.1;
        ColBlack.g += 0.1;
        ColBlack.b += 0.1;
    }

    if (Pressed) {
        *Value = !*Value;
    }


    brush->SetColor(ColBlack);

    tar->FillRectangle(RectButton, brush);


    D2D1::ColorF Black(D2D1::ColorF::Black);


    brush->SetColor(Black);


    FormatOthers = true;
    draw_text(RectButton.right + 10, RectButton.top, Name, D2D1::ColorF(D2D1::ColorF::White));
    FormatOthers = false;


    if (*Value) {
        D3DCOLORVALUE FKD = InnerColor;
        D2D1_RECT_F CheckBoxD = RectButton;
        CheckBoxD.left += 5;
        CheckBoxD.top += 5;
        CheckBoxD.bottom -= 5;
        CheckBoxD.right -= 5;

        brush->SetColor(FKD);

        D2D1_ROUNDED_RECT CheckBoxD1{ CheckBoxD, 1.0, 1.0 };

        tar->FillRoundedRectangle(CheckBoxD1, brush);;
    }

    return Pressed;
}

const int SizeSlider = 120;

bool Overlay::SliderFloat(const char* Name, float* Value, float min, float max) {

    Slider = true;

    float Steps = (max / min) * 10;
    Steps /= SizeSlider;

    RECT    window_metrics;

    if (!GetWindowRect(window, &window_metrics))
        return false;

    float SliderPos = (*Value) / Steps;
    int sizex = 0;

    sizex = strnlen_s(Name, 100);

    //sizex = (strnlen_s(Name, 100) * format->GetFontStretch());

    bool Pressed = false;
    bool Hovered = false;

    ButtonBehaviour(&Pressed, &Hovered, Name);

    int Spacing = 30;


    D2D_RECT_F RectButton;
    RectButton.left = WindowPos.x + 20;
    RectButton.top = WindowPos.y - WindowHeight + 50 + Spacing * NumbCurrentItem;
    RectButton.right = WindowPos.x + 20 + SizeSlider;
    RectButton.bottom = WindowPos.y - WindowHeight + 50 + Spacing * NumbCurrentItem + 20;



    if (AddItem(RectButton, Name)) {
        return false;
    }

    if (Pressed) {
        *Value = ((MousePosF.x - RectButton.left) * Steps);
    }



    D3DCOLORVALUE Black = Outercolor;

    if (Hovered) {
        Black.r += 0.1;
        Black.g += 0.1;
        Black.b += 0.1;
    }


    brush->SetColor(Black);

    D2D1_ROUNDED_RECT RoumdedRect = { RectButton, 5.0f, 5.0f };

    tar->FillRoundedRectangle(RoumdedRect, brush);


    float Hight = RectButton.top - RectButton.bottom;
    Hight /= 2;

    FormatOthers = true;

    std::string ValuenShit = Name; ValuenShit += ": " + std::to_string(*Value);

    draw_text(RectButton.right + 20, (RectButton.bottom + Hight) - 10, ValuenShit.c_str(), D2D1::ColorF(D2D1::ColorF::White));

    FormatOthers = false;

    //ButtonSize
    RectButton.left += SliderPos + 5;
    RectButton.right = RectButton.left - 10;




    D2D1_ROUNDED_RECT DD = { RectButton, 5.0f, 5.0f };
    DD.rect.left = WindowPos.x + 20;
    DD.rect.right += 10;


    brush->SetColor(InnerColor);

    tar->FillRoundedRectangle(DD, brush);

    brush->SetColor(InnerColorSoft);

    D2D1_ROUNDED_RECT Circle = { RectButton, 5.0f, 5.0f };
    tar->FillRoundedRectangle(Circle, brush);

    return Pressed;
}


void Overlay::Spacing() {

    D2D_RECT_F RectButton{ 0.0f, 0.0f, 0.0f, 0.0f };

    if (AddItem(RectButton, std::string("EMPTY" + std::to_string(SpacingAmount)).c_str(), true)) {

        return;
    }
}


DeVec2 WinSize = { 450, 290 };
float PaddingX = 80;
float PaddingY = 10;
float LoadValue = 1.0f;
float Fade = 255.0f;


bool Overlay::LoadingScreen() {

    begin_scene();
    clear_scene();

    float FadeCol = Fade / 255.0f;

    D3DCOLORVALUE Col = MenuOuterColor;
    Col.a = FadeCol;

    brush->SetColor(Col);

    //Outer Area
    D2D_RECT_F RectButton{};
    RectButton.left = (WindowSize.x / 2) - WinSize.x / 2;
    RectButton.top = (WindowSize.y / 2) - WinSize.y / 2;
    RectButton.right = (WindowSize.x / 2) + WinSize.x / 2;
    RectButton.bottom = (WindowSize.y / 2) + WinSize.y / 2;

    D2D1_ROUNDED_RECT Circle = { RectButton, 5.0f, 5.0f };

    tar->FillRoundedRectangle(Circle, brush);



    //Inner Bar
    D2D_RECT_F RectButtonF = RectButton;

    float FFF = 20;

    RectButtonF.left += PaddingX;
    RectButtonF.right -= PaddingX;
    //RectButtonF.top = (RectButton.bottom + FFF) - (RectButton.top / 2 + PaddingY);
    //RectButtonF.bottom = (RectButton.bottom + FFF) - (RectButton.top / 2 - PaddingY);
    RectButtonF.top += ((RectButton.bottom - RectButton.top) / 2);
    RectButtonF.bottom = RectButtonF.top - FFF;

    D2D1_ROUNDED_RECT CircleF = { RectButtonF, 5.0f, 5.0f };


    Col = Outercolor;
    Col.a = FadeCol;

    brush->SetColor(Col);

    tar->FillRoundedRectangle(CircleF, brush);



    D2D1_ROUNDED_RECT InnerBar = CircleF;


    InnerBar.rect.right = CircleF.rect.left + ((CircleF.rect.right - CircleF.rect.left) * LoadValue / 100);


    Col = InnerColor;
    Col.a = FadeCol;
    brush->SetColor(Col);

    tar->FillRoundedRectangle(InnerBar, brush);

    FormatOthers = true;
    if (LoadValue < 100) {
        LoadValue += 0.125f;

        draw_text(RectButtonF.left, RectButtonF.top - 60, std::string("Loading: " + std::to_string((int)LoadValue) + "%").c_str(), D2D1::ColorF(D2D1::ColorF::White, FadeCol));
    }
    else
    {

        draw_text(RectButtonF.left, RectButtonF.top - 60, std::string("Done!: " + std::to_string((int)LoadValue) + "%").c_str(), D2D1::ColorF(D2D1::ColorF::White, FadeCol));
        FormatOthers = false;
        end_scene();


        if (Fade < 5.0f) return true;

        Fade -= 8.0f;
    }
    FormatOthers = false;

    end_scene();

    return false;
}
