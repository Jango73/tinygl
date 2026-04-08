#include "../../common/tinygl_demo.h"

#include <SDL2/SDL.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TINYGL_DEMO_DEFAULT_WIDTH 960
#define TINYGL_DEMO_DEFAULT_HEIGHT 720
#define TINYGL_DEMO_DEFAULT_FPS 60

/************************************************************************/

typedef struct TINYGL_DEMO_SDL2_APP {
    SDL_Window *Window;
    SDL_Renderer *Renderer;
    SDL_Texture *Texture;
    TINYGL_DEMO_APP Demo;
    int Width;
    int Height;
    int MaxFrames;
    int TargetFramesPerSecond;
    int FrameCount;
    int Paused;
    int StepRequested;
    Uint64 LastCounter;
} TINYGL_DEMO_SDL2_APP;

/************************************************************************/

/**
 * @brief Print an SDL2 demo error.
 * @param Message The error message.
 * @return Always returns 0.
 */
static int printSdl2Error(const char *Message) {
    fprintf(stderr, "tinygl_demo_sdl2: %s\n", Message);
    return 0;
}

/************************************************************************/

/**
 * @brief Parse a positive integer command-line value.
 * @param Text The text to parse.
 * @param OutValue The parsed value destination.
 * @return 1 on success, 0 on failure.
 */
static int parsePositiveInteger(const char *Text, int *OutValue) {
    char *EndPtr;
    long Value;

    errno = 0;
    Value = strtol(Text, &EndPtr, 10);
    if (errno != 0 || EndPtr == Text || *EndPtr != '\0' || Value <= 0 ||
        Value > 100000) {
        return 0;
    }

    *OutValue = (int)Value;
    return 1;
}

/************************************************************************/

/**
 * @brief Print the command-line usage.
 * @param ProgramName The executable name.
 */
static void printUsage(const char *ProgramName) {
    fprintf(stderr,
            "usage: %s [--width N] [--height N] [--frames N] [--fps N] "
            "[--paused]\n"
            "  --width N   Set the initial window width\n"
            "  --height N  Set the initial window height\n"
            "  --frames N  Exit after N rendered frames\n"
            "  --fps N     Limit the demo to N frames per second\n"
            "  --paused    Start paused on the first frame\n"
            "\n"
            "keyboard controls:\n"
            "  1           Show the default unlit rotating cube\n"
            "  2           Show the rotating cube with vertex lighting\n"
            "  Space       Pause or resume the animation\n"
            "  N           Render one frame while paused\n",
            ProgramName);
}

/************************************************************************/

/**
 * @brief Print the interactive view mode menu.
 */
static void printViewModeMenu(void) {
    fprintf(stderr,
            "tinygl_demo_sdl2 controls:\n"
            "  1 - Default rotating cube\n"
            "  2 - Rotating cube with lighting\n"
            "  Space - Pause or resume the animation\n"
            "  N - Render one frame while paused\n");
}

/************************************************************************/

/**
 * @brief Select a demo view mode from a keyboard key.
 * @param App The SDL2 demo state.
 * @param KeySymbol The SDL key symbol.
 */
static void selectViewMode(TINYGL_DEMO_SDL2_APP *App, SDL_Keycode KeySymbol) {
    TINYGL_DEMO_VIEW_MODE ViewMode;

    ViewMode = tinyglDemoGetViewMode(&App->Demo);

    switch (KeySymbol) {
    case SDLK_1: {
        ViewMode = TINYGL_DEMO_VIEW_MODE_UNLIT_CUBE;
    } break;

    case SDLK_2: {
        ViewMode = TINYGL_DEMO_VIEW_MODE_LIT_CUBE;
    } break;

    case SDLK_SPACE: {
        App->Paused = !App->Paused;
        if (App->Paused) {
            App->LastCounter = 0;
        }
        return;
    } break;

    case SDLK_n: {
        if (App->Paused) {
            App->StepRequested = 1;
            App->LastCounter = 0;
        }
        return;
    } break;

    default:
        return;
    }

    tinyglDemoSetViewMode(&App->Demo, ViewMode);
}

/************************************************************************/

/**
 * @brief Parse SDL2 demo command-line arguments.
 * @param Argc The argument count.
 * @param Argv The argument vector.
 * @param App The SDL2 demo state.
 * @return 1 on success, 0 on failure.
 */
static int parseArguments(int Argc, char **Argv, TINYGL_DEMO_SDL2_APP *App) {
    int Index;

    App->Width = TINYGL_DEMO_DEFAULT_WIDTH;
    App->Height = TINYGL_DEMO_DEFAULT_HEIGHT;
    App->MaxFrames = 0;
    App->TargetFramesPerSecond = TINYGL_DEMO_DEFAULT_FPS;
    App->Paused = 0;

    for (Index = 1; Index < Argc; Index++) {
        if (strcmp(Argv[Index], "--width") == 0) {
            if (Index + 1 >= Argc ||
                !parsePositiveInteger(Argv[Index + 1], &App->Width)) {
                printUsage(Argv[0]);
                return 0;
            }
            Index++;
            continue;
        }

        if (strcmp(Argv[Index], "--height") == 0) {
            if (Index + 1 >= Argc ||
                !parsePositiveInteger(Argv[Index + 1], &App->Height)) {
                printUsage(Argv[0]);
                return 0;
            }
            Index++;
            continue;
        }

        if (strcmp(Argv[Index], "--frames") == 0) {
            if (Index + 1 >= Argc ||
                !parsePositiveInteger(Argv[Index + 1], &App->MaxFrames)) {
                printUsage(Argv[0]);
                return 0;
            }
            Index++;
            continue;
        }

        if (strcmp(Argv[Index], "--fps") == 0) {
            if (Index + 1 >= Argc || !parsePositiveInteger(Argv[Index + 1],
                                                           &App->TargetFramesPerSecond)) {
                printUsage(Argv[0]);
                return 0;
            }
            Index++;
            continue;
        }

        if (strcmp(Argv[Index], "--paused") == 0) {
            App->Paused = 1;
            continue;
        }

        printUsage(Argv[0]);
        return 0;
    }

    return 1;
}

/************************************************************************/

/**
 * @brief Recreate the SDL texture after a surface size change.
 * @param App The SDL2 demo state.
 * @return 1 on success, 0 on failure.
 */
static int recreateTexture(TINYGL_DEMO_SDL2_APP *App) {
    if (App->Texture != NULL) {
        SDL_DestroyTexture(App->Texture);
        App->Texture = NULL;
    }

    App->Texture = SDL_CreateTexture(App->Renderer, SDL_PIXELFORMAT_XRGB8888,
                                     SDL_TEXTUREACCESS_STREAMING, App->Width,
                                     App->Height);
    if (App->Texture == NULL) {
        fprintf(stderr, "tinygl_demo_sdl2: unable to create texture: %s\n",
                SDL_GetError());
        return 0;
    }

    return 1;
}

/************************************************************************/

/**
 * @brief Initialize the SDL2 host backend.
 * @param App The SDL2 demo state.
 * @return 1 on success, 0 on failure.
 */
static int initializeSdl2(TINYGL_DEMO_SDL2_APP *App) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "tinygl_demo_sdl2: unable to initialize SDL2: %s\n",
                SDL_GetError());
        return 0;
    }

    App->Window = SDL_CreateWindow("TinyGL SDL2 Demo", SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, App->Width,
                                   App->Height, SDL_WINDOW_RESIZABLE);
    if (App->Window == NULL) {
        fprintf(stderr, "tinygl_demo_sdl2: unable to create window: %s\n",
                SDL_GetError());
        return 0;
    }

    App->Renderer = SDL_CreateRenderer(App->Window, -1, SDL_RENDERER_SOFTWARE);
    if (App->Renderer == NULL) {
        fprintf(stderr, "tinygl_demo_sdl2: unable to create renderer: %s\n",
                SDL_GetError());
        return 0;
    }

    return recreateTexture(App);
}

/************************************************************************/

/**
 * @brief Release the SDL2 host backend resources.
 * @param App The SDL2 demo state.
 */
static void shutdownSdl2(TINYGL_DEMO_SDL2_APP *App) {
    if (App->Texture != NULL) {
        SDL_DestroyTexture(App->Texture);
        App->Texture = NULL;
    }

    if (App->Renderer != NULL) {
        SDL_DestroyRenderer(App->Renderer);
        App->Renderer = NULL;
    }

    if (App->Window != NULL) {
        SDL_DestroyWindow(App->Window);
        App->Window = NULL;
    }

    SDL_Quit();
}

/************************************************************************/

/**
 * @brief Resize the demo surface and SDL texture together.
 * @param App The SDL2 demo state.
 * @param Width The new width.
 * @param Height The new height.
 * @return 1 on success, 0 on failure.
 */
static int resizeDemo(TINYGL_DEMO_SDL2_APP *App, int Width, int Height) {
    if (Width <= 0 || Height <= 0) {
        return 1;
    }

    if (!tinyglDemoResize(&App->Demo, Width, Height)) {
        return 0;
    }

    App->Width = Width;
    App->Height = Height;

    return recreateTexture(App);
}

/************************************************************************/

/**
 * @brief Process all pending SDL events.
 * @param App The SDL2 demo state.
 * @param Running The running flag.
 * @return 1 on success, 0 on failure.
 */
