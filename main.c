#include "SDL.h"
#include "gRender.h"
#include "time.h"

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

void ToSurfacePixels(SDL_Surface* surface, Camera* camera)
{
    if (surface->w == camera->_screenSize._w &&
        surface->h == camera->_screenSize._h)
    {
        uint16_t x, y;
        uint8_t* pixelData = camera->_screenData;

        for (y=0; y<camera->_screenSize._h; ++y)
        {
            for (x=0; x<camera->_screenSize._w; ++x)
            {
                SetSurfacePixel(surface, x, y, (SDL_Color){*pixelData, *(pixelData+1), *(pixelData+2), 255});
                pixelData += 3;
            }
        }
    }
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

uint8_t screen[640*320*3];
Camera camera = {{0.0f,0.0f,0.0f}, {640,320}, (uint8_t*)&screen, 51.52f};

Matrix3x3 rotationMatrixZ, rotationMatrixX;

int main(int argc, char *argv[])
{
    Index i;
    Coord3 coord;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Event event;

    SDL_Surface* surfaceScreen = SDL_CreateRGBSurfaceWithFormat(0, 640, 320, 32, SDL_PIXELFORMAT_RGB888);
    if (surfaceScreen == NULL)
    {
        return -1;
    }

    unsigned int quit = 0;

    CreateRotationMatrixOnAxeZ(&rotationMatrixZ, 0.175f);
    CreateRotationMatrixOnAxeX(&rotationMatrixX, 0.087f);

    //Translating
    for (i=0; i<monCube._trianglesSize; ++i)
    {
        coord = monCube._triangles[i]._v0;
        coord._x *= 10.0f;
        coord._y *= 10.0f;
        coord._z *= 10.0f;

        /*coord._x += 400.0f;
        coord._y += 100.0f;
        coord._z += 0.0f;*/

        monCube._triangles[i]._v0 = coord;

        coord = monCube._triangles[i]._v1;
        coord._x *= 10.0f;
        coord._y *= 10.0f;
        coord._z *= 10.0f;

        /*coord._x += 400.0f;
        coord._y += 100.0f;
        coord._z += 0.0f;*/

        monCube._triangles[i]._v1 = coord;

        coord = monCube._triangles[i]._v2;
        coord._x *= 10.0f;
        coord._y *= 10.0f;
        coord._z *= 10.0f;

        /*coord._x += 400.0f;
        coord._y += 100.0f;
        coord._z += 0.0f;*/

        monCube._triangles[i]._v2 = coord;
    }
    monCube._center._x *= 10.0f;
    monCube._center._y *= 10.0f;
    monCube._center._z *= 10.0f;

    /*monCube._center._x += 400.0f;
    monCube._center._y += 100.0f;
    monCube._center._z += 0.0f;*/

    Triangle testTriangle;
    testTriangle._v0 = (Coord3){0.0f, 0.0f, 0.0f};
    testTriangle._v1 = (Coord3){30.0f, 0.0f, 0.0f};
    testTriangle._v2 = (Coord3){0.0f, 30.0f, 0.0f};

    SetCameraFov(&camera);
    SetCameraScreen(&camera);

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
                if (event.key.keysym.sym == SDLK_w)
                {
                    camera._origin._z -= 100.0f;
                }
                if (event.key.keysym.sym == SDLK_s)
                {
                    camera._origin._z += 100.0f;
                }
                if (event.key.keysym.sym == SDLK_a)
                {
                    camera._origin._x += 100.0f;
                }
                if (event.key.keysym.sym == SDLK_d)
                {
                    camera._origin._x -= 100.0f;
                }
                if (event.key.keysym.sym == SDLK_SPACE)
                {
                    FILE *ptr;
                    ptr = fopen("test.csv","w");

                    fprintf(ptr,"\"x\", \"y\", \"z\"\n");
                    for (i=0; i<monCube._trianglesSize; ++i)
                    {
                        coord = monCube._triangles[i]._v0;
                        fprintf(ptr,"%.2f, %.2f, %.2f\n", coord._x, coord._y, coord._z);

                        coord = monCube._triangles[i]._v1;
                        fprintf(ptr,"%.2f, %.2f, %.2f\n", coord._x, coord._y, coord._z);

                        coord = monCube._triangles[i]._v2;
                        fprintf(ptr,"%.2f, %.2f, %.2f\n", coord._x, coord._y, coord._z);
                    }

                    fclose(ptr);
                }
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_FillRect(surfaceScreen, NULL, SDL_MapRGB(surfaceScreen->format, 0,0,0));
        ClearCamera(&camera);

        ApplyMatrixToObject(&rotationMatrixZ, &monCube);
        ApplyMatrixToObject(&rotationMatrixX, &monCube);

        clock_t before = clock();
        DrawObject3D(&monCube, &camera);
        clock_t difference = clock() - before;
        int msec = difference * 1000 / CLOCKS_PER_SEC;
        printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);

        DrawTriangles(&testTriangle, 1, &camera);

        ToSurfacePixels(surfaceScreen, &camera);

        SDL_Texture* textureScreen = SDL_CreateTextureFromSurface(renderer, surfaceScreen);
        SDL_RenderCopy(renderer, textureScreen, NULL, NULL);
        SDL_DestroyTexture(textureScreen);

        SDL_RenderPresent(renderer);
        //SDL_Delay(60);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}