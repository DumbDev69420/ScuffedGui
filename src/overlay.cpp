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


    RGBA DefaultColRgba;


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

void Overlay::begin_scene()
{
    tar->BeginDraw();
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

void Overlay::draw_text(int x, int y, const char* text, D2D1::ColorF color, ...)
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

    brush->SetColor(color);





    if (FormatOthers) {
        tar->DrawTextW(Conv.c_str(), length, formatOther, D2D1::RectF(x, y, window_metrics.right - window_metrics.left, window_metrics.bottom - window_metrics.top), brush);
    }
    else
    {
        tar->DrawTextW(Conv.c_str(), length, format, D2D1::RectF(x, y, window_metrics.right - window_metrics.left, window_metrics.bottom - window_metrics.top), brush);
    }



}

bool InAreaOfButton(const Item& item);


void Overlay::DrawMenu(D2D1::ColorF colorInner, D2D1::ColorF Outside) {

    RECT    window_metrics;

    if (!GetWindowRect(window, &window_metrics))
        return;

    PtrToOverlay = this;



    int x, y;

    x = WindowPos.x;
    y = WindowPos.y;

    D2D_POINT_2F Point1 = { x,y };

    D2D1_RECT_F Rect{ x, y, x + WindowWidht, y - WindowHeight };

    D2D1_ROUNDED_RECT RoundedRect = { Rect, 10.0f, 10.0f };


    D2D1_ROUNDED_RECT RoundedRectFF = RoundedRect;

    RoundedRectFF.rect.right -= 10;
    RoundedRectFF.rect.bottom += 10;
    RoundedRectFF.rect.top -= 10;
    RoundedRectFF.rect.left += 10;



    Item itemWindow;
    itemWindow.Pos = Rect;



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



    brush->SetColor(colorInner);

    tar->FillRoundedRectangle(RoundedRect, brush);


    brush->SetColor(Outside);
    tar->FillRoundedRectangle(RoundedRectFF, brush);





    draw_text(Rect.left + WindowWidht / 2 - MainMenuTextLenght * 6, Rect.bottom + 10, MainMenuText, D2D1::ColorF(D2D1::ColorF::Green));
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
        if (GetAsyncKeyState(VK_LBUTTON) && GetTickCount64() > ItemList[NumbCurrentItem].TickCount) {

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
    RectButton.right = WindowPos.x + sizex * sizeOther + 4;
    RectButton.bottom = WindowPos.y - WindowHeight + 50 + Spacing * NumbCurrentItem + 25;




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

    D2D1::ColorF ColBlack(D2D1::ColorF::Black);


    if (Hovered) {
        ColBlack.r += 0.1;
        ColBlack.g += 0.1;
        ColBlack.b += 0.1;
    }

    brush->SetColor(ColBlack);

    tar->FillRectangle(RectButton, brush);


    FormatOthers = true;
    draw_text(RectButton.left + 5, RectButton.top, Name, D2D1::ColorF(D2D1::ColorF::White));
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

    D2D1::ColorF ColBlack(0);


    ColBlack = (D2D1::ColorF::Black);


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
        D2D1::ColorF FKD = (D2D1::ColorF::WhiteSmoke);
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



    D2D1::ColorF Black(D2D1::ColorF::Black);


    brush->SetColor(Black);


    tar->FillRectangle(RectButton, brush);



    float Hight = RectButton.top - RectButton.bottom;
    Hight /= 2;

    FormatOthers = true;

    std::string ValuenShit = Name; ValuenShit += ": " + std::to_string(*Value);

    draw_text(RectButton.right + 20, (RectButton.bottom + Hight) - 10, ValuenShit.c_str(), D2D1::ColorF::White);

    FormatOthers = false;

    D2D1_POINT_2F Point1 = { RectButton.left, RectButton.bottom + Hight };

    D2D1_POINT_2F Point2 = { RectButton.right, RectButton.bottom + Hight };

    tar->DrawLine(Point1, Point2, brush);


    RectButton.left += SliderPos;
    RectButton.right = RectButton.left + 20;

    D2D1_ROUNDED_RECT Circle = { RectButton, 20.0f, 20.0f };

    tar->DrawRoundedRectangle(Circle, brush);

    return Pressed;
}


void Overlay::Spacing() {

    D2D_RECT_F RectButton{ 0.0f, 0.0f, 0.0f, 0.0f };

    if (AddItem(RectButton, std::string("EMPTY" + std::to_string(SpacingAmount)).c_str(), true)) {
        return;
    }
}
