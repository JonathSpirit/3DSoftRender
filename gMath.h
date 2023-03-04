#include "stdint.h"

typedef uint16_t Index;

typedef struct
{
    float _x, _y, _z;
} Coord3;

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

//https://people.eecs.ku.edu/~jrmiller/Courses/VectorGeometry/VectorOperations.html
float GetMagnitudeFromCoord(Coord3* a);
float GetDotProductFrom2Coord(Coord3* a, Coord3* b);
void GetCrossProductFrom2Coord(Coord3* a, Coord3* b, Coord3* result);
float GetDistanceBetween2Coord(Coord3* a, Coord3* b);
float GetAngleBetween2Coord(Coord3* a, Coord3* b);
void Normalize(Coord3* a);

uint8_t RayIntersect(Ray* ray, Triangle* triangle, Coord3* intersectPoint, float* distance, float* u, float* v);

float Deg2rad(float deg);

void GetTriangleNormal(Triangle* triangle);

void CreateRotationMatrixOnAxeX(Matrix3x3* matrix, float theta);
void CreateRotationMatrixOnAxeY(Matrix3x3* matrix, float theta);
void CreateRotationMatrixOnAxeZ(Matrix3x3* matrix, float theta);
void CreateTranslationMatrix(Matrix3x3* matrix, float x, float y, float z);
void CreateIdentityMatrix(Matrix3x3* matrix);

void MultiplyCoordWithMatrix(Matrix3x3* matrix, Coord3* coord, Coord3* result);
void MultiplyCoordWithMatrixInPlace(Matrix3x3* matrix, Coord3* coord);
void MultiplyMatrixWithMatrix(Matrix3x3* matrixA, Matrix3x3* matrixB, Matrix3x3* result);