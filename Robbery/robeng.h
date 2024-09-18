#pragma once

#ifdef ROBENG_EXPORTS 
#define ROBENG_API _declspec(dllexport)
#else 
#define ROBENG_API _declspec(dllimport)
#endif

#include <random>
#include <chrono>
#include <vector>
#include <d2d1.h>

constexpr float scr_width = 1000.0f;
constexpr float scr_height = 800.0f;
constexpr float ground_y = 750.0f;

enum class dirs { up = 0, down = 1, left = 2, right = 3, stop = 4 };
enum class fields { clay = 0, desert = 1, gold = 2, grain = 3, grass = 4, sea = 5, wood = 6,treasure = 7 };
enum class creatures { fat = 0, lazy = 1, moto = 2, serge = 3, hero = 4 };

namespace dll
{
	struct ROBENG_API SPOT
	{
		float x = 0;
		float y = 0;
		float ex = 49.0f;
		float ey = 49.0f;
		int row = 0;
		int col = 0;
		fields type = fields::clay;
		float speed_modifier = 0;
		const float width = 50.0f;
		const float height = 50.0f;
	};

	struct ROBENG_API AI_INPUT
	{
		float hero_x{};
		float hero_y{};

		float guard_obj_x{};
		float guard_obj_y{};
		
		float move_gear{};

		bool distract_on = false;
		
		bool need_to_set_patrol = false;
		bool horizontal_patrol = false;

		float distracter_x = -1.0f;
		float distracter_y = -1.0f;
	};
	
	class ROBENG_API FIELD
	{
	private:

		SPOT GameField[14][20]{};

		std::random_device rd{};
		std::vector<int>vSeed;
		std::seed_seq* seed_sequencer = nullptr;
		std::mt19937* twister = nullptr;

		fields OneSpot();

	public:

		FIELD();
		virtual ~FIELD() {};
		
		fields GetSpotType(int row, int col) const;

		SPOT GetSpotDims(int row, int col) const;

		void operator () (int row, int col, fields new_type);

		FIELD& operator =(FIELD& new_field);

	};

	class ROBENG_API ATOM
	{
		protected:
			float width = 0;
			float height = 0;

		public:
			float x = 0;
			float y = 0;
			float ex = 0;
			float ey = 0;

			ATOM(float sx, float sy, float swidth = 1.0f, float sheight = 1.0f);
			virtual ~ATOM() {};

			float GetWidth()const;
			float GetHeight()const;
			void SetEdges();
			void NewDims(float new_width, float new_height);

			void operator() (float new_width, float new_height);
	};

	class ROBENG_API HERO :public ATOM
	{
		private:
			float speed = 1.0f;

			float move_start_x = 0;
			float move_start_y = 0;
			float move_end_x = 0;
			float move_end_y = 0;

			float slope = 0;
			float intercept = 0;

			bool hor_line = false;
			bool ver_line = false;

			int current_frame = 0;
			int last_frame = 9;
			int frame_delay = 10;

			HERO(float first_x, float first_y) :ATOM(first_x, first_y, 40.0f, 34.0f)
			{
				dir = dirs::right;
			}

		public:
			dirs dir = dirs::stop;

			friend void* CreatureFactory(creatures creature, float first_x, float first_y);

			void Release();

			int GetFrame();

			void LineSetup(float to_where_x,float to_where_y);
			void Move(float gear);
	};

	class ROBENG_API POLICE :public ATOM
	{
	private:
		creatures type = creatures::fat;
		float speed = 0.8f;

		D2D1_RECT_F SeeRect{};
		D2D1_RECT_F PatrolRect{};

		float move_start_x = 0;
		float move_start_y = 0;
		float move_end_x = 0;
		float move_end_y = 0;

		float slope = 0;
		float intercept = 0;

		bool hor_line = false;
		bool ver_line = false;
		
		int current_frame = 0;
		int last_frame = 9;
		int frame_delay = 10;
		int current_frame_delay = 0;

		POLICE(creatures what, float sx, float sy) :ATOM(sx, sy)
		{
			type = what;
			switch (type)
			{
			case creatures::lazy:
				NewDims(33.0f, 50.0f);
				speed = 0.5f;
				last_frame = 1;
				frame_delay = 25;
				break;

			case creatures::moto:
				NewDims(85.0f, 50.0f);
				speed = 2.0f;
				last_frame = 19;
				frame_delay = 5;
				break;

			case creatures::serge:
				NewDims(22.0f, 50.0f);
				speed = 1.0f;
				last_frame = 7;
				frame_delay = 7;
				break;

			case creatures::fat:
				NewDims(34.0f, 50.0f);
				speed = 0.8f;
				last_frame = 9;
				frame_delay = 10;
				break;
			}
		}

		void LineSetup(float endx, float endy);
		void Run(float gear);
		void Patrol(float gear);

	public:

		dirs dir = dirs::right;
		bool now_patroling = false;

		friend void* CreatureFactory(creatures creature, float first_x, float first_y);

		void Release();

		int GetFrame();

		creatures GetType() const;

		void AIDispatcher(AI_INPUT Info);
	};

	typedef HERO* hero_ptr;
	typedef ATOM* atom_ptr;
	typedef POLICE* police_ptr;

	//FACTORY FUNCTION *********************************

	void* CreatureFactory(creatures creature, float first_x, float first_y)
	{
		void* ret = nullptr;

		switch (creature)
		{
		case creatures::hero:
			ret = new HERO(first_x, first_y);
			break;

		case creatures::fat:
			ret = new POLICE(creatures::fat, first_x, first_y);
			break;

		case creatures::lazy:
			ret = new POLICE(creatures::lazy, first_x, first_y);
			break;

		case creatures::moto:
			ret = new POLICE(creatures::moto, first_x, first_y);
			break;

		case creatures::serge:
			ret = new POLICE(creatures::serge, first_x, first_y);
			break;
		}
		return ret;
	}
}