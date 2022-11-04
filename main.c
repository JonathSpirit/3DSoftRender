#include "SDL.h"
#include "math.h"
#include "stdint.h"

typedef uint16_t Index;

typedef struct
{
    float _x, _y, _z;
} Coord3;

typedef struct
{
    unsigned char _w, _h;
} ScreenSize;

typedef struct
{
    Coord3 _pos;
    float _scale;
    ScreenSize _screenSize;
} Camera;

typedef struct
{
    Index _pointA;
    Index _pointB;
} Line3;

typedef struct
{
    Coord3* _points;
    Index _pointsSize;

    Line3* _lines;
    Index _linesSize;

    Coord3 _center;
} Object3D;

typedef struct
{
    float _row1[3];
    float _row2[3];
    float _row3[3];
} Matrix3x3;

//https://people.eecs.ku.edu/~jrmiller/Courses/VectorGeometry/VectorOperations.html
float GetMagnitudeFromCoord(Coord3* a)
{
    return sqrtf( a->_x*a->_x + a->_y*a->_y + a->_z*a->_z );
}
float GetDotProductFrom2Coord(Coord3* a, Coord3* b)
{
    return a->_x*b->_x + a->_y*b->_y + a->_z*b->_z;
}
void GetCrossProductFrom2Coord(Coord3* a, Coord3* b, Coord3* result)
{
    result->_x = -a->_z*b->_y + a->_y*b->_z;
    result->_y = a->_z*b->_x - a->_x*b->_z;
    result->_z = -a->_y*b->_x + a->_x*b->_y;
}
float GetDistanceBetween2Coord(Coord3* a, Coord3* b)
{
    Coord3 buff = {b->_x-a->_x, b->_y-a->_y, b->_z-a->_z};
    return sqrtf( buff._x*buff._x + buff._y*buff._y + buff._z*buff._z );
}
float GetAngleBetween2Coord(Coord3* a, Coord3* b)
{
    Coord3 crossResult;
    GetCrossProductFrom2Coord(a, b, &crossResult);
    // |A·B| = |A| |B| COS(?)
    // |A×B| = |A| |B| SIN(?)

    return atan2f( GetMagnitudeFromCoord(&crossResult), GetDotProductFrom2Coord(a, b) );
    //return Math.Atan2(Cross(A,B), Dot(A,B));
}

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

uint8_t r,g,b;
void DrawPoint(SDL_Surface* surface, Coord3* coord, Camera* camera)
{
    static Coord3 axeXp = {1.0f, 0.0f, 0.0f};
    static Coord3 axeYp = {0.0f, 1.0f, 0.0f};

    float angle;
    float mag, distance;
    float resultx, resulty;

    angle = GetAngleBetween2Coord(&axeXp, coord);
    mag = GetDistanceBetween2Coord(&camera->_pos, coord);
    resultx = cosf(angle)*mag;
    angle = GetAngleBetween2Coord(&axeYp, coord);
    resulty = cosf(angle)*mag;

    //printf("Drawing at x:%d y:%d with color r:%u g:%u b:%u\n", (int)resultx, (int)resulty, r, g, b);
    SetSurfacePixel(surface, (int)resultx, (int)resulty, (SDL_Color){r, g, b, 255});
}
void DrawObject3D(SDL_Surface* surface, Object3D* object, Camera* camera)
{
    Index i, points;

    float angle;
    float mag, distance;
    float resultx, resulty;

    Coord3 coordNormal, coordA, coordB;

    static Coord3 axeXp = {1.0f, 0.0f, 0.0f};
    static Coord3 axeYp = {0.0f, 1.0f, 0.0f};

    //Drawing points
    /*for (i=0; i<object->_pointsSize; ++i)
    {
        DrawPoint(surface, &object->_points[i], camera);
    }*/

    //Drawing lines
    for (i=0; i<object->_linesSize; ++i)
    {
        coordA = object->_points[ object->_lines[i]._pointA ];
        coordB = object->_points[ object->_lines[i]._pointB ];

        mag = GetDistanceBetween2Coord(&coordA, &coordB);

        coordNormal._x = (coordA._x - coordB._x)/mag;
        coordNormal._y = (coordA._y - coordB._y)/mag;
        coordNormal._z = (coordA._z - coordB._z)/mag;

        distance = mag / 500.0f;
        for (points=0; points<501; ++points)
        {
            coordB._x = coordA._x + coordNormal._x*-distance*(float)points;
            coordB._y = coordA._y + coordNormal._y*-distance*(float)points;
            coordB._z = coordA._z + coordNormal._z*-distance*(float)points;

            DrawPoint(surface, &coordB, camera);
        }
    }
}

