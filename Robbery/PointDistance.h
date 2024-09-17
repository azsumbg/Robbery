#pragma once

#ifdef POINTDISTANCE_EXPORTS
#define POINT_API _declspec(dllexport)
#else
#define POINT_API _declspec(dllimport)
#endif

#include <vector>

namespace distance
{
	struct POINT_API OnePoint
	{
		float x{};
		float y{};
	};

	extern POINT_API float FindDistance(OnePoint point1, OnePoint point2);

	extern "C" POINT_API bool SortPoints(std::vector<OnePoint>& vInput, OnePoint MyPoint, OnePoint& Output);
}
