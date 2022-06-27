#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef CG_VECTOR2_H
#define CG_VECTOR2_H

//Vector2 - X and Y Coordinates
typedef struct Vector2Struct
{
	int x;
	int y;
} Vector2;

/* FUNCTION DECLARATIONS */
// Creating a 2-D Vector
Vector2 CreateVec2(const int x, const int y);

// Adding Two Vectors (v1 + v2)
Vector2 Vec2_Add(const Vector2 v1, const Vector2 v2);

// Subtracting Two Vectors (v1 - v2)
Vector2 Vec2_Sub(const Vector2 v1, const Vector2 v2);

// Multiplying By Scalar
Vector2 Vec2_Mul(const Vector2 v, const int Multiplier);

// Dividing by a Scalar
Vector2 Vec2_Div(const Vector2 v, const int Div);

// Dot Product
int Vec2_Dot(const Vector2 v1, const Vector2 v2);

// Magnitude
int Vec2_Mag(const Vector2 v);

#endif //CG_VECTOR2_H
