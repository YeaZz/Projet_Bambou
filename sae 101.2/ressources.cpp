#include <iostream>
#include <ctime>
#include <windows.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <fstream>

using namespace std;

//Variables globales
const int WINDOW_WIDTH = 1080;
const int WINDOW_HEIGHT = 720;
bool RUN = true;
int NB_DAYS = 0, ALGORITHM_CHOICE = 0;
const int BAMBOU_MAX = 20, COEFFICIENT_GROWTH_MAX = 100;
const int BAMBOU_WIDTH = 30, BAMBOU_SEPARATION = 20;
const int GRAPH_SIZE = 8;
SDL_Rect sdl_bambou[BAMBOU_MAX][BAMBOU_MAX];
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;
int FPS_DISPLAY_DELAY = 0;

//Struct
struct Bambou
{
	int coefficient_growth = 0;
	int size = 0;
	string textures = "";
};
struct Garden
{
	int nb_bambou = 0;
	Bambou bambou_slots[BAMBOU_MAX];
};
struct Panda
{
	int position = 0;
	int life = 10;
	bool in_charge = true;
};
struct Statistics
{
	int algorithm = 2;
	int window_height = 0;
	int maximum_size = 0;
	double x = 1.45;
	int max_size[GRAPH_SIZE] = { 0 };
	int min_size[GRAPH_SIZE] = { 0 };
	int moyenne[GRAPH_SIZE] = { 0 };
	int all_time_max = 0;
};
struct Ui
{
	bool manual = false;
	bool play = false;
	bool settings = false;
	int algorithm = 1;
	int bambou = 20;
	int speed = 15;
	int speed_displayed = 50;
	double cutting_factor = 1.45;
};
struct Mouse
{
	int x = 0;
	int y = 0;
	bool left_click = false;
	bool left_press = false;
	bool left_release = true;
};

//Fonctions
void update_ui(Garden& garden, Panda& panda, Statistics& statistics, Ui& ui, Mouse& mouse)
{
	if (ui.settings)
	{
		if (mouse.x < 718)
		{
			ui.settings = false;
		}
	}
	else
	{
		if (mouse.x >= 1038)
		{
			ui.settings = true;
		}
	}
	if (ui.cutting_factor >= 10)
	{
		ui.cutting_factor = 9.99;
	}
	if (ui.cutting_factor <= 1)
	{
		ui.cutting_factor = 1.00;
	}
	if (ui.bambou >= 20)
	{
		ui.bambou = 20;
	}
	if (ui.bambou <= 1)
	{
		ui.bambou = 1;
	}
	if (ui.speed >= 60)
	{
		ui.speed = 60;
	}
	if (ui.speed <= 5)
	{
		ui.speed = 5;
	}
	if (ui.speed_displayed >= 60)
	{
		ui.speed_displayed = 60;
	}
	if (ui.speed_displayed <= 5)
	{
		ui.speed_displayed = 5;
	}
}
void panda_charge(Panda& panda)
{
	if (panda.life <= 0)
	{
		panda.in_charge = true;
	}
	if (panda.in_charge)
	{
		panda.life += 3;
	}
	else
	{
		panda.life -= 1;
	}
	if (panda.life > 10)
	{
		panda.life = 10;
	}
}
void statistics_end_day(Garden& garden, Statistics& stats) {
	int max_temp = stats.maximum_size, i;
	for (i = 0; i < garden.nb_bambou; i++)
	{
		if (garden.bambou_slots[i].size > max_temp)
		{
			stats.maximum_size = max_temp;
		}
	}
}

void init_garden(Garden& garden, int nb_bambou)
{
	garden.nb_bambou = nb_bambou;
	int random_leave;
	for (int i = 0; i < garden.nb_bambou; i++)
	{
		Bambou& bambou = garden.bambou_slots[i];
		bambou.coefficient_growth = (rand() % COEFFICIENT_GROWTH_MAX) + 1;
		for (int i = 0; i < 50; i++) {
			random_leave = rand() % 100;
			if (random_leave > 75)
			{
				bambou.textures.push_back('1');
			}
			else if (random_leave > 50)
			{
				bambou.textures.push_back('2');
			}
			else
			{
				bambou.textures.push_back('0');
			}
		}
	}
}

void init_statistics(Garden& garden, Statistics& statistics)
{
	int somme = 0;
	for (int i = 0; i < garden.nb_bambou; i++)
	{
		somme += garden.bambou_slots[i].coefficient_growth;
	}
	statistics.window_height = somme * 2;
}
void reduce_algorithm(Garden& garden, Panda& panda, Ui& ui, Statistics& statistics)
{
	if (panda.life <= 0)
	{
		panda.in_charge = true;
	}
	if (!panda.in_charge)
	{
		if (statistics.algorithm == 1)
		{
			//Algoirithme du max
			int max = 0, position = 0;
			for (int i = 0; i < garden.nb_bambou; i++)
			{
				if (garden.bambou_slots[i].size > max)
				{
					max = garden.bambou_slots[i].size;
					position = i;
				}
			}
			panda.position = position;
		}
		else if (statistics.algorithm == 2)
		{
			//Algorithme de Reduce-Fastest
			double x = statistics.x;
			int fastest = 0, i, storpos = 0;
			int limite_coupe = (int)(x * (statistics.window_height / 2.0));
			bool passed = false;

			for (i = 0; i < garden.nb_bambou; i++) {
				if (garden.bambou_slots[i].size > limite_coupe)
				{
					if (garden.bambou_slots[i].coefficient_growth > fastest)
					{
						passed = true;
						fastest = garden.bambou_slots[i].coefficient_growth;
						storpos = i;
					}
				}
			}
			panda.position = storpos;
			if (!passed)
			{
				panda.in_charge = true;
			}
		}
		else
		{
			//Algorithme alÃ©atoire (le piiire)
			panda.position = rand() % (garden.nb_bambou + 1);
		}
	}
}

void bambou_grows(Garden& garden)
{
	for (int i = 0; i < garden.nb_bambou; i++)
	{
		garden.bambou_slots[i].size += garden.bambou_slots[i].coefficient_growth;
	}
}

SDL_Texture* loadText(SDL_Renderer* rendu, const char* text, SDL_Color coul, TTF_Font* font)
{
	SDL_Surface* surface = TTF_RenderText_Solid(font, text, coul);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(rendu, surface);
	SDL_FreeSurface(surface);
	return texture;
}

SDL_Texture* loadImage(SDL_Renderer* rendu, const char* name, int alpha)
{
	SDL_Surface* loadedImage = NULL;
	SDL_Texture* texture = NULL;
	loadedImage = IMG_Load(name);
	SDL_SetSurfaceAlphaMod(loadedImage, alpha);
	if (loadedImage != NULL)
	{
		texture = SDL_CreateTextureFromSurface(rendu, loadedImage);
		SDL_FreeSurface(loadedImage);
		loadedImage = NULL;
	}
	return texture;
}

void display_image(SDL_Renderer* rendu, SDL_Texture* texture, int x, int y, bool centered)
{
	if (centered)
	{
		SDL_Rect position;
		position.x = x;
		position.y = y;
		SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
		position.x = x - (position.w / 2);
		SDL_RenderCopy(rendu, texture, NULL, &position);
	}
	else
	{
		SDL_Rect position;
		position.x = x;
		position.y = y;
		SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
		SDL_RenderCopy(rendu, texture, NULL, &position);
	}
}

void display_text(SDL_Renderer* rendu, SDL_Color& color, TTF_Font* font, const char text[100], int x, int y, bool centered)
{
	SDL_Texture* text_image = loadText(rendu, text, color, font);
	display_image(rendu, text_image, x, y, centered);
}

void display_bambou(SDL_Renderer* rendu, Garden garden, Panda& panda, Statistics stats, SDL_Texture* panda_texture, SDL_Texture* fn_texture, SDL_Texture* fd_texture, SDL_Texture* fg_texture, Mouse& mouse)
{
	int nb = garden.nb_bambou, bambou_grove_size = BAMBOU_WIDTH * nb + BAMBOU_SEPARATION * nb - BAMBOU_SEPARATION, size_before_bambou = (WINDOW_WIDTH - bambou_grove_size) / 2;
	SDL_Rect rect;
	rect.w = BAMBOU_WIDTH;
	for (int i = 0; i < nb; i++)
	{
		rect.x = BAMBOU_WIDTH * i + BAMBOU_SEPARATION * i + size_before_bambou;
		rect.y = WINDOW_HEIGHT - garden.bambou_slots[i].size;

		//Affichage du robot panda
		if (!panda.in_charge)
		{
			if (panda.position == i)
			{
				display_image(rendu, panda_texture, ((BAMBOU_WIDTH * i + BAMBOU_SEPARATION * i + size_before_bambou) - 5) - 100, 10, false);
			}
		}
	}
	int bambou_updated_size;
	for (int i = 0; i < nb; i++)
	{
		//Affichage des bambous
		Bambou bambou = garden.bambou_slots[i];
		bambou_updated_size = bambou.size * (600.0 / stats.window_height);
		rect.h = bambou_updated_size;
		rect.x = BAMBOU_WIDTH * i + BAMBOU_SEPARATION * i + size_before_bambou;
		rect.y = WINDOW_HEIGHT - bambou_updated_size;
		SDL_SetRenderDrawColor(rendu, 27, 218, 113, 255);
		SDL_RenderFillRect(rendu, &rect);
	}
	char random_leave;
	for (int i = 0; i < garden.nb_bambou; i++)
	{
		//Affichage des feuilles
		Bambou bambou = garden.bambou_slots[i];
		bambou_updated_size = bambou.size * (600.0 / stats.window_height);
		rect.x = BAMBOU_WIDTH * i + BAMBOU_SEPARATION * i + size_before_bambou;
		rect.y = WINDOW_HEIGHT - bambou_updated_size;
		display_image(rendu, fn_texture, (BAMBOU_WIDTH * i + BAMBOU_SEPARATION * i + size_before_bambou) - 5, (WINDOW_HEIGHT - bambou_updated_size) - 6, false);
		float bambou_coefficient_growth_updated = bambou.coefficient_growth * (600.0 / stats.window_height);
		for (int j = 0; j < (bambou_updated_size / bambou_coefficient_growth_updated) - 1; j++)
		{
			random_leave = bambou.textures.at(j % 50);
			if (random_leave == '1')
			{
				display_image(rendu, fd_texture, (BAMBOU_WIDTH * i + BAMBOU_SEPARATION * i + size_before_bambou) - 5, (WINDOW_HEIGHT - bambou_updated_size - 6) + (int)(bambou_coefficient_growth_updated * (j + 1)), false);
			}
			else if (random_leave == '2')
			{
				display_image(rendu, fg_texture, (BAMBOU_WIDTH * i + BAMBOU_SEPARATION * i + size_before_bambou) - 24, (WINDOW_HEIGHT - bambou_updated_size - 6) + (int)(bambou_coefficient_growth_updated * (j + 1)), false);
			}
			else
			{
				display_image(rendu, fn_texture, (BAMBOU_WIDTH * i + BAMBOU_SEPARATION * i + size_before_bambou) - 5, (WINDOW_HEIGHT - bambou_updated_size - 6) + (int)(bambou_coefficient_growth_updated * (j + 1)), false);
			}
		}
	}
}

void backup_fin(Ui ui, Panda panda, Garden garden, Statistics& statistics) {
	string const nomFichier("backup.txt");
	ofstream monFlux(nomFichier.c_str());

	if (monFlux)
	{
		monFlux << "Manual =" << ui.manual << ";" << endl;
		monFlux << "Choix de l'algorithme =" << statistics.algorithm << ";" << endl;
		monFlux << "Position du panda =" << panda.position << ";" << endl;
		monFlux << "Panda en charge =" << panda.in_charge << ";" << endl;
		monFlux << "Etat de charge du panda =" << panda.life << ";" << endl;
		monFlux << "Nombre de bambou =" << garden.nb_bambou << ";" << endl;
		for (int i = 0; i < garden.nb_bambou; i++)
		{
			monFlux << "Coefficient du bambou n " << i + 1 << " =" << garden.bambou_slots[i].coefficient_growth << ";" << endl;
			monFlux << "Taille du bambou n " << i + 1 << " =" << garden.bambou_slots[i].size << ";" << endl;
		}
		monFlux << "Nombre de jours =" << NB_DAYS << ";" << endl;
		monFlux << "Coeffcient reduce fastest =" << statistics.x << ";" << endl;
	}
}

void initialisation(Ui& ui, Panda& panda, Garden& garden, Statistics& statistics) {
	ifstream entree("backup.txt", ios::in);
	char ligne[100];
	if (entree) {
		entree.getline(ligne, 100, '=');
		entree.getline(ligne, 10, ';');

		ui.manual = atoi(ligne);

		entree.getline(ligne, 100, '=');
		entree.getline(ligne, 10, ';');

		statistics.algorithm = atoi(ligne);

		entree.getline(ligne, 100, '=');
		entree.getline(ligne, 10, ';');

		panda.position = atoi(ligne);

		entree.getline(ligne, 100, '=');
		entree.getline(ligne, 10, ';');

		panda.in_charge = atoi(ligne);

		entree.getline(ligne, 100, '=');
		entree.getline(ligne, 10, ';');

		panda.life = atoi(ligne);

		entree.getline(ligne, 100, '=');
		entree.getline(ligne, 10, ';');

		garden.nb_bambou = atoi(ligne);

		for (int i = 0; i < garden.nb_bambou; i++) {

			entree.getline(ligne, 100, '=');
			entree.getline(ligne, 10, ';');

			garden.bambou_slots[i].coefficient_growth = atoi(ligne);

			entree.getline(ligne, 100, '=');
			entree.getline(ligne, 10, ';');

			garden.bambou_slots[i].size = atoi(ligne);
		}

		entree.getline(ligne, 100, '=');
		entree.getline(ligne, 10, ';');

		NB_DAYS = atoi(ligne);

		entree.getline(ligne, 100, '=');
		entree.getline(ligne, 10, ';');

		statistics.x = atof(ligne);

		entree.close();
	}
	else
	{
		init_garden(garden, 10);
	}
}

void update_statistics(Garden& garden, Statistics& stats)
{
	int max = garden.bambou_slots[0].size,
		moy = garden.bambou_slots[0].size,
		min = garden.bambou_slots[0].size;
	Bambou bambou;
	for (int i = 1; i < garden.nb_bambou; i++)
	{
		bambou = garden.bambou_slots[i];
		moy += bambou.size;
		if (bambou.size < min)
		{
			min = bambou.size;
		}
		if (bambou.size > max)
		{
			max = bambou.size;
		}
		if (bambou.size > stats.all_time_max)
		{
			stats.all_time_max = bambou.size;
		}
	}
	stats.max_size[(NB_DAYS - 1) % GRAPH_SIZE] = max;
	stats.moyenne[(NB_DAYS - 1) % GRAPH_SIZE] = moy / garden.nb_bambou;
	stats.min_size[(NB_DAYS - 1) % GRAPH_SIZE] = min;
}

int correct_size(int val, double coef, Statistics& stats) {
	return (int)((double)val * (coef / stats.window_height));
}

void display_graph(SDL_Renderer* rendu, Garden& garden, Statistics& stats, int tab[], int r, int g, int b) {
	SDL_SetRenderDrawColor(rendu, r, g, b, 255);
	int updated_size, x, y, old_x, old_y, valeur[GRAPH_SIZE];
	for (int i = 0; i < GRAPH_SIZE; i++)
	{
		valeur[i] = tab[(NB_DAYS + i) % GRAPH_SIZE];
	}
	for (int i = 0; i < GRAPH_SIZE; i++)
	{
		updated_size = correct_size(valeur[i], 170.0, stats);
		x = 762 + i * 36;
		y = 566 - updated_size;
		if (y < 396)
		{
			y = 396;
		}
		if (i > 0)
		{
			SDL_RenderDrawLine(rendu, old_x, old_y, x, y);
			SDL_RenderDrawLine(rendu, old_x, old_y - 1, x, y - 1);
		}
		old_x = x;
		old_y = y;
	}
}

void display_all_time_max_size(SDL_Renderer* rendu, Statistics& stats) {
	int updated_size = correct_size(stats.all_time_max, 170.0, stats);
	SDL_SetRenderDrawColor(rendu, 255, 0, 0, 255);
	int y = 566 - updated_size;
	if (y < 396)
	{
		y = 396;
	}
	SDL_RenderDrawLine(rendu, 762, y, 1018, y);
	y = 566 - updated_size - 1;
	if (y < 396)
	{
		y = 396;
	}
	SDL_RenderDrawLine(rendu, 762, y, 1018, y);
}