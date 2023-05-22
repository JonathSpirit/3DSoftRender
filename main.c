#include "SDL.h"
#include "gRender.h"
#include <stdio.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320
#define AVERAGE_TICKS_COUNT 40

void SetSurfacePixel(SDL_Surface* surface, int x, int y, SDL_Color color)
{
    if (surface == NULL)
    {
        return;
    }
    if (x<0 || y<0 || x>=surface->w || y>=surface->h)
    {
        return;
    }

    uint32_t* const target_pixel = (uint32_t*)(((uint8_t*)surface->pixels)
                                            + y * surface->pitch
                                            + x * surface->format->BytesPerPixel);
    *target_pixel = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a);
}

Triangle monCubeTriangles[12] = {
        {{0.0f,0.0f,0.0f}, {20.0f,0.0f,0.0f}, {0.0f,20.0f,0.0f}}, {{20.0f,20.0f,0.0f}, {20.0f,0.0f,0.0f}, {0.0f,20.0f,0.0f}},
        {{0.0f,0.0f,20.0f}, {20.0f,0.0f,20.0f}, {0.0f,20.0f,20.0f}}, {{20.0f,20.0f,20.0f}, {20.0f,0.0f,20.0f}, {0.0f,20.0f,20.0f}},

        {{0.0f,20.0f,0.0f}, {20.0f,20.0f,0.0f}, {0.0f,20.0f,20.0f}}, {{20.0f,20.0f,20.0f}, {20.0f,20.0f,0.0f}, {0.0f,20.0f,20.0f}},
        {{0.0f,0.0f,0.0f}, {20.0f,0.0f,0.0f}, {0.0f,0.0f,20.0f}}, {{20.0f,0.0f,20.0f}, {20.0f,0.0f,0.0f}, {0.0f,0.0f,20.0f}},

        {{0.0f,0.0f,0.0f}, {0.0f,0.0f,20.0f}, {0.0f,20.0f,0.0f}}, {{0.0f,20.0f,20.0f}, {0.0f,0.0f,20.0f}, {0.0f,20.0f,0.0f}},
        {{20.0f,0.0f,0.0f}, {20.0f,0.0f,20.0f}, {20.0f,20.0f,0.0f}}, {{20.0f,20.0f,20.0f}, {20.0f,0.0f,20.0f}, {20.0f,20.0f,0.0f}}
};

Object3D monCube = {(Triangle*)&monCubeTriangles, sizeof(monCubeTriangles) / sizeof(Triangle), {10.0f, 10.0f, 10.0f}};

uint8_t screen[SCREEN_WIDTH*SCREEN_HEIGHT*3];
Camera camera = {{0.0f,0.0f,500.0f}, {SCREEN_WIDTH,SCREEN_HEIGHT}, (uint8_t*)&screen, 51.52f};

Matrix3x3 rotationMatrixZ, rotationMatrixX, rotationMatrixY;

SDL_Surface* gTextureTest = NULL;