static int handleEvents(TINYGL_DEMO_SDL2_APP *App, int *Running) {
    SDL_Event Event;

    while (SDL_PollEvent(&Event)) {
        if (Event.type == SDL_QUIT) {
            *Running = 0;
            continue;
        }

        if (Event.type == SDL_WINDOWEVENT &&
            Event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            if (!resizeDemo(App, Event.window.data1, Event.window.data2)) {
                return 0;
            }
        }

        if (Event.type == SDL_KEYDOWN && Event.key.repeat == 0) {
            selectViewMode(App, Event.key.keysym.sym);
        }
    }

    return 1;
}

/************************************************************************/

/**
 * @brief Render one SDL2-presented frame.
 * @param App The SDL2 demo state.
 * @return 1 on success, 0 on failure.
 */
static int renderFrame(TINYGL_DEMO_SDL2_APP *App) {
    Uint64 CurrentCounter;
    Uint64 FrameEndCounter;
    Uint64 Frequency;
    Uint64 TargetTicks;
    Uint64 ElapsedTicks;
    GLfloat DeltaSeconds;

    CurrentCounter = SDL_GetPerformanceCounter();
    Frequency = SDL_GetPerformanceFrequency();
    if (App->LastCounter == 0) {
        DeltaSeconds = 1.0f / 60.0f;
    } else {
        DeltaSeconds = (GLfloat)((double)(CurrentCounter - App->LastCounter) /
                                 (double)Frequency);
    }
    App->LastCounter = CurrentCounter;

    if (!tinyglDemoRenderFrame(&App->Demo, DeltaSeconds)) {
        return 0;
    }

    if (SDL_UpdateTexture(App->Texture, NULL, tinyglDemoGetPixels(&App->Demo),
                          tinyglDemoGetPitch(&App->Demo)) != 0) {
        fprintf(stderr, "tinygl_demo_sdl2: unable to update texture: %s\n",
                SDL_GetError());
        return 0;
    }

    if (SDL_RenderClear(App->Renderer) != 0) {
        fprintf(stderr, "tinygl_demo_sdl2: unable to clear renderer: %s\n",
                SDL_GetError());
        return 0;
    }

    if (SDL_RenderCopyEx(App->Renderer, App->Texture, NULL, NULL, 0.0, NULL,
                         SDL_FLIP_VERTICAL) != 0) {
        fprintf(stderr, "tinygl_demo_sdl2: unable to copy texture: %s\n",
                SDL_GetError());
        return 0;
    }

    SDL_RenderPresent(App->Renderer);
    App->FrameCount++;
    App->StepRequested = 0;

    if (App->TargetFramesPerSecond > 0) {
        FrameEndCounter = SDL_GetPerformanceCounter();
        TargetTicks = Frequency / (Uint64)App->TargetFramesPerSecond;
        ElapsedTicks = FrameEndCounter - CurrentCounter;

        if (ElapsedTicks < TargetTicks) {
            Uint64 RemainingTicks;
            Uint32 DelayMilliseconds;

            RemainingTicks = TargetTicks - ElapsedTicks;
            DelayMilliseconds =
                (Uint32)((RemainingTicks * 1000) / Frequency);

            if (DelayMilliseconds > 0) {
                SDL_Delay(DelayMilliseconds);
            }
        }
    }

    return 1;
}

/************************************************************************/

/**
 * @brief Run the SDL2 demo main loop.
 * @param App The SDL2 demo state.
 * @return 0 on success, 1 on failure.
 */
static int runDemo(TINYGL_DEMO_SDL2_APP *App) {
    int Running;

    Running = 1;
    while (Running) {
        if (!handleEvents(App, &Running)) {
            return 1;
        }

        if (!Running) {
            break;
        }

        if (App->Paused && !App->StepRequested) {
            SDL_Delay(10);
            continue;
        }

        if (!renderFrame(App)) {
            return 1;
        }

        if (App->MaxFrames > 0 && App->FrameCount >= App->MaxFrames) {
            break;
        }
    }

    return 0;
}

/************************************************************************/

/**
 * @brief Entry point for the SDL2 TinyGL demo.
 * @param Argc The argument count.
 * @param Argv The argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int Argc, char **Argv) {
    TINYGL_DEMO_SDL2_APP App;
    int ExitCode;

    memset(&App, 0, sizeof(App));

    if (!parseArguments(Argc, Argv, &App)) {
        return 1;
    }

    if (!initializeSdl2(&App)) {
        shutdownSdl2(&App);
        return 1;
    }

    if (!tinyglDemoInitialize(&App.Demo, App.Width, App.Height)) {
        tinyglDemoShutdown(&App.Demo);
        shutdownSdl2(&App);
        return 1;
    }

    printViewModeMenu();

    ExitCode = runDemo(&App);

    tinyglDemoShutdown(&App.Demo);
    shutdownSdl2(&App);

    if (ExitCode != 0) {
        return printSdl2Error("demo terminated with an error");
    }

    return 0;
}
