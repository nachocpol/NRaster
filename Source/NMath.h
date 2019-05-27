#pragma once

namespace math
{
	struct Vec2
	{
		Vec2() 
			: x(0.0f)
			, y(0.0f)
		{
		}
		Vec2(float _x, float _y)
			: x(_x)
			, y(_y)
		{
		}

		float x;
		float y;
	};

	struct Vec3
	{
		Vec3()
			: x(0.0f)
			, y(0.0f)
			, z(0.0f)
		{
		}
		Vec3(float _x, float _y, float _z)
			: x(_x)
			, y(_y)
			, z(_z)
		{
		}

		float x;
		float y;
		float z;
	};
}