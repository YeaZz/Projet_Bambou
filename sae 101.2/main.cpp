#include <iostream>
#include <ctime>
#include <windows.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include "ressources.cpp"

using namespace std;

//Boucle principale
int main(int argn, char* argv[])
{
    //Hide console
    HWND windowHandle = GetConsoleWindow();
    ShowWindow(windowHandle, SW_HIDE);

    //Initialisation fenetre SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        return 1;
    }
    SDL_Window* win = SDL_CreateWindow("Bambou Simulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (win == NULL)
    {
        return 1;
    }
    SDL_Renderer* rendu = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderPresent(rendu);

    //Polices
    TTF_Init();
    TTF_Font* font_12 = TTF_OpenFont("C:\\Windows\\Fonts\\LTYPEB.ttf", 12);
    TTF_Font* font_20 = TTF_OpenFont("C:\\Windows\\Fonts\\LTYPEB.ttf", 20);
    TTF_Font* font_22 = TTF_OpenFont("C:\\Windows\\Fonts\\LTYPEB.ttf", 22);

    //Couleurs
    SDL_Color white_color = { 255, 255, 255, 255 };
    SDL_Color dark_color = { 0, 0, 0, 255 };

    //Importation des images
    SDL_Texture* backgroud_texture = loadImage(rendu, "sprites/background.png", 255);
    SDL_Texture* panda_texture = loadImage(rendu, "sprites/panda.png", 255);
    SDL_Texture* fn_texture = loadImage(rendu, "sprites/feuille_nue.png", 255);
    SDL_Texture* fd_texture = loadImage(rendu, "sprites/feuille_droite.png", 255);
    SDL_Texture* fg_texture = loadImage(rendu, "sprites/feuille_gauche.png", 255);
    SDL_Texture* ui_closed = loadImage(rendu, "sprites/ui_closed.png", 255);
    SDL_Texture* ui_opened = loadImage(rendu, "sprites/ui_opened.png", 255);
    SDL_Texture* button_manual_false = loadImage(rendu, "sprites/button_manual_false.png", 255);
    SDL_Texture* button_manual_true = loadImage(rendu, "sprites/button_manual_true.png", 255);
    SDL_Texture* button_battery = loadImage(rendu, "sprites/button_battery.png", 255);
    SDL_Texture* info_jour = loadImage(rendu, "sprites/info_jour.png", 255);
    SDL_Texture* info_battery = loadImage(rendu, "sprites/info_battery.png", 255);
    SDL_Texture* info_battery_slot = loadImage(rendu, "sprites/info_battery_slot.png", 255);
    SDL_Texture* button_play = loadImage(rendu, "sprites/button_play.png", 255);
    SDL_Texture* button_pause = loadImage(rendu, "sprites/button_pause.png", 255);
    SDL_Texture* button_next = loadImage(rendu, "sprites/button_next.png", 255);
    SDL_Texture* button_reboot = loadImage(rendu, "sprites/button_reboot.png", 255);
    SDL_Texture* button_manual_false_hidden = loadImage(rendu, "sprites/button_manual_false.png", 75);
    SDL_Texture* button_next_hidden = loadImage(rendu, "sprites/button_next.png", 75);
    SDL_Texture* button_reboot_hidden = loadImage(rendu, "sprites/button_reboot.png", 75);
    SDL_Texture* bubble_charging = loadImage(rendu, "sprites/bubble_charging.png", 255);
    SDL_Texture* button_cutting_factor = loadImage(rendu, "sprites/button_cutting_factor.png", 255);
    SDL_Texture* button_cutting_factor_hidden = loadImage(rendu, "sprites/button_cutting_factor.png", 75);
    SDL_Texture* button_info = loadImage(rendu, "sprites/button_info.png", 255);
    SDL_Texture* ui_info = loadImage(rendu, "sprites/ui_info.png", 255);

    //Initialisation random
    srand(time_t(NULL));

    //Initialisation structures
    Garden garden;
    Panda panda;
    Statistics statistics;
    Ui ui;
    Mouse mouse;

    //Initialisation(ui, panda, garden);
    init_garden(garden, 20);
    init_statistics(garden, statistics);

    //Initialisation FPS
    Uint32 frameStart;
    int frameTime;

    initialisation(ui, panda, garden, statistics);

    SDL_Event event;
    while (RUN)
    {
        //Démmarage du programme
        frameStart = SDL_GetTicks();
        FPS_DISPLAY_DELAY += 1;

        if (ui.play && (FPS_DISPLAY_DELAY >= ui.speed))
        {
            //Accroissement du nombre de jours et d'FPS
            NB_DAYS += 1;
            FPS_DISPLAY_DELAY = 0;

            //Accroissement des bambous
            bambou_grows(garden);

            //Traitement du robot panda
            panda_charge(panda);

            //Reboot le panda
            panda.in_charge = false;

            //Mise à jour statistics
            update_statistics(garden, statistics);

            //Passage du panda selon l'algorithme choisis
            reduce_algorithm(garden, panda, ui, statistics);
            if (!panda.in_charge) { garden.bambou_slots[panda.position].size = 0; }
        }

        //Evenements SDL
        SDL_PollEvent(&event);
        switch (event.type)
        {
        case SDL_MOUSEMOTION:
            mouse.x = event.button.x;
            mouse.y = event.button.y;
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                mouse.left_press = false;
                mouse.left_release = true;
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                mouse.left_click = true;
                mouse.left_press = true;
                mouse.left_release = false;
            }
            break;

        case SDL_QUIT:
            backup_fin(ui, panda, garden, statistics);
            RUN = false;
            break;
        }

        //Interaction souris
        if (!ui.play)
        {
            if ((((mouse.x >= 15) && (mouse.x <= 66)) && ((mouse.y >= 653) && (mouse.y <= 703))) && (mouse.left_click))
            {
                ui.manual = !ui.manual;
            }
        }
        if (ui.manual)
        {
            if ((((mouse.x >= 75) && (mouse.x <= 126)) && ((mouse.y >= 653) && (mouse.y <= 703))) && (mouse.left_click))
            {
                NB_DAYS += 1;
                FPS_DISPLAY_DELAY = 0;
                panda.in_charge = true;
                bambou_grows(garden);
                panda_charge(panda);
                update_statistics(garden, statistics);
            }
            int nb = garden.nb_bambou, bambou_grove_size = BAMBOU_WIDTH * nb + BAMBOU_SEPARATION * nb - BAMBOU_SEPARATION, size_before_bambou = (WINDOW_WIDTH - bambou_grove_size) / 2;
            for (int i = 0; i < garden.nb_bambou; i++)
            {
                if (mouse.left_click)
                {
                    if ((mouse.x >= size_before_bambou + (i * (BAMBOU_WIDTH + BAMBOU_SEPARATION))) && (mouse.x < (size_before_bambou + BAMBOU_WIDTH) + (i * (BAMBOU_WIDTH + BAMBOU_SEPARATION))))
                    {
                        if ((!ui.settings && !(((mouse.x >= 75) && (mouse.x <= 126)) && ((mouse.y >= 653) && (mouse.y <= 703)))) && (panda.life > 0))
                        {
                            NB_DAYS += 1;
                            FPS_DISPLAY_DELAY = 0;
                            panda.in_charge = false;
                            bambou_grows(garden);
                            panda_charge(panda);
                            update_statistics(garden, statistics);
                            panda.position = i;
                            if (!panda.in_charge) { garden.bambou_slots[panda.position].size = 0; }
                        }
                    }
                }
            }
        }
        else
        {
            if ((((mouse.x >= 75) && (mouse.x <= 126)) && ((mouse.y >= 653) && (mouse.y <= 703))) && (mouse.left_click))
            {
                ui.play = !ui.play;
            }
            if (!ui.play)
            {
                if ((((mouse.x >= 135) && (mouse.x <= 186)) && ((mouse.y >= 653) && (mouse.y <= 703))) && (mouse.left_click))
                {
                    NB_DAYS += 1;
                    FPS_DISPLAY_DELAY = 0;
                    bambou_grows(garden);
                    panda_charge(panda);
                    update_statistics(garden, statistics);
                    panda.in_charge = false;
                    reduce_algorithm(garden, panda, ui, statistics);
                    if (!panda.in_charge) { garden.bambou_slots[panda.position].size = 0; }
                }
                if ((((mouse.x >= 195) && (mouse.x <= 246)) && ((mouse.y >= 653) && (mouse.y <= 703))) && (mouse.left_click))
                {
                    for (int i = 0; i < garden.nb_bambou; i++)
                    {
                        garden.bambou_slots[i].size = 0;
                    }
                    panda.life = 10;
                    panda.in_charge = true;
                    NB_DAYS = 0;
                    for (int i = 0; i < GRAPH_SIZE; i++)
                    {
                        statistics.max_size[i] = 0;
                        statistics.min_size[i] = 0;
                        statistics.moyenne[i] = 0;
                    }
                    statistics.all_time_max = 0;
                }
            }
        }
        if (ui.settings)
        {
            if ((((mouse.x >= 751) && (mouse.x <= 789)) && ((mouse.y >= 50) && (mouse.y <= 99))) && (mouse.left_click))
            {
                if (ui.algorithm == 2)
                {
                    ui.algorithm = 0;
                }
                else
                {
                    ui.algorithm += 1;
                }
            }
            if ((((mouse.x >= 1012) && (mouse.x <= 1050)) && ((mouse.y >= 50) && (mouse.y <= 99))) && (mouse.left_click))
            {
                if (ui.algorithm == 0)
                {
                    ui.algorithm = 2;
                }
                else
                {
                    ui.algorithm -= 1;
                }
            }
            if (ui.algorithm == 2)
            {
                if ((((mouse.x >= 828) && (mouse.x <= 864)) && ((mouse.y >= 138) && (mouse.y <= 187))) && (mouse.left_press))
                {
                    ui.cutting_factor -= 0.01;
                }
                else if ((((mouse.x >= 938) && (mouse.x <= 975)) && ((mouse.y >= 138) && (mouse.y <= 187))) && (mouse.left_press))
                {
                    ui.cutting_factor += 0.01;
                }
            }
            if ((((mouse.x >= 826) && (mouse.x <= 864)) && ((mouse.y >= 224) && (mouse.y <= 273))) && (mouse.left_click))
            {
                ui.bambou -= 1;
            }
            if ((((mouse.x >= 938) && (mouse.x <= 975)) && ((mouse.y >= 224) && (mouse.y <= 273))) && (mouse.left_click))
            {
                ui.bambou += 1;
            }
            if ((((mouse.x >= 826) && (mouse.x <= 975)) && ((mouse.y >= 290) && (mouse.y <= 339))) && (mouse.left_click))
            {
                for (int i = 0; i < garden.nb_bambou; i++)
                {
                    garden.bambou_slots[i].size = 0;
                }
                panda.life = 10;
                panda.in_charge = true;
                NB_DAYS = 0;
                init_garden(garden, ui.bambou);
                statistics.algorithm = ui.algorithm;
                statistics.maximum_size = 0;
                statistics.window_height = 0;
                statistics.x = ui.cutting_factor;
                init_statistics(garden, statistics);
                for (int i = 0; i < GRAPH_SIZE; i++)
                {
                    statistics.max_size[i] = 0;
                    statistics.min_size[i] = 0;
                    statistics.moyenne[i] = 0;
                }
                statistics.all_time_max = 0;
            }
            if ((((mouse.x >= 826) && (mouse.x <= 864)) && ((mouse.y >= 637) && (mouse.y <= 686))) && (mouse.left_click))
            {
                ui.speed += 5;
                ui.speed_displayed -= 5;
            }
            if ((((mouse.x >= 938) && (mouse.x <= 975)) && ((mouse.y >= 637) && (mouse.y <= 686))) && (mouse.left_click))
            {
                ui.speed -= 5;
                ui.speed_displayed += 5;
            }
        }

        //Reinitialisation de la souris
        mouse.left_click = false;

        //Mise a jour des données de l'UI
        update_ui(garden, panda, statistics, ui, mouse);

        //Affichage
        display_image(rendu, backgroud_texture, 0, 0, false);
        display_bambou(rendu, garden, panda, statistics, panda_texture, fn_texture, fd_texture, fg_texture, mouse);
        if (ui.settings)
        {
            display_image(rendu, ui_opened, 0, 0, false);
            display_graph(rendu, garden, statistics, statistics.max_size, 79, 5, 253);
            display_graph(rendu, garden, statistics, statistics.moyenne, 179, 17, 233);
            display_graph(rendu, garden, statistics, statistics.min_size, 238, 23, 233);
            display_all_time_max_size(rendu, statistics);
            char buffer[100];
            _itoa_s(statistics.window_height, buffer, 10);
            display_text(rendu, dark_color, font_12, buffer, 760, 381, true);
            _itoa_s(statistics.window_height / 2, buffer, 10);
            display_text(rendu, dark_color, font_12, buffer, 760, 475, true);
        }
        else
        {
            display_image(rendu, ui_closed, 0, 0, false);
        }
        if (ui.manual)
        {
            display_image(rendu, button_manual_true, 16, 654, false);
            display_image(rendu, button_battery, 76, 654, false);
            display_image(rendu, info_jour, 136, 654, false);
            display_image(rendu, info_battery, 246, 654, false);
            char buffer[100];
            _itoa_s(NB_DAYS, buffer, 10);
            display_text(rendu, white_color, font_20, buffer, 186, 676, true);
            for (int i = 0; i < panda.life; i++)
            {
                display_image(rendu, info_battery_slot, 272 + (i * 10), 682, false);
            }
            display_image(rendu, button_info, 406, 654, false);
            if (((mouse.x >= 406) && (mouse.x <= 456)) && ((mouse.y >= 654) && (mouse.y <= 704)))
            {
                display_image(rendu, ui_info, 0, 0, false);
            }
        }
        else
        {
            if (!ui.play)
            {
                display_image(rendu, button_manual_false, 16, 654, false);
                display_image(rendu, button_play, 76, 654, false);
                display_image(rendu, button_next, 136, 654, false);
                display_image(rendu, button_reboot, 196, 654, false);
            }
            else
            {
                display_image(rendu, button_manual_false_hidden, 16, 654, false);
                display_image(rendu, button_pause, 76, 654, false);
                display_image(rendu, button_next_hidden, 136, 654, false);
                display_image(rendu, button_reboot_hidden, 196, 654, false);
            }
            display_image(rendu, info_jour, 256, 654, false);
            display_image(rendu, info_battery, 366, 654, false);
            display_image(rendu, button_info, 526, 654, false);
            if (((mouse.x >= 526) && (mouse.x <= 576)) && ((mouse.y >= 654) && (mouse.y <= 704)))
            {
                display_image(rendu, ui_info, 0, 0, false);
            }
            char buffer[100];
            _itoa_s(NB_DAYS, buffer, 10);
            display_text(rendu, white_color, font_20, buffer, 306, 676, true);
            for (int i = 0; i < panda.life; i++)
            {
                display_image(rendu, info_battery_slot, 392 + (i * 10), 682, false);
            }
        }
        if (panda.in_charge)
        {
            display_image(rendu, bubble_charging, 0, 0, false);
        }
        if (ui.settings)
        {
            if (ui.algorithm == 1)
            {
                char buffer[100] = "TALLEST";
                display_text(rendu, white_color, font_22, buffer, 901, 62, true);
            }
            else if (ui.algorithm == 2)
            {
                char buffer[100] = "FASTEST";
                display_text(rendu, white_color, font_22, buffer, 901, 62, true);
            }
            else
            {
                char buffer[100] = "RANDOM";
                display_text(rendu, white_color, font_22, buffer, 901, 62, true);
            }
            if (ui.algorithm == 2)
            {
                display_image(rendu, button_cutting_factor, 802, 116, false);

                int valeur = (int)(ui.cutting_factor * 100);
                if (valeur >= 100)
                {
                    char buffer[100];
                    _itoa_s(valeur, buffer, 10);
                    buffer[4] = '\0';
                    buffer[3] = buffer[2];
                    buffer[2] = buffer[1];
                    buffer[1] = '.';
                    display_text(rendu, white_color, font_20, buffer, 901, 151, true);
                }
                else if (valeur >= 10)
                {
                    char buffer[100];
                    _itoa_s(valeur, buffer, 10);
                    buffer[4] = '\0';
                    buffer[3] = buffer[1];
                    buffer[2] = buffer[0];
                    buffer[1] = '.';
                    buffer[0] = '0';
                    display_text(rendu, white_color, font_20, buffer, 901, 151, true);
                }
                else
                {
                    char buffer[100];
                    _itoa_s(valeur, buffer, 10);
                    buffer[4] = '\0';
                    buffer[3] = buffer[0];
                    buffer[2] = '0';
                    buffer[1] = '.';
                    buffer[0] = '0';
                    display_text(rendu, white_color, font_20, buffer, 901, 151, true);
                }
            }
            else
            {
                display_image(rendu, button_cutting_factor_hidden, 802, 116, false);
            }
            char buffer[100];
            _itoa_s(ui.bambou, buffer, 10);
            display_text(rendu, white_color, font_20, buffer, 901, 237, true);
            _itoa_s(ui.speed_displayed, buffer, 10);
            display_text(rendu, white_color, font_20, buffer, 901, 650, true);
        }

        //Refresh display
        SDL_RenderPresent(rendu);

        //Traitement des FPS
        frameTime = SDL_GetTicks() - frameStart;
        if (FRAME_DELAY > frameTime)
        {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    //Fermeture SDL
    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}