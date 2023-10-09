#include "src/overlay.h"
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include "icl.h"
#include "proc.h"

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")



//Base by iraizo on github: https://github.com/iraizo/nvidia-overlay-hijack





void render() {}




int main()
{
    Overlay* overlay = new Overlay(L"Signika Negative", 28.0, 15.0);

    if (!overlay->init())
        return 1;
    if (!overlay->startup_d2d())
        return 1;

    bool DifText = false;
    bool Open = false;


    


    const char* WaterMarkText = "ScuffedGUI Made by Senpai42";

    while (!overlay->Quit && !GetAsyncKeyState(VK_DELETE))
    {
        

        GetCursorPos(&overlay->MousePosF);

        overlay->begin_scene();
        overlay->clear_scene();
        if (Open == true) {
            
            overlay->DrawMenu(D2D1::ColorF(D2D1::ColorF::Blue)); {
                if (overlay->Button("Exit  ")) {
                    overlay->Quit = true;
                }

            }


            
            
        }


        overlay->draw_text(10, 10, WaterMarkText, D2D1::ColorF(D2D1::ColorF::Green));

        overlay->end_scene();


        if (GetAsyncKeyState(VK_INSERT)& 1) {
            Open = !Open;
        }

    }



    delete overlay;

    return 0;
}
