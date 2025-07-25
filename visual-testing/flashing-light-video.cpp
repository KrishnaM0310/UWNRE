#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>
#include <SDL2/SDL.h>

// Draw checkerboard background
void drawCheckerboard(SDL_Renderer* renderer, int rows, int cols, int windowWidth, int windowHeight, bool invertColors) {
    int blockWidth = windowWidth / cols;
    int blockHeight = windowHeight / rows;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            bool isBlue = (i + j) % 2 == 0;
            if (invertColors) isBlue = !isBlue;

            SDL_SetRenderDrawColor(renderer,
                isBlue ? 150 : 255,
                isBlue ? 150 : 255,
                isBlue ? 150 : 255,
                255);

            SDL_Rect block = { j * blockWidth, i * blockHeight, blockWidth, blockHeight };
            SDL_RenderFillRect(renderer, &block);
        }
    }
}

// Draw circle dot
void drawDot(SDL_Renderer* renderer, int x, int y, int radius, SDL_Color color, bool visible) {
    if (!visible) return;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for (int w = 0; w < radius * 2; ++w) {
        for (int h = 0; h < radius * 2; ++h) {
            int dx = radius - w;
            int dy = radius - h;
            if (dx * dx + dy * dy <= radius * radius) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

// Core run mode function with frequency measurement
void runMode(SDL_Renderer* renderer, int windowWidth, int windowHeight,
             bool flashCheckerboard, bool flashRedDot, int durationSeconds = 15, double frequencyHz = 15.0) {

    const int rows = 30, cols = 45;
    const double frameDuration = 1.0 / frequencyHz;
    auto startTime = std::chrono::steady_clock::now();
    auto lastFrameTime = startTime;

    bool checkerboardState = false;
    bool redDotVisible = false;

    // Variables to track checkerboard toggle timing
    int checkerboardToggleCount = 0;
    double checkerboardTotalToggleDuration = 0.0;
    auto checkerboardLastToggleTime = startTime;
    auto checkerboardFreqPrintTime = startTime;

    // Variables to track red dot toggle timing
    int redDotToggleCount = 0;
    double redDotTotalToggleDuration = 0.0;
    auto redDotLastToggleTime = startTime;
    auto redDotFreqPrintTime = startTime;

    SDL_Event event;
    bool running = true;

    while (running) {
        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) return;
        }

        // Wait until the next frame
        auto now = std::chrono::steady_clock::now();
        while (std::chrono::duration<double>(now - lastFrameTime).count() < frameDuration) {
            now = std::chrono::steady_clock::now();
        }
        lastFrameTime = now;

        // Toggle checkerboard state and measure frequency
        if (flashCheckerboard) {
            checkerboardState = !checkerboardState;

            auto nowToggle = std::chrono::steady_clock::now();
            double toggleDuration = std::chrono::duration<double>(nowToggle - checkerboardLastToggleTime).count();

            checkerboardTotalToggleDuration += toggleDuration;
            checkerboardToggleCount++;
            checkerboardLastToggleTime = nowToggle;

            double timeSinceLastPrint = std::chrono::duration<double>(nowToggle - checkerboardFreqPrintTime).count();
            if (timeSinceLastPrint >= 1.0 && checkerboardToggleCount > 0) {
                double averagePeriod = checkerboardTotalToggleDuration / checkerboardToggleCount;
                double freq = 1.0 / averagePeriod;
                std::cout << std::fixed << std::setprecision(3)
                          << "Checkerboard Frequency: " << freq << " Hz" << std::endl;

                checkerboardToggleCount = 0;
                checkerboardTotalToggleDuration = 0.0;
                checkerboardFreqPrintTime = nowToggle;
            }
        }

        // Toggle red dot visibility and measure frequency
        if (flashRedDot) {
            redDotVisible = !redDotVisible;

            auto nowToggle = std::chrono::steady_clock::now();
            double toggleDuration = std::chrono::duration<double>(nowToggle - redDotLastToggleTime).count();

            redDotTotalToggleDuration += toggleDuration;
            redDotToggleCount++;
            redDotLastToggleTime = nowToggle;

            double timeSinceLastPrint = std::chrono::duration<double>(nowToggle - redDotFreqPrintTime).count();
            if (timeSinceLastPrint >= 1.0 && redDotToggleCount > 0) {
                double averagePeriod = redDotTotalToggleDuration / redDotToggleCount;
                double freq = 1.0 / averagePeriod;
                std::cout << std::fixed << std::setprecision(3)
                        << "Red Dot Frequency: " << freq << " Hz" << std::endl;

                redDotToggleCount = 0;
                redDotTotalToggleDuration = 0.0;
                redDotFreqPrintTime = nowToggle;
            }
        } else {
            redDotVisible = true; // <- this ensures it's shown when not flashing
        }


        // Clear screen and draw
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        drawCheckerboard(renderer, rows, cols, windowWidth, windowHeight, checkerboardState);

        SDL_Color black = {0, 0, 0, 255};
        drawDot(renderer, windowWidth / 2, windowHeight / 2, 10, black, true);

        SDL_Color red = {255, 0, 0, 255};
        drawDot(renderer, windowWidth / 2 + 300, windowHeight / 2 - 200, 30, red, redDotVisible);

        SDL_RenderPresent(renderer);

        // Check total duration
        auto elapsed = std::chrono::duration<double>(now - startTime).count();
        if (elapsed >= durationSeconds) break;
    }
}

// black window
void showBlackScreen(SDL_Renderer* renderer, int windowWidth, int windowHeight, int durationSeconds) {
    SDL_Event event;
    auto startTime = std::chrono::steady_clock::now();

    while (true) {
        // Allow quitting during the black screen
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) return;
        }

        // Check if duration has passed
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(now - startTime).count();
        if (elapsed >= durationSeconds) break;

        // Fill screen with black
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        // Brief delay to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// Each mode: call with appropriate flags
void flashingCheckerboardStaticDot(SDL_Renderer* renderer, int windowWidth, int windowHeight) {
    runMode(renderer, windowWidth, windowHeight, true, false);
}

void staticCheckerboardFlashingDot(SDL_Renderer* renderer, int windowWidth, int windowHeight) {
    runMode(renderer, windowWidth, windowHeight, false, true);
}

void flashingBoth(SDL_Renderer* renderer, int windowWidth, int windowHeight) {
    runMode(renderer, windowWidth, windowHeight, true, true);
}

void flashingWhiteDot(SDL_Renderer* renderer, int windowWidth, int windowHeight,
                      int durationSeconds, double frequencyHz = 12.0) {
    const double frameDuration = 1.0 / frequencyHz;
    auto startTime = std::chrono::steady_clock::now();
    auto lastFrameTime = startTime;

    bool dotVisible = false;

    SDL_Event event;
    bool running = true;

    while (running) {
        // Handle quit and ESC events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT ||
               (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                return;
            }
        }

        // Wait for next frame
        auto now = std::chrono::steady_clock::now();
        while (std::chrono::duration<double>(now - lastFrameTime).count() < frameDuration) {
            now = std::chrono::steady_clock::now();
        }
        lastFrameTime = now;

        // Toggle dot visibility
        dotVisible = !dotVisible;

        // Clear screen to black
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Static white center dot
        SDL_Color white = {255, 255, 255, 255};
        drawDot(renderer, windowWidth / 2, windowHeight / 2, 5, white, true); // <- small static dot

        // Flashing white dot off-center
        drawDot(renderer, windowWidth / 2 + 300, windowHeight / 2 - 200, 30, white, dotVisible);

        SDL_RenderPresent(renderer);

        // Exit after duration
        double elapsed = std::chrono::duration<double>(now - startTime).count();
        if (elapsed >= durationSeconds) break;
    }
}


int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL Init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    int windowWidth = DM.w;
    int windowHeight = DM.h;

    // const int windowWidth = 800, windowHeight = 600;
    SDL_Window* window = SDL_CreateWindow("Flashing Modes", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
    SDL_SetWindowInputFocus(window);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Call each visual mode in sequence
    showBlackScreen(renderer, windowWidth, windowHeight, 15);
    staticCheckerboardFlashingDot(renderer, windowWidth, windowHeight);
    showBlackScreen(renderer, windowWidth, windowHeight, 10);
    flashingCheckerboardStaticDot(renderer, windowWidth, windowHeight);
    showBlackScreen(renderer, windowWidth, windowHeight, 10);
    flashingBoth(renderer, windowWidth, windowHeight);
    showBlackScreen(renderer, windowWidth, windowHeight, 10);
    flashingWhiteDot(renderer, windowWidth, windowHeight, 15);
    showBlackScreen(renderer, windowWidth, windowHeight, 10);
    flashingWhiteDot(renderer, windowWidth, windowHeight, 15);
    showBlackScreen(renderer, windowWidth, windowHeight, 10);
    flashingWhiteDot(renderer, windowWidth, windowHeight, 15);
    showBlackScreen(renderer, windowWidth, windowHeight, 10);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
