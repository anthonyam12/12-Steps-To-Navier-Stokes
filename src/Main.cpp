#include <math.h>
#include <memory>
#include <sstream>
#include <vector>
#include <SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "Core.h"
#include "Step.h"
#include "Step1.h"
#include "Step2.h"
#include "Step3.h"
#include "Step4.h"
#include "Step5.h"
#include "Step6.h"
#include "Step7.h"
#include "Step8.h"
#include "Step9.h"
#include "Step10.h"
#include "Step11.h"
#include "Step12.h"

const auto maxUpdatesPerFrame = 5;

std::unique_ptr<Step> createStep(int stepNumber)
{
    switch(stepNumber)
    {
    case 1:
        return std::unique_ptr<Step>(new Step1LinearConvection1D());
    case 2:
        return std::unique_ptr<Step>(new Step2NonlinearConvection1D());
    case 3:
        return std::unique_ptr<Step>(new Step3Diffusion1D());
    case 4:
        return std::unique_ptr<Step>(new Step4BurgersEquation1D());
    case 5:
        return std::unique_ptr<Step>(new Step5LinearConvection2D());
    case 6:
        return std::unique_ptr<Step>(new Step6NonlinearConvection2D());
    case 7:
        return std::unique_ptr<Step>(new Step7Diffusion2D());
    case 8:
        return std::unique_ptr<Step>(new Step8BurgersEquation2D());
    case 9:
        return std::unique_ptr<Step>(new Step9LaplaceEquation2D());
    case 10:
        return std::unique_ptr<Step>(new Step10PoissonEquation2D());
    case 11:
        return std::unique_ptr<Step>(new Step11CavityFlow());
    case 12:
        return std::unique_ptr<Step>(new Step12ChannelFlow());
    default:
        throw std::runtime_error("Invalid step number.");
    }
}
int wrapStepNumber(int stepNumber)
{
    return ((stepNumber - 1) % 12) + 1;
}

bool quit;
Uint64 lastPerfCount;
double accumulatedTime;
int stepNumber = 1;
std::unique_ptr<Step> step;
SDL_Window* window;
SDL_Renderer* renderer;

void changeStep(int newStepNumber)
{
    stepNumber = newStepNumber;
    step = createStep(stepNumber);
    SDL_SetWindowTitle(window, step->title.c_str());
};
void runFrame()
{
    // update timing
    const auto curPerfCount = SDL_GetPerformanceCounter();
    const double dt = (double)(curPerfCount - lastPerfCount) / SDL_GetPerformanceFrequency();
    accumulatedTime += dt;
    lastPerfCount = curPerfCount;

    // handle events
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_QUIT)
        {
            quit = true;
        }
        else if(event.type == SDL_KEYDOWN)
        {
            if(event.key.keysym.sym == SDLK_LEFT)
            {
                changeStep(wrap(stepNumber - 1, 1, 12));
            }
            else if(event.key.keysym.sym == SDLK_RIGHT)
            {
                changeStep(wrap(stepNumber + 1, 1, 12));
            }
        }
    }

    // update
    auto updatesThisFrame = 0;
    while((updatesThisFrame < maxUpdatesPerFrame) && (accumulatedTime >= step->fixedTimeStep))
    {
        step->update(step->fixedTimeStep);
        accumulatedTime -= step->fixedTimeStep;

        updatesThisFrame++;
    }

    // clear window
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    step->draw(renderer);

    // render window
    SDL_RenderPresent(renderer);
}
int main(int argc, char* argv[])
{
    quit = false;
    
    // init SDL
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("12 Steps to Navier-Stokes",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, 0);

    stepNumber = 1;

    changeStep(stepNumber);

    lastPerfCount = SDL_GetPerformanceCounter();
    accumulatedTime = 0;

#ifndef __EMSCRIPTEN__
    while(!quit)
    {
        runFrame();
    }
#else
    emscripten_set_main_loop(runFrame, 0, 1);
#endif

    // cleanup SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}