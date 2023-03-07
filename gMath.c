#include "gMath.h"
#define _USE_MATH_DEFINES
#include "math.h"
#include "float.h"

float GetInverseMagnitudeFromCoord(Coord3* a)
{
    float x = a->_x * a->_x + a->_y * a->_y + a->_z * a->_z;
    const float xhalf = 0.5f * x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i >> 1);
    x = *(float*)&i;
    x = x*(1.5f-(xhalf*x*x));
    //x = x*(1.5f-(xhalf*x*x));
    return x;

    //return 1.0f / sqrtf( a->_x*a->_x + a->_y*a->_y + a->_z*a->_z );
}
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
    float mag = GetInverseMagnitudeFromCoord(a);
    a->_x *= mag;
    a->_y *= mag;
    a->_z *= mag;
}

uint8_t RayIntersect(Ray* ray, Triangle* triangle, Coord3* intersectPoint, float* distance, float* u, float* v)
{
    //https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution
    float dotParallel = GetDotProductFrom2Coord(&triangle->_normal, &ray->_direction);

    //The Ray And The Triangle Are Parallel
    if (fabsf(dotParallel) < FLT_EPSILON)
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

    float area = 0.5f / GetInverseMagnitudeFromCoord(&triangle->_normal); //area of the triangle

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

    *u = (0.5f / GetInverseMagnitudeFromCoord(&C)) / area;

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

    *v = (0.5f / GetInverseMagnitudeFromCoord(&C)) / area;

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
void CreateRotationMatrixOnAxeY(Matrix3x3* matrix, float theta)
{
    matrix->_row1[0] = cosf(theta);
    matrix->_row1[1] = 0.0f;
    matrix->_row1[2] = sinf(theta);

    matrix->_row2[0] = 0.0f;
    matrix->_row2[1] = 1.0f;
    matrix->_row2[2] = 0.0f;

    matrix->_row3[0] = -sinf(theta);
    matrix->_row3[1] = 0.0f;
    matrix->_row3[2] = cosf(theta);
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
void CreateTranslationMatrix(Matrix3x3* matrix, float x, float y, float z)
{
    matrix->_row1[0] = 1.0f;
    matrix->_row1[1] = 0.0f;
    matrix->_row1[2] = 0.0f;

    matrix->_row2[0] = 0.0f;
    matrix->_row2[1] = 1.0f;
    matrix->_row2[2] = 0.0f;

    matrix->_row3[0] = x;
    matrix->_row3[1] = y;
    matrix->_row3[2] = z;
}
void CreateIdentityMatrix(Matrix3x3* matrix)
{
    matrix->_row1[0] = 1.0f;
    matrix->_row1[1] = 0.0f;
    matrix->_row1[2] = 0.0f;

    matrix->_row2[0] = 0.0f;
    matrix->_row2[1] = 1.0f;
    matrix->_row2[2] = 0.0f;

    matrix->_row3[0] = 0.0f;
    matrix->_row3[1] = 0.0f;
    matrix->_row3[2] = 1.0f;
}

void MultiplyCoordWithMatrix(Matrix3x3* matrix, Coord3* coord, Coord3* result)
{
    result->_x = matrix->_row1[0]*coord->_x + matrix->_row1[1]*coord->_y + matrix->_row1[2]*coord->_z;
    result->_y = matrix->_row2[0]*coord->_x + matrix->_row2[1]*coord->_y + matrix->_row2[2]*coord->_z;
    result->_z = matrix->_row3[0]*coord->_x + matrix->_row3[1]*coord->_y + matrix->_row3[2]*coord->_z;
}
void MultiplyCoordWithMatrixInPlace(Matrix3x3* matrix, Coord3* coord)
{
    Coord3 result;
    result._x = matrix->_row1[0]*coord->_x + matrix->_row1[1]*coord->_y + matrix->_row1[2]*coord->_z;
    result._y = matrix->_row2[0]*coord->_x + matrix->_row2[1]*coord->_y + matrix->_row2[2]*coord->_z;
    result._z = matrix->_row3[0]*coord->_x + matrix->_row3[1]*coord->_y + matrix->_row3[2]*coord->_z;
    *coord = result;
}
void MultiplyMatrixWithMatrix(Matrix3x3* matrixA, Matrix3x3* matrixB, Matrix3x3* result)
{
    result->_row1[0] = matrixA->_row1[0]*matrixB->_row1[0] + matrixA->_row1[1]*matrixB->_row2[0] + matrixA->_row1[2]*matrixB->_row3[0];
    result->_row1[1] = matrixA->_row1[0]*matrixB->_row1[1] + matrixA->_row1[1]*matrixB->_row2[1] + matrixA->_row1[2]*matrixB->_row3[1];
    result->_row1[2] = matrixA->_row1[0]*matrixB->_row1[2] + matrixA->_row1[1]*matrixB->_row2[2] + matrixA->_row1[2]*matrixB->_row3[2];

    result->_row2[0] = matrixA->_row2[0]*matrixB->_row1[0] + matrixA->_row2[1]*matrixB->_row2[0] + matrixA->_row2[2]*matrixB->_row3[0];
    result->_row2[1] = matrixA->_row2[0]*matrixB->_row1[1] + matrixA->_row2[1]*matrixB->_row2[1] + matrixA->_row2[2]*matrixB->_row3[1];
    result->_row2[2] = matrixA->_row2[0]*matrixB->_row1[2] + matrixA->_row2[1]*matrixB->_row2[2] + matrixA->_row2[2]*matrixB->_row3[2];

    result->_row3[0] = matrixA->_row3[0]*matrixB->_row1[0] + matrixA->_row3[1]*matrixB->_row2[0] + matrixA->_row3[2]*matrixB->_row3[0];
    result->_row3[1] = matrixA->_row3[0]*matrixB->_row1[1] + matrixA->_row3[1]*matrixB->_row2[1] + matrixA->_row3[2]*matrixB->_row3[1];
    result->_row3[2] = matrixA->_row3[0]*matrixB->_row1[2] + matrixA->_row3[1]*matrixB->_row2[2] + matrixA->_row3[2]*matrixB->_row3[2];
}