int main(int argc, char *argv[])
{
    /*Coord3 testCoord = {204.15f, -2362.2155f};
    float testResult;
    uint64_t b;

    uint32_t testFirst = SDL_GetTicks();
    for (b=0; b<4000000000; ++b)
    {
        testResult = GetInverseMagnitudeFromCoord(&testCoord);
    }
    uint32_t diffResult = SDL_GetTicks() - testFirst;
    printf("result ms : %u ms", diffResult);

    //21626 ms
    //22934 ms (no fast sqrt)

    return 0;*/

    Index i, a;
    Coord3 coord;

    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_Event event;

    SDL_Texture* textureScreen = SDL_CreateTexture(renderer,
                                                   SDL_PIXELFORMAT_RGB888,
                                                   SDL_TEXTUREACCESS_STREAMING,
                                                   SCREEN_WIDTH, SCREEN_HEIGHT);
    void* texturePixels = NULL;
    void* surfacePixels = NULL;
    int texturePitch = 0; //unused but required

    if (textureScreen == NULL)
    {
        printf("Unable to create texture ! SDL Error: %s", SDL_GetError());
        return -1;
    }

    SDL_Surface* rawTexture = SDL_LoadBMP("texture.bmp");
    if (rawTexture == NULL)
    {
        printf("Unable to load image %s! SDL Error: %s", "texture.bmp", SDL_GetError());
        return -1;
    }

    gTextureTest = SDL_ConvertSurfaceFormat(rawTexture, SDL_PIXELFORMAT_BGR888, 0);
    if (gTextureTest == NULL)
    {
        printf("Unable to convert surface ! SDL Error: %s", SDL_GetError());
        return -1;
    }
    SDL_FreeSurface(rawTexture);

    unsigned int quit = 0;

    CreateRotationMatrixOnAxeX(&rotationMatrixX, 0.087f);
    CreateRotationMatrixOnAxeY(&rotationMatrixY, 0.099f);
    CreateRotationMatrixOnAxeZ(&rotationMatrixZ, 0.175f);

    //Translating
    for (i=0; i<monCube._trianglesSize; ++i)
    {
        coord = monCube._triangles[i]._v0;
        coord._x *= 10.0f;
        coord._y *= 10.0f;
        coord._z *= 10.0f;

        monCube._triangles[i]._v0 = coord;

        coord = monCube._triangles[i]._v1;
        coord._x *= 10.0f;
        coord._y *= 10.0f;
        coord._z *= 10.0f;

        monCube._triangles[i]._v1 = coord;

        coord = monCube._triangles[i]._v2;
        coord._x *= 10.0f;
        coord._y *= 10.0f;
        coord._z *= 10.0f;

        monCube._triangles[i]._v2 = coord;
    }
    monCube._center._x *= 10.0f;
    monCube._center._y *= 10.0f;
    monCube._center._z *= 10.0f;

    Triangle testTriangle;
    testTriangle._v0 = (Coord3){0.0f, 0.0f, 0.0f};
    testTriangle._v1 = (Coord3){30.0f, 0.0f, 0.0f};
    testTriangle._v2 = (Coord3){0.0f, 30.0f, 0.0f};

    SetCameraFov(&camera);
    SetCameraScreen(&camera);

    CreateIdentityMatrix(&camera._rotationMatrix);

    uint32_t ticksBefore = SDL_GetTicks();
    uint32_t ticksTotals = 0;
    uint32_t ticksCount = 0;

    while (quit == 0)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = 1;
            }
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_q)
                {
                    RotateCamera(&camera, 0.0f,-Deg2rad(10.0f),0.0f);
                }
                if (event.key.keysym.sym == SDLK_e)
                {
                    RotateCamera(&camera, 0.0f,Deg2rad(10.0f),0.0f);
                }

                if (event.key.keysym.sym == SDLK_w)
                {
                    Coord3 forward = {0.0f, 0.0f, -1.0f};
                    MultiplyCoordWithMatrixInPlace(&camera._rotationMatrix, &forward);
                    Normalize(&forward);

                    forward._x *= 100.0f;
                    forward._y *= 100.0f;
                    forward._z *= 100.0f;

                    camera._origin._x = camera._origin._x + forward._x;
                    camera._origin._y = camera._origin._y + forward._y;
                    camera._origin._z = camera._origin._z + forward._z;
                }
                if (event.key.keysym.sym == SDLK_s)
                {
                    //move to the back
                    Coord3 backward = {0.0f, 0.0f, 1.0f};
                    MultiplyCoordWithMatrixInPlace(&camera._rotationMatrix, &backward);
                    Normalize(&backward);

                    backward._x *= 100.0f;
                    backward._y *= 100.0f;
                    backward._z *= 100.0f;

                    camera._origin._x = camera._origin._x + backward._x;
                    camera._origin._y = camera._origin._y + backward._y;
                    camera._origin._z = camera._origin._z + backward._z;
                }
                if (event.key.keysym.sym == SDLK_a)
                {
                    //move to the left
                    Coord3 left = {1.0f, 0.0f, 0.0f};
                    MultiplyCoordWithMatrixInPlace(&camera._rotationMatrix, &left);
                    Normalize(&left);

                    left._x *= 100.0f;
                    left._y *= 100.0f;
                    left._z *= 100.0f;

                    camera._origin._x = camera._origin._x + left._x;
                    camera._origin._y = camera._origin._y + left._y;
                    camera._origin._z = camera._origin._z + left._z;
                }
                if (event.key.keysym.sym == SDLK_d)
                {
                    //move to the right
                    Coord3 right = {-1.0f, 0.0f, 0.0f};
                    MultiplyCoordWithMatrixInPlace(&camera._rotationMatrix, &right);
                    Normalize(&right);

                    right._x *= 100.0f;
                    right._y *= 100.0f;
                    right._z *= 100.0f;

                    camera._origin._x = camera._origin._x + right._x;
                    camera._origin._y = camera._origin._y + right._y;
                    camera._origin._z = camera._origin._z + right._z;
                }
                if (event.key.keysym.sym == SDLK_SPACE)
                {
                    FILE *ptr = NULL;
                    fopen_s(&ptr, "test.csv","w");

                    if (ptr != NULL)
                    {
                        fprintf(ptr, "\"x\", \"y\", \"z\"\n");
                        for (i = 0; i < monCube._trianglesSize; ++i)
                        {
                            coord = monCube._triangles[i]._v0;
                            fprintf(ptr, "%.2f, %.2f, %.2f\n", coord._x, coord._y, coord._z);

                            coord = monCube._triangles[i]._v1;
                            fprintf(ptr, "%.2f, %.2f, %.2f\n", coord._x, coord._y, coord._z);

                            coord = monCube._triangles[i]._v2;
                            fprintf(ptr, "%.2f, %.2f, %.2f\n", coord._x, coord._y, coord._z);
                        }

                        fclose(ptr);
                    }
                }
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        ClearCamera(&camera);

        ApplyMatrixToObject(&rotationMatrixZ, &monCube);
        ApplyMatrixToObject(&rotationMatrixX, &monCube);
        ApplyMatrixToObject(&rotationMatrixY, &monCube);

        DrawObject3D(&monCube, &camera);
        DrawTriangles(&testTriangle, 1, &camera);

        SDL_LockTexture(textureScreen,
                        NULL,
                        &texturePixels, &texturePitch);
        surfacePixels = camera._screenData;
        for (i=0; i<SCREEN_HEIGHT; ++i)
        {
            for (a=0; a<SCREEN_WIDTH; ++a)
            {
                ((uint8_t*)texturePixels)[0] = ((uint8_t*)surfacePixels)[2];
                ((uint8_t*)texturePixels)[1] = ((uint8_t*)surfacePixels)[1];
                ((uint8_t*)texturePixels)[2] = ((uint8_t*)surfacePixels)[0];

                (uint8_t*)texturePixels += SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_RGB888);
                (uint8_t*)surfacePixels += 3;
            }
        }
        SDL_UnlockTexture(textureScreen);

        SDL_RenderCopy(renderer, textureScreen, NULL, NULL);
        SDL_RenderPresent(renderer);

        ticksTotals += SDL_GetTicks() - ticksBefore;

        if (++ticksCount >= AVERAGE_TICKS_COUNT)
        {
            printf("ticks average (%u): %u ms\n", AVERAGE_TICKS_COUNT, ticksTotals/ticksCount);
            ticksCount = 0;
            ticksTotals = 0;

            //123ms, average count = 40, (x64, Windows 11, AMD Ryzen 9 3900X 12-Core Processor, debug)
            //41ms, average count = 40, (x64, Windows 11, AMD Ryzen 9 3900X 12-Core Processor, release)
        }

        ticksBefore = SDL_GetTicks();
    }

    SDL_FreeSurface(gTextureTest);
    SDL_DestroyTexture(textureScreen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}