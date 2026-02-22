#pragma once

#ifdef MINES_EXPORTS
#define MINES_API __declspec(dllexport)
#else 
#define MINES_API __declspec(dllimport)
#endif

#include <random>

constexpr float scr_width{ 800.0f };
constexpr float scr_height{ 600.0f };

constexpr float sky{ 50.0f };
constexpr float ground{ 50.0f };

constexpr float CELL_DIM{ 50.0f };

constexpr int LEVEL1_ROWS{ 9 };
constexpr int LEVEL1_COLS{ 5 };

constexpr int LEVEL2_ROWS{ 11 };
constexpr int LEVEL2_COLS{ 6 };

constexpr int LEVEL3_ROWS{ 13 };
constexpr int LEVEL3_COLS{ 8 };

constexpr int LEVEL4_ROWS{ 15 };
constexpr int LEVEL4_COLS{ 10 };

constexpr int LEVEL1_MINES{ 8 };
constexpr int LEVEL2_MINES{ 10 };
constexpr int LEVEL3_MINES{ 13 };
constexpr int LEVEL4_MINES{ 22 };

constexpr int NO_MINE{ 0 };
constexpr int MINE{ 10 };
constexpr int MARKED_MINE{ 9 };
constexpr int UNKNOWN_MINE{ -1 };

struct FRECT
{
	float left{ 0 };
	float up{ 0 };
	float right{ 0 };
	float down{ 0 };
};
struct TILE
{
	int content{ 0 };
	
	int mines_arround{ 0 };

	FRECT dims{};

	bool selected = false;
};

namespace dll
{
	class MINES_API RANDIT
	{
	private:

		std::mt19937* twister{ nullptr };

	public:
		RANDIT();
		~RANDIT();

		int operator()(int min, int max);
	};

	class MINES_API GRID
	{
	private:
		int rows{};
		int cols{};
		int all_mines{ 0 };
		TILE** array{ nullptr };

		RANDIT RandGen{};
	
		void MinesArround(int row, int col);

	public:
		GRID(int rows_number, int cols_number, int level);
		~GRID();
		void Release();

		int MinesRemaining()const;
		void MineMarked(int row, int col, bool mark_it);
		bool IsTileSelected(int row, int col) const;
		int SelectTile(int row, int col);
		int ShowTileInfo(int row, int col);
		FRECT GetTileDims(int row, int col) const;
	};

}