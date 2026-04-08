#include "sui/SUI.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>

auto main(int argc, char* argv[]) -> int {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        return -1;
    }
    if (!TTF_Init()) {
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Test CppProject", 800, 600, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    TTF_Font* mainFont = TTF_OpenFont("Assets/Fonts/NanoDyongSong.ttf", 24);
    if (mainFont == nullptr) {
        std::cerr << "Font not found!" << '\n';
    }

    UI gui(renderer);

    auto& rootBox = gui.AddElement<Box>(0xFFFFFFFF, [&](Box& b) -> void {
        b.setBgColor({30, 30, 35, 255}).setWeights({1.F, 1.F, 1.F}, {1.F, 1.F, 1.F});
    });

    auto& button = gui.AddElement<Box>(&rootBox, [&](Box& b) -> void {
        b.setGridPos(1, 1, 2, 2)
            .setBgColor({70, 130, 180, 255})
            .setRadius(15.F)
            .setMargin(20, 20, 40, 40)
            .setShadow({0, 0, 0, 100}, 10.F, 5.F, 5.F);

        b.hasMouseEvents = true;
        b.onHoverIn = [](int x, int y) -> void { std::cout << "Hover In!" << '\n'; };
        b.onHoverOut = [](int x, int y) -> void { std::cout << "Hover Out!" << '\n'; };
        b.onMouseDown = [](int x, int y) -> void { std::cout << "Buttom Down!" << '\n'; };
        b.onMouseUp = [](int x, int y) -> void { std::cout << "Button Up!" << '\n'; };
    });

    gui.AddElement<Text>(&button, [&](Text& t) -> void {
        t.setGridPos(0, 0, 1, 1)
            .setStyle({255, 255, 255, 255}, true, TTF_STYLE_UNDERLINE, mainFont, 24.F, true)
            .setText("TESTETSETSDSDVSD VA EFD!")
            .setAlignment(Alignment::MiddleMiddle);
    });

    gui.calculate();

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        float mx = 0.F;
        float my = 0.F;
        Uint32 mouseState = SDL_GetMouseState(&mx, &my);
        std::array<bool, 6> states = {
            (bool)(mouseState & SDL_BUTTON_LMASK),  (bool)(mouseState & SDL_BUTTON_MMASK),
            (bool)(mouseState & SDL_BUTTON_RMASK),  (bool)(mouseState & SDL_BUTTON_X1MASK),
            (bool)(mouseState & SDL_BUTTON_X2MASK), (bool)(mouseState & SDL_BUTTON_LMASK)};

        int w = 0;
        int h = 0;
        SDL_GetWindowSize(window, &w, &h);

        gui.renderInteractionTexture(w, h);
        gui.handleInteractions(static_cast<int>(mx), static_cast<int>(my), states);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        gui.render(w, h);

        SDL_RenderPresent(renderer);
    }

    if (mainFont != nullptr) {
        TTF_CloseFont(mainFont);
    }
    TTF_Quit();
    SDL_Quit();

    return 0;
}
