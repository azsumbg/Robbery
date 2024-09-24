#pragma once

#ifdef CONTAINER_EXPORTS
#define CONTAINER_API __declspec(dllexport)
#else 
#define CONTAINER_API __declspec(dllimport)
#endif


namespace point_cont_space
{
	struct CONTAINER_API OnePoint
	{
		float x{ 0 };
		float y{ 0 };
	};

	class CONTAINER_API CONTAINER
	{
		private:
			OnePoint* m_ArrayPointer{ nullptr };
			size_t m_size{ 0 };
			size_t max_size{ 0 };
			
		public:

			CONTAINER(size_t size) :max_size{ size }, m_ArrayPointer{ new OnePoint[size]{} } {}

			~CONTAINER()
			{
				if (m_size > 0)delete[] m_ArrayPointer;
			}

			bool empty() const;
			size_t size() const;

			void push_back(OnePoint data);
			
			OnePoint& operator [] (size_t position) const;
	};

	float CONTAINER_API FindDistance(OnePoint start, OnePoint end);

	OnePoint CONTAINER_API FindNearestPoint(OnePoint MyPoint, CONTAINER& PointsToCheck);
}
