#include "gMath.h"

typedef struct
{
    uint16_t _w, _h;
} ScreenSize;

typedef struct
{
    Coord3 _origin;

    ScreenSize _screenSize;
    uint8_t* _screenData;

    float _fov;
    float _scale;
    float _imageAspectRatio;
} Camera;

typedef struct
{
    Triangle* _triangles;
    Index _trianglesSize;

    Coord3 _center;
} Object3D;

void SetPixel(Camera* camera, uint16_t x, uint16_t y, uint8_t r, uint8_t g,  uint8_t b);

void SetCameraFov(Camera* camera);
void SetCameraScreen(Camera* camera);

void ClearCamera(Camera* camera);

void DrawTriangles(Triangle* triangles, Index trianglesSize, Camera* camera);
void DrawObject3D(Object3D* object, Camera* camera);

void ApplyMatrixToObject(Matrix3x3* matrix, Object3D* object);
