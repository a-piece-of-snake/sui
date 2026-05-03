#include "sui/SUI.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

auto main(int argc, char* argv[]) -> int {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("CppProject", 1000, 700, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    TTF_Font* mainFont = TTF_OpenFont("Assets/Fonts/NanoDyongSong.ttf", 24);

    UI gui(renderer);

    auto& rootBox = gui.AddElement<Box>(0xFFFFFFFF, [&](Box& b) -> void {
        b.setBgColor({20, 20, 25, 255})
            .setDims({{SizeUnit::Weight, 1.F}},
                     {{SizeUnit::Pixel, 250.F}, {SizeUnit::Weight, 1.F}});
    });

    auto& sidebar = gui.AddElement<Box>(&rootBox, [&](Box& b) -> void {
        b.setGridPos(0, 0, 1, 1).setBgColor({35, 35, 40, 255}).setPadding(20, 20, 20, 20);
    });

    auto& rightBox = gui.AddElement<Box>(&rootBox, [&](Box& b) -> void {
        b.setGridPos(0, 1, 1, 2)
            .setBgColor({30, 30, 35, 255})
            .setDims({{SizeUnit::Weight, 1.F}, {SizeUnit::Weight, 1.F}}, {{SizeUnit::Weight, 1.F}});
    });

    auto& topSection = gui.AddElement<Box>(&rightBox, [&](Box& b) -> void {
        b.setGridPos(0, 0, 1, 1)
            .setDims({{SizeUnit::Weight, 1.F}},
                     {{SizeUnit::Weight, 1.F}, {SizeUnit::Weight, 1.F}, {SizeUnit::Weight, 1.F}})
            .setGaps(10.F, 10.F);
    });

    for (int i = 0; i < 3; ++i) {
        gui.AddElement<Box>(&topSection, [i](Box& b) -> void {
            b.setGridPos(0, i, 1, i + 1)
                .setBgColor({50, 50, 60, 255})
                .setMargin(10, 10, 10, 10)
                .setRadius(10.F);
        });
    }
    auto& bottomSection = gui.AddElement<Box>(&rightBox, [&](Box& b) -> void {
        b.setGridPos(1, 0, 2, 1).setDims({{SizeUnit::Weight, 1.F}}, {{SizeUnit::Weight, 1.F}});
    });
    auto& button = gui.AddElement<Box>(&bottomSection, [&](Box& b) -> void {
        b.setGridPos(0, 0, 1, 1)
            .setBgColor({70, 130, 180, 255})
            .setRadius(12.F)
            .setPadding(10, 10, 10, 10)
            .setMargin(100, 100, 200, 200)
            .setShadow({0, 0, 0, 150}, 15.F, 8.F, 8.F)
            .setBorder({255, 255, 255, 200}, 2.F);

        b.onHoverIn = [](int x, int y) -> void { std::cout << "Hover In!" << '\n'; };
        b.onHoverOut = [](int x, int y) -> void { std::cout << "Hover Out!" << '\n'; };
        b.onMouseDown = [](int x, int y) -> void { std::cout << "Buttom Down!" << '\n'; };
        b.onMouseUp = [](int x, int y) -> void { std::cout << "Button Up!" << '\n'; };
        b.hasMouseEvents = true;
    });

    auto& buttonText = gui.AddElement<Text>(&button, [&](Text& t) -> void {
        t.setGridPos(0, 0, 1, 1)
            .setStyle({255, 255, 255, 255}, true, 0, mainFont, 28.F, true)
            .setText("TEST")
            .setAlignment(Alignment::MiddleMiddle);
    });

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

        gui.calculate();
        gui.renderInteractionTexture(w, h);
        gui.handleInteractions(static_cast<int>(mx), static_cast<int>(my), states);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        gui.render(w, h);
        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(mainFont);
    SDL_Quit();
    return 0;
}
