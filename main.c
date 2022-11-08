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
    uint16_t _w, _h;
} ScreenSize;

typedef struct
{
    Coord3 _pos;
    float _scale;
    ScreenSize _screenSize;
    uint8_t* _screenData;
} Camera;

typedef struct
{
    float _row1[3];
    float _row2[3];
    float _row3[3];
} Matrix3x3;

typedef struct
{
    Coord3 _v0;
    Coord3 _v1;
    Coord3 _v2;
    Coord3 _normal;
} Triangle;

typedef struct
{
    Coord3 _origin;
    Coord3 _direction;
} Ray;

typedef struct
{
    Triangle* _triangles;
    Index _trianglesSize;

    Coord3 _center;
} Object3D;

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
void Normalize(Coord3* a)
{
    float mag = GetMagnitudeFromCoord(a);
    a->_x /= mag;
    a->_y /= mag;
    a->_z /= mag;
}

uint8_t RayIntersect(Ray* ray, Triangle* triangle, Coord3* intersectPoint, float* distance, float* u, float* v)
{
    //https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution
    float dotParallel = GetDotProductFrom2Coord(&triangle->_normal, &ray->_direction);
    const float kEpsilon = 1e-8f;

    //The Ray And The Triangle Are Parallel
    if (fabsf(dotParallel) < kEpsilon)
    {
        return 0;
    }

    float planeDistance = GetDotProductFrom2Coord(&triangle->_normal, &triangle->_v0);
    *distance = - ((GetDotProductFrom2Coord(&triangle->_normal, &ray->_origin) + planeDistance) / dotParallel);

    //The triangle is "behind" the ray
    if (*distance < 0.0f)
    {
        return 0;
    }

    float area = GetMagnitudeFromCoord(&triangle->_normal) / 2.0f;  //area of the triangle

    // compute the intersection point
    intersectPoint->_x = -(ray->_origin._x + *distance*ray->_direction._x);
    intersectPoint->_y = -(ray->_origin._y + *distance*ray->_direction._y);
    intersectPoint->_z = -(ray->_origin._z + *distance*ray->_direction._z);

    Coord3 edge;
    Coord3 C;
    Coord3 vp;

    //edge 0
    edge._x = triangle->_v1._x - triangle->_v0._x;
    edge._y = triangle->_v1._y - triangle->_v0._y;
    edge._z = triangle->_v1._z - triangle->_v0._z;

    vp._x = intersectPoint->_x - triangle->_v0._x;
    vp._y = intersectPoint->_y - triangle->_v0._y;
    vp._z = intersectPoint->_z - triangle->_v0._z;

    GetCrossProductFrom2Coord(&edge, &vp, &C);

    if (GetDotProductFrom2Coord(&triangle->_normal, &C) < 0.0f)
    {
        return 0;
    }

    //edge 1
    edge._x = triangle->_v2._x - triangle->_v1._x;
    edge._y = triangle->_v2._y - triangle->_v1._y;
    edge._z = triangle->_v2._z - triangle->_v1._z;

    vp._x = intersectPoint->_x - triangle->_v1._x;
    vp._y = intersectPoint->_y - triangle->_v1._y;
    vp._z = intersectPoint->_z - triangle->_v1._z;

    GetCrossProductFrom2Coord(&edge, &vp, &C);

    *u = (GetMagnitudeFromCoord(&C) / 2.0f) / area;

    if (GetDotProductFrom2Coord(&triangle->_normal, &C) < 0.0f)
    {
        return 0;
    }

    //edge 2
    edge._x = triangle->_v0._x - triangle->_v2._x;
    edge._y = triangle->_v0._y - triangle->_v2._y;
    edge._z = triangle->_v0._z - triangle->_v2._z;

    vp._x = intersectPoint->_x - triangle->_v2._x;
    vp._y = intersectPoint->_y - triangle->_v2._y;
    vp._z = intersectPoint->_z - triangle->_v2._z;

    GetCrossProductFrom2Coord(&edge, &vp, &C);

    *v = (GetMagnitudeFromCoord(&C) / 2.0f) / area;

    if (GetDotProductFrom2Coord(&triangle->_normal, &C) < 0.0f)
    {
        return 0;
    }

    // implementing the single/double sided feature
    /*if (GetDotProductFrom2Coord(&ray->_direction, &triangle->_normal) > 0.0f)
    {
        return 0;
    }*/

    return 1;
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

void SetPixel(Camera* camera, uint16_t x, uint16_t y, uint8_t r, uint8_t g,  uint8_t b)
{
    if (x>=camera->_screenSize._w || y>=camera->_screenSize._h)
    {
        return;
    }

    uint8_t* const pixelData = (camera->_screenData
                                + ((uint32_t)y * camera->_screenSize._w + (uint32_t)x) * 3);
    *pixelData = r;
    *(pixelData+1) = g;
    *(pixelData+2) = b;
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

void ClearCamera(Camera* camera)
{
    uint32_t size = camera->_screenSize._w * camera->_screenSize._h * 3;
    uint32_t i;

    for (i=0; i<size; ++i)
    {
        camera->_screenData[i] = 0x00;
    }
}

float Deg2rad(float deg)
{
    return deg * ((float)M_PI / 180.0f);
}

void GetTriangleNormal(Triangle* triangle)
{
    Coord3 A;
    Coord3 B;

    //v1 - v0
    A._x = triangle->_v1._x - triangle->_v0._x;
    A._y = triangle->_v1._y - triangle->_v0._y;
    A._z = triangle->_v1._z - triangle->_v0._z;

    //v2 - v0
    B._x = triangle->_v2._x - triangle->_v0._x;
    B._y = triangle->_v2._y - triangle->_v0._y;
    B._z = triangle->_v2._z - triangle->_v0._z;

    GetCrossProductFrom2Coord(&A, &B, &triangle->_normal);
}

void DrawTriangles(Triangle* triangles, Index trianglesSize, Camera* camera)
{
    Ray ray;
    uint16_t x, y;
    uint8_t intersect;
    Coord3 intersectPoint;

    ray._origin = camera->_pos;

    float fov = 51.52f;
    float scale = tanf(Deg2rad(fov * 0.5f));
    float imageAspectRatio = (float)camera->_screenSize._w / (float)camera->_screenSize._h;
    float distance, u, v;
    float smallestDistance = FLT_MAX;
    Index i;

    for (y=0; y<camera->_screenSize._h; ++y)
    {
        for (x=0; x<camera->_screenSize._w; ++x)
        {
            // compute primary ray
            ray._direction._x = -(2 * ((float)x + 0.5f) / (float)camera->_screenSize._w - 1) * imageAspectRatio * scale;
            ray._direction._y = -(1 - 2 * ((float)y + 0.5f) / (float)camera->_screenSize._h) * scale;
            ray._direction._z = -1.0f;
            Normalize(&ray._direction);

            for (i=0; i<trianglesSize; ++i)
            {
                if (x==0 && y==0)
                {
                    GetTriangleNormal(triangles+i);
                }

                if ( RayIntersect(&ray, triangles+i, &intersectPoint, &distance, &u, &v) == 1 )
                {
                    if (distance < smallestDistance)
                    {
                        smallestDistance = distance;
                        SetPixel(camera, x,y, (uint8_t)(u * 255.0f),(uint8_t)(v*255.0f),(uint8_t)((1.0f - u - v) * 255.0f));
                    }
                }
            }

            smallestDistance = FLT_MAX;
        }
    }
}
void DrawObject3D(Object3D* object, Camera* camera)
{
    DrawTriangles(object->_triangles, object->_trianglesSize, camera);
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

    //printf("------------------------\n");
    for (i=0; i<object->_trianglesSize; ++i)
    {
        //v0
        oldPos = object->_triangles[i]._v0;
        oldPos._x -= object->_center._x;
        oldPos._y -= object->_center._y;
        oldPos._z -= object->_center._z;

        MultiplyMatrixWithCoord(matrix, &oldPos, &newPos);
        newPos._x += object->_center._x;
        newPos._y += object->_center._y;
        newPos._z += object->_center._z;

        object->_triangles[i]._v0 = newPos;

        //printf("x: %f.2 y: %f.2 z: %f.2\n", newPos._x, newPos._y, newPos._z);

        //v1
        oldPos = object->_triangles[i]._v1;
        oldPos._x -= object->_center._x;
        oldPos._y -= object->_center._y;
        oldPos._z -= object->_center._z;

        MultiplyMatrixWithCoord(matrix, &oldPos, &newPos);
        newPos._x += object->_center._x;
        newPos._y += object->_center._y;
        newPos._z += object->_center._z;

        object->_triangles[i]._v1 = newPos;

        //v2
        oldPos = object->_triangles[i]._v2;
        oldPos._x -= object->_center._x;
        oldPos._y -= object->_center._y;
        oldPos._z -= object->_center._z;

        MultiplyMatrixWithCoord(matrix, &oldPos, &newPos);
        newPos._x += object->_center._x;
        newPos._y += object->_center._y;
        newPos._z += object->_center._z;

        object->_triangles[i]._v2 = newPos;
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
Camera camera = {{0.0f,0.0f,0.0f}, 1.0f, {640,320}, (uint8_t*)&screen};

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
                    camera._pos._z -= 100.0f;
                }
                if (event.key.keysym.sym == SDLK_s)
                {
                    camera._pos._z += 100.0f;
                }
                if (event.key.keysym.sym == SDLK_a)
                {
                    camera._pos._x += 100.0f;
                }
                if (event.key.keysym.sym == SDLK_d)
                {
                    camera._pos._x -= 100.0f;
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

        DrawObject3D(&monCube, &camera);
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