#include "gRender.h"
#include "math.h"
#include "float.h"

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

void SetCameraFov(Camera* camera)
{
    camera->_scale = tanf(Deg2rad(camera->_fov * 0.5f));
}
void SetCameraScreen(Camera* camera)
{
    camera->_imageAspectRatio = (float)camera->_screenSize._w / (float)camera->_screenSize._h;
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
void RotateCamera(Camera* camera, float thetaX, float thetaY, float thetaZ)
{
    Matrix3x3 rotationMatrixX;
    Matrix3x3 rotationMatrixY;
    Matrix3x3 rotationMatrixZ;

    CreateRotationMatrixOnAxeX(&rotationMatrixX, thetaX);
    CreateRotationMatrixOnAxeY(&rotationMatrixY, thetaY);
    CreateRotationMatrixOnAxeZ(&rotationMatrixZ, thetaZ);

    Matrix3x3 buffer = camera->_rotationMatrix;
    MultiplyMatrixWithMatrix(&buffer, &rotationMatrixX, &camera->_rotationMatrix);
    buffer = camera->_rotationMatrix;
    MultiplyMatrixWithMatrix(&buffer, &rotationMatrixY, &camera->_rotationMatrix);
    buffer = camera->_rotationMatrix;
    MultiplyMatrixWithMatrix(&buffer, &rotationMatrixZ, &camera->_rotationMatrix);
}

void DrawTriangles(Triangle* triangles, Index trianglesSize, Camera* camera)
{
    Ray ray;
    uint16_t x, y;
    Coord3 intersectPoint;

    ray._origin = camera->_origin;

    float distance, u, v;
    float smallestDistance = FLT_MAX;
    Index i;

    Coord3 direction;

    for (y=0; y<camera->_screenSize._h; ++y)
    {
        for (x=0; x<camera->_screenSize._w; ++x)
        {
            //compute primary ray
            direction._x = -(2 * ((float)x + 0.5f) / (float)camera->_screenSize._w - 1) * camera->_imageAspectRatio * camera->_scale;
            direction._y = -(1 - 2 * ((float)y + 0.5f) / (float)camera->_screenSize._h) * camera->_scale;
            direction._z = -1.0f;
            MultiplyCoordWithMatrix(&camera->_rotationMatrix, &direction, &ray._direction);

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

        MultiplyCoordWithMatrix(matrix, &oldPos, &newPos);
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

        MultiplyCoordWithMatrix(matrix, &oldPos, &newPos);
        newPos._x += object->_center._x;
        newPos._y += object->_center._y;
        newPos._z += object->_center._z;

        object->_triangles[i]._v1 = newPos;

        //v2
        oldPos = object->_triangles[i]._v2;
        oldPos._x -= object->_center._x;
        oldPos._y -= object->_center._y;
        oldPos._z -= object->_center._z;

        MultiplyCoordWithMatrix(matrix, &oldPos, &newPos);
        newPos._x += object->_center._x;
        newPos._y += object->_center._y;
        newPos._z += object->_center._z;

        object->_triangles[i]._v2 = newPos;
    }
}