void CreateRotationMatrixOnAxeZ(Matrix3x3* matrix, float theta)
{
    matrix->_row1[0] = cosf(theta);
    matrix->_row1[1] = -sinf(theta);
    matrix->_row1[2] = 0.0f;

    matrix->_row2[0] = sinf(theta);
    matrix->_row2[1] = cosf(theta);
    matrix->_row2[2] = 0.0f;

    matrix->_row3[0] = 0.0f;
    matrix->_row3[1] = 0.0f;
    matrix->_row3[2] = 1.0f;
}
void CreateRotationMatrixOnAxeX(Matrix3x3* matrix, float theta)
{
    matrix->_row1[0] = 1.0f;
    matrix->_row1[1] = 0.0f;
    matrix->_row1[2] = 0.0f;

    matrix->_row2[0] = 0.0f;
    matrix->_row2[1] = cosf(theta);
    matrix->_row2[2] = -sinf(theta);

    matrix->_row3[0] = 0.0f;
    matrix->_row3[1] = sinf(theta);
    matrix->_row3[2] = cosf(theta);
}

void MultiplyMatrixWithCoord(Matrix3x3* matrix, Coord3* coord, Coord3* result)
{
    result->_x = matrix->_row1[0]*coord->_x + matrix->_row1[1]*coord->_y + matrix->_row1[2]*coord->_z;
    result->_y = matrix->_row2[0]*coord->_x + matrix->_row2[1]*coord->_y + matrix->_row2[2]*coord->_z;
    result->_z = matrix->_row3[0]*coord->_x + matrix->_row3[1]*coord->_y + matrix->_row3[2]*coord->_z;
}

void ApplyMatrixToObject(Matrix3x3* matrix, Object3D* object)
{
    Coord3 oldPos;
    Coord3 newPos;
    Index i;

    for (i=0; i<object->_pointsSize; ++i)
    {
        oldPos = object->_points[i];
        oldPos._x -= object->_center._x;
        oldPos._y -= object->_center._y;
        oldPos._z -= object->_center._z;

        MultiplyMatrixWithCoord(matrix, &oldPos, &newPos);
        newPos._x += object->_center._x;
        newPos._y += object->_center._y;
        newPos._z += object->_center._z;

        object->_points[i] = newPos;
    }
}

Coord3 monCubePoints[8] = {{0.0f,0.0f,0.0f}, {20.0f,0.0f,0.0f}, {0.0f, 0.0f, 20.0f}, {20.0f, 0.0f, 20.0f},
                           {0.0f,20.0f,0.0f}, {20.0f,20.0f,0.0f}, {0.0f, 20.0f, 20.0f}, {20.0f, 20.0f, 20.0f}};

Line3 monCubeLines[12] = {{0,1}, {0,2}, {0,4},
                          {7,6}, {7,5}, {7,3},
                          {3,2}, {3,1},
                          {4,6}, {4,5},
                          {6,2}, {5,1}};

Object3D monCube = {monCubePoints, 8, monCubeLines, 12, {10.0f,10.0f,10.0f}};

Camera camera = {{0.0f,0.0f,0.0f}, 1.0f, {64,32}};

Matrix3x3 rotationMatrixZ, rotationMatrixX;

int main(int argc, char *argv[])
{
    Index i;
    Coord3 coord;
    unsigned char count=0;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
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
    for (i=0; i<monCube._pointsSize; ++i)
    {
        coord = monCube._points[i];
        coord._x += 22.0f;
        coord._y += 6.0f;
        coord._z += 0.0f;

        coord._x *= 10.0f;
        coord._y *= 10.0f;
        coord._z *= 10.0f;

        monCube._points[i] = coord;
    }
    monCube._center._x += 22.0f;
    monCube._center._y += 6.0f;
    monCube._center._z += 0.0f;

    monCube._center._x *= 10.0f;
    monCube._center._y *= 10.0f;
    monCube._center._z *= 10.0f;

    while (quit == 0)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = 1;
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_FillRect(surfaceScreen, NULL, SDL_MapRGB(surfaceScreen->format, 0,0,0));

        ApplyMatrixToObject(&rotationMatrixZ, &monCube);
        ApplyMatrixToObject(&rotationMatrixX, &monCube);

        DrawObject3D(surfaceScreen, &monCube, &camera);

        ++count;

        if ((count&0x07) == 0)
        {
            count = 1;
        }

        r=0xFF;
        g=0xFF;
        b=0xFF;

        SDL_Texture* textureScreen = SDL_CreateTextureFromSurface(renderer, surfaceScreen);
        SDL_RenderCopy(renderer, textureScreen, NULL, NULL);
        SDL_DestroyTexture(textureScreen);

        SDL_RenderPresent(renderer);
        SDL_Delay(60);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}