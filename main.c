#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>

#define WIDTH 60
#define HEIGHT 60
#define SIZE 10
#define DELAY 0.4

SDL_Window* gWindow;
SDL_Renderer* gRenderer;

void init_fields();
void quit_fields();

void init() {
    SDL_Init(SDL_INIT_VIDEO);
    gWindow = SDL_CreateWindow("WireWorld", SDL_WINDOWPOS_CENTERED, 
            SDL_WINDOWPOS_CENTERED, WIDTH * SIZE, HEIGHT * SIZE, SDL_WINDOW_SHOWN);
    gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
    init_fields();
}

void quit() {
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
    quit_fields();
}

enum eCellType {
    CONDUCTOR, HEAD, TAIL, EMPTY
};

enum eCellType** field;
enum eCellType** newField;

void init_fields() {
    field = (enum eCellType**)malloc(HEIGHT * sizeof(enum eCellType*));
    newField = (enum eCellType**)malloc(HEIGHT * sizeof(enum eCellType*));
    for (int i = 0; i < HEIGHT; i++) {
        field[i] = (enum eCellType*)malloc(WIDTH * sizeof(enum eCellType));
        newField[i] = (enum eCellType*)malloc(WIDTH * sizeof(enum eCellType));
        for (int j = 0; j < WIDTH; j++) 
            field[i][j] = newField[i][j] = EMPTY;
            
    }
}

void quit_fields() {
    for (int i = 0; i < HEIGHT; i++) {
        free(field[i]);
        free(newField[i]);
    }
    free(field);
    free(newField);
}

void present() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            SDL_Rect rect = {j * SIZE, i * SIZE, SIZE, SIZE};
            switch(field[i][j]) {
                case EMPTY: 
                    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xFF);
                    break;
                case HEAD: 
                    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0xFF, 0xFF);
                    break;
                case TAIL:
                    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0, 0, 0xFF);
                    break;
                case CONDUCTOR:
                    SDL_SetRenderDrawColor(gRenderer, 0, 0xFF, 0xFF, 0xFF);
                    break;
            }
            SDL_RenderFillRect(gRenderer, &rect);
        }
    }
    SDL_RenderPresent(gRenderer);
}

bool edit() {
    SDL_Event e;
    bool shiftPressed = false;
    while (true) {
        if (SDL_PollEvent(&e)) {
            switch(e.type) {
                case SDL_QUIT:
                    return false;
                case SDL_KEYDOWN:
                    switch(e.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            return false;
                        case SDLK_LSHIFT:
                        case SDLK_RSHIFT:
                            shiftPressed = true;
                            break;
                        case SDLK_SPACE:
                            return true;
                    }
                    break;
                case SDL_KEYUP: 
                    if (e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT)
                        shiftPressed = false;
                    break;
                case SDL_MOUSEBUTTONUP: {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    int fieldX = x / SIZE, fieldY = y / SIZE;
                    field[fieldY][fieldX] = shiftPressed ? EMPTY : (enum eCellType)((field[fieldY][fieldX] + 1) % 4);
                    present();
                }
                break;
            }
        }
    }
}

bool is_head(const int y, const int x) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return false;
    return field[y][x] == HEAD;
}

bool heads_around(const int y, const int x) {
    int nHeads = 0;
    for (int i = -1; i <= 1; i++)
        for (int j = -1; j <= 1; j++)
            nHeads += field[y + i][x + j] == HEAD;
    return nHeads == 1 || nHeads == 2;
}

enum eResponse {
    QUIT, EDIT, CONTINUE
};

enum eResponse update() {
    SDL_Delay(DELAY);
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            return QUIT;
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE)
                return QUIT;
            else if (e.key.keysym.sym == SDLK_SPACE)
                return EDIT;
        }
    }
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            switch(field[i][j]) {
                case EMPTY:
                    newField[i][j] = EMPTY;
                    break;
                case HEAD:
                    newField[i][j] = TAIL;
                    break;
                case TAIL:
                    newField[i][j] = CONDUCTOR;
                    break;
                case CONDUCTOR:
                    newField[i][j] = heads_around(i, j) ? HEAD : CONDUCTOR;
                    break;
            }
        }
    }
    for (int i = 0; i < HEIGHT; i++)
        for (int j = 0; j < WIDTH; j++)
            field[i][j] = newField[i][j];
    present();
    return CONTINUE;
}

int main() {
    init();
    bool editing = true;
    bool finish = false;
    while (!finish) {
        if (editing) {
            if (edit())
                editing = false;
            else
                finish = true;
        } else {
            switch(update()) {
                case QUIT:
                    finish = true;
                    break;
                case EDIT:
                    editing = true;
                    break;
                case CONTINUE:
                    break;
            }
        }
    }
    quit();
    return 0;
}
