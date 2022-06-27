#include "Vector2.h"

/* FUNCTION DEFINITIONS */

Vector2 CreateVec2(const int x, const int y)
{
	return (Vector2){x, y};
}

Vector2 Vec2_Add(const Vector2 v1, const Vector2 v2)
{
	return CreateVec2(v1.x + v2.x, v1.y + v2.y);
}

Vector2 Vec2_Sub(const Vector2 v1, const Vector2 v2)
{
	return CreateVec2(v1.x - v2.x, v1.y - v2.y);
}

Vector2 Vec2_Mul(const Vector2 v, const int Multiplier)
{
	return CreateVec2(v.x * Multiplier, v.y * Multiplier);
}


Vector2 Vec2_Div(const Vector2 v, const int Div)
{
	if (Div == 0)
		return CreateVec2(0, 0);

	int x = (int)round((float)v.x / (float)Div);
	int y = (int)round((float)v.y / (float)Div);
	
	return CreateVec2(x, y);
}

int Vec2_Dot(const Vector2 v1, const Vector2 v2)
{
	return ((v1.x * v2.x) + (v1.y * v2.y));
}

int Vec2_Mag(const Vector2 v)
{
	return (int)round(sqrtf((float)((v.x * v.x) + (v.y * v.y))));
}

