#include "framework.h"
#include "New Minesweeper.h"
#include <mmsystem.h>
#include <d2d1.h>
#include <dwrite.h>
#include "FCheck.h"
#include "ErrH.h"
#include "D2BMPLOADER.h"
#include "mines.h"
#include <chrono>
#include <clocale>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "fcheck.lib")
#pragma comment(lib, "errh.lib")
#pragma comment(lib, "d2bmploader.lib")
#pragma comment(lib, "mines.lib")

constexpr wchar_t bWinClassName[]{ L"MyNewMines" };
constexpr char tmp_file[]{ ".\\res\\data\\temp.dat" };
constexpr wchar_t Ltmp_file[]{ L".\\res\\data\\temp.dat" };
constexpr wchar_t record_file[]{ L".\\res\\data\\record.dat" };
constexpr wchar_t save_file[]{ L".\\res\\data\\save.dat" };
constexpr wchar_t help_file[]{ L".\\res\\data\\help.dat" };
constexpr wchar_t sound_file[]{ L".\\res\\snd\\main.wav" };

constexpr int mNew{ 1001 };
constexpr int mLvl{ 1002 };
constexpr int mExit{ 1003 };
constexpr int mSave{ 1004 };
constexpr int mLoad{ 1005 };
constexpr int mHoF{ 1006 };

constexpr int no_record{ 2001 };
constexpr int first_record{ 2002 };
constexpr int record{ 2003 };

WNDCLASS bWinClass{};
HINSTANCE bIns{ nullptr };
HWND bHwnd{ nullptr };
HICON mainIcon{ nullptr };
HCURSOR mainCursor{ nullptr };
HCURSOR outCursor{ nullptr };
HMENU bBar{ nullptr };
HMENU bMain{ nullptr };
HMENU bStore{ nullptr };
HDC PaintDC{ nullptr };
PAINTSTRUCT bPaint{};
MSG bMsg{};
BOOL bRet{ 0 };
UINT bTimer{ 0 };

POINT cur_pos{};

D2D1_RECT_F b1Rect{ 40.0f, 0, scr_width / 3.0f - 50.0f, 50.0f };
D2D1_RECT_F b2Rect{ scr_width / 3.0f + 40.0f, 0, scr_width * 2.0f / 3.0f - 50.0f, 50.0f };
D2D1_RECT_F b3Rect{ scr_width * 2.0f / 3.0f + 40.0f, 0, scr_width - 70.0f, 50.0f };

D2D1_RECT_F b1TxtRect{ 60.0f, 10.0f, scr_width / 3.0f - 50.0f, 50.0f };
D2D1_RECT_F b2TxtRect{ scr_width / 3.0f + 60.0f, 10.0f, scr_width * 2.0f / 3.0f - 50.0f, 50.0f };
D2D1_RECT_F b3TxtRect{ scr_width * 2.0f / 3.0f + 60.0f, 10.0f, scr_width - 70.0f, 50.0f };

bool pause{ false };
bool sound{ true };
bool in_client{ true };
bool show_help{ false };
bool b1Hglt{ false };
bool b2Hglt{ false };
bool b3Hglt{ false };
bool name_set{ false };

bool bomb_exploded{ false };
bool turn_the_game{ false };
bool level_skipped = false;

wchar_t current_player[16]{ L"TARLYO" };

int level = 1;
int score = 0;
int mins = 0;
int secs = 0;

int current_level_rows = 0;
int current_level_cols = 0;

float scale_x{ 0 };
float scale_y{ 0 };

ID2D1Factory* iFactory{ nullptr };
ID2D1HwndRenderTarget* Draw{ nullptr };

ID2D1RadialGradientBrush* b1BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b2BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b3BckgBrush{ nullptr };

ID2D1SolidColorBrush* statBrush{ nullptr };
ID2D1SolidColorBrush* fieldBrush{ nullptr };
ID2D1SolidColorBrush* txtBrush{ nullptr };
ID2D1SolidColorBrush* hgltBrush{ nullptr };
ID2D1SolidColorBrush* inactBrush{ nullptr };

ID2D1SolidColorBrush* N1Brush{ nullptr };
ID2D1SolidColorBrush* N2Brush{ nullptr };
ID2D1SolidColorBrush* N3Brush{ nullptr };
ID2D1SolidColorBrush* N4Brush{ nullptr };
ID2D1SolidColorBrush* N5Brush{ nullptr };
ID2D1SolidColorBrush* N6Brush{ nullptr };
ID2D1SolidColorBrush* N7Brush{ nullptr };
ID2D1SolidColorBrush* N8Brush{ nullptr };

IDWriteFactory* iWriteFactory{ nullptr };
IDWriteTextFormat* nrmFormat{ nullptr };
IDWriteTextFormat* midFormat{ nullptr };
IDWriteTextFormat* bigFormat{ nullptr };

ID2D1Bitmap* bmpLogo{ nullptr };
ID2D1Bitmap* bmpWin{ nullptr };
ID2D1Bitmap* bmpLoose{ nullptr };
ID2D1Bitmap* bmpRecord{ nullptr };

ID2D1Bitmap* bmpIntro[17]{ nullptr };
ID2D1Bitmap* bmpFlag[100]{ nullptr };
ID2D1Bitmap* bmpExplosion[24]{ nullptr };

////////////////////////////////////////////////////////////////

struct FLAG
{
	D2D1_RECT_F loc{};
	int frame{ 0 };
};

dll::GRID* Grid{ nullptr };
dll::RANDIT RandIt{};

struct TILEINFO
{
	D2D1_RECT_F dims{};
	int content{};
	int number{};
	bool active = false;
	bool suspicious = false;
	FLAG flag_info{};
};
std::vector<TILEINFO>vTiles;

D2D1_RECT_F Explosion{};

//////////////////////////////////////////////////////////////////

template<typename T>concept HasRelease = requires(T var)
{
	var.Release();
};
template<HasRelease T>bool FreeMem(T** var)
{
	if ((*var))
	{
		(*var)->Release();
		(*var) = nullptr;
		return true;
	}
	return false;
}
void LogErr(const wchar_t* what)
{
	std::wofstream err(L".\\res\\data\\error.log", std::ios::app);
	err << what << L" time of occurrence: " << std::chrono::system_clock::now() << std::endl;
	err.close();
}
void ClearResources()
{
	if (!(FreeMem(&iFactory)))LogErr(L"Error releasing D2D1 iFactory !");
	if (!(FreeMem(&Draw)))LogErr(L"Error releasing D2D1 HwndRenderTarget !");

	if (!(FreeMem(&b1BckgBrush)))LogErr(L"Error releasing D2D1 b1BckgBrush !");
	if (!(FreeMem(&b2BckgBrush)))LogErr(L"Error releasing D2D1 b1BckgBrush !"); 
	if (!(FreeMem(&b3BckgBrush)))LogErr(L"Error releasing D2D1 b1BckgBrush !");

	if (!(FreeMem(&statBrush)))LogErr(L"Error releasing D2D1 statBrush !");
	if (!(FreeMem(&fieldBrush)))LogErr(L"Error releasing D2D1 fieldBrush !");
	if (!(FreeMem(&txtBrush)))LogErr(L"Error releasing D2D1 txtBrush !");
	if (!(FreeMem(&hgltBrush)))LogErr(L"Error releasing D2D1 hltBrush !");
	if (!(FreeMem(&inactBrush)))LogErr(L"Error releasing D2D1 inactBrush !");

	if (!(FreeMem(&N1Brush)))LogErr(L"Error releasing D2D1 N1Brush !");
	if (!(FreeMem(&N2Brush)))LogErr(L"Error releasing D2D1 N2Brush !");
	if (!(FreeMem(&N3Brush)))LogErr(L"Error releasing D2D1 N3Brush !");
	if (!(FreeMem(&N4Brush)))LogErr(L"Error releasing D2D1 N4Brush !");
	if (!(FreeMem(&N5Brush)))LogErr(L"Error releasing D2D1 N5Brush !");
	if (!(FreeMem(&N6Brush)))LogErr(L"Error releasing D2D1 N6Brush !");
	if (!(FreeMem(&N7Brush)))LogErr(L"Error releasing D2D1 N7Brush !");
	if (!(FreeMem(&N8Brush)))LogErr(L"Error releasing D2D1 N8Brush !");

	if (!(FreeMem(&iWriteFactory)))LogErr(L"Error releasing D2D1 iWriteFactory !");
	if (!(FreeMem(&nrmFormat)))LogErr(L"Error releasing D2D1 nrmTextFormat !");
	if (!(FreeMem(&midFormat)))LogErr(L"Error releasing D2D1 midTextFormat !");
	if (!(FreeMem(&bigFormat)))LogErr(L"Error releasing D2D1 bigTextFormat !");

	if (!(FreeMem(&bmpLogo)))LogErr(L"Error releasing D2D1 bmpLogo !");
	if (!(FreeMem(&bmpWin)))LogErr(L"Error releasing D2D1 bmpWin !");
	if (!(FreeMem(&bmpLoose)))LogErr(L"Error releasing D2D1 bmpLoose !");
	if (!(FreeMem(&bmpRecord)))LogErr(L"Error releasing D2D1 bmpRecord !");

	for (int i = 0; i < 17; ++i)if (!(FreeMem(&bmpIntro[i])))LogErr(L"Error releasing D2D1 bmpIntro !");
	for (int i = 0; i < 100; ++i)if (!(FreeMem(&bmpFlag[i])))LogErr(L"Error releasing D2D1 bmpFlag !");
	for (int i = 0; i < 24; ++i)if (!(FreeMem(&bmpExplosion[i])))LogErr(L"Error releasing D2D1 bmpExplosion !");
}
void ErrExit(int what)
{
	MessageBeep(MB_ICONERROR);
	MessageBox(NULL, ErrHandle(what), L"Критична грешка !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

	ClearResources();
	std::remove(tmp_file);
	exit(1);
}
int IntroFrame()
{
	static int frame = 0;
	static int frame_delay = 5;

	--frame_delay;
	if (frame_delay <= 0)
	{
		frame_delay = 5;
		++frame;
		if (frame > 16)frame = 0;
	}
	return frame;
}
BOOL CheckRecord()
{
	if (score < 1)return no_record;

	int result{ 0 };
	CheckFile(record_file, &result);
	if (result == FILE_NOT_EXIST)
	{
		std::wofstream rec(record_file);
		rec << score << std::endl;
		for (int i = 0; i < 16; ++i)rec << static_cast<int>(current_player[i]) << std::endl;
		rec.close();
		return first_record;
	}
	else
	{
		std::wifstream check(record_file);
		check >> result;
		check.close();
	}

	if (result < score)
	{
		std::wofstream rec(record_file);
		rec << score << std::endl;
		for (int i = 0; i < 16; ++i)rec << static_cast<int>(current_player[i]) << std::endl;
		rec.close();
		return record;
	}

	return no_record;
}
void GameOver()
{
	KillTimer(bHwnd, bTimer);
	PlaySound(NULL, NULL, NULL);
	if (turn_the_game && !level_skipped)
	{
		int bonus_time = 200 + 60 * level;
		if (secs < bonus_time)score += bonus_time - secs;

		Draw->BeginDraw();
		Draw->DrawBitmap(bmpIntro[9], D2D1::RectF(0, 0, scr_width, scr_height));
		Draw->DrawTextW(L"ПРЕВЪРТЯ ИГРАТА !", 18, bigFormat, D2D1::RectF(100.0f, scr_height / 2.0f - 100.0f, scr_width, scr_height),
			txtBrush);
		Draw->EndDraw();
		if (sound)mciSendString(L"play .\\res\\snd\\levelup.wav", NULL, NULL, NULL);
		Sleep(2500);
		score += 1000;
	};

	switch (CheckRecord())
	{
	case no_record:
		Draw->BeginDraw();
		Draw->DrawBitmap(bmpLoose, D2D1::RectF(0, 0, scr_width, scr_height));
		Draw->EndDraw();
		if (sound)PlaySound(L".\\res\\snd\\loose.wav", NULL, SND_SYNC);
		else Sleep(3500);
		break;

	case first_record:
		Draw->BeginDraw();
		Draw->DrawBitmap(bmpWin, D2D1::RectF(0, 0, scr_width, scr_height));
		Draw->EndDraw();
		if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_SYNC);
		else Sleep(3500);
		break;

	case record:
		Draw->BeginDraw();
		Draw->DrawBitmap(bmpRecord, D2D1::RectF(0, 0, scr_width, scr_height));
		Draw->EndDraw();
		if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_SYNC);
		else Sleep(3500);
		break;
	}

	bMsg.message = WM_QUIT;
	bMsg.wParam = 0;
}
void InitGame()
{
	wcscpy_s(current_player, L"TARLYO");
	level = 1;
	score = 0;
	mins = 0;
	secs = 0;
	level_skipped = false;

	current_level_rows = 0;
	current_level_cols = 0;

	if(Grid)delete Grid;
	Grid = new dll::GRID(LEVEL1_ROWS, LEVEL1_COLS, 1);

	current_level_rows = LEVEL1_ROWS;
	current_level_cols = LEVEL1_COLS;

	vTiles.clear();
	for (int rows = 0; rows < LEVEL1_ROWS; ++rows)
	{
		for (int cols = 0; cols < LEVEL1_COLS; ++cols)
		{
			TILEINFO dummy{};
			FRECT temp{ Grid->GetTileDims(rows,cols) };
			dummy.dims.left = temp.left;
			dummy.dims.right = temp.right;
			dummy.dims.top = temp.up;
			dummy.dims.bottom = temp.down;
			dummy.number = rows * current_level_cols + cols;
			vTiles.push_back(dummy);
		}
	}
}
void LevelUp()
{
	if (!level_skipped)
	{
		int bonus_time = 200 + 60 * level;
		if (secs < bonus_time)score += bonus_time - secs;

		Draw->BeginDraw();
		Draw->DrawBitmap(bmpIntro[9], D2D1::RectF(0, 0, scr_width, scr_height));
		Draw->DrawTextW(L"НИВОТО ИЗЧИСТЕНО !", 19, bigFormat, D2D1::RectF(100.0f, scr_height / 2.0f - 100.0f, scr_width, scr_height),
			txtBrush);
		Draw->EndDraw();
		if (sound)mciSendString(L"play .\\res\\snd\\levelup.wav", NULL, NULL, NULL);
		Sleep(4000);
	};
	level_skipped = false;

	mins = 0;
	secs = 0;

	++level;

	current_level_rows = 0;
	current_level_cols = 0;

	delete Grid;
	Grid = nullptr;

	switch (level)
	{
	case 2:
		Grid = new dll::GRID(LEVEL2_ROWS, LEVEL2_COLS, 2);
		current_level_rows = LEVEL2_ROWS;
		current_level_cols = LEVEL2_COLS;
		break;

	case 3:
		Grid = new dll::GRID(LEVEL3_ROWS, LEVEL3_COLS, 3);
		current_level_rows = LEVEL3_ROWS;
		current_level_cols = LEVEL3_COLS;
		break;

	case 4:
		Grid = new dll::GRID(LEVEL4_ROWS, LEVEL4_COLS, 4);
		current_level_rows = LEVEL4_ROWS;
		current_level_cols = LEVEL4_COLS;
		break;

	default: turn_the_game = true;
	}

	if (turn_the_game)GameOver();

	vTiles.clear();
	if(Grid)
		for (int rows = 0; rows < current_level_rows; ++rows)
		{
			for (int cols = 0; cols < current_level_cols; ++cols)
			{
				TILEINFO dummy{};
				FRECT temp{ Grid->GetTileDims(rows,cols) };
				dummy.dims.left = temp.left;
				dummy.dims.right = temp.right;
				dummy.dims.top = temp.up;
				dummy.dims.bottom = temp.down;
				dummy.number = rows * current_level_cols + cols;
				vTiles.push_back(dummy);
			}
		}
}
void HallOfFame()
{
	int result = 0;
	CheckFile(record_file, &result);
	if (result == FILE_NOT_EXIST)
	{
		if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
		MessageBox(bHwnd, L"Липсва рекорд на играта !\n\nПостарай се повече !", L"Липсва файл",
			MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
		return;
	}

	wchar_t rec_txt[100]{ L"НАЙ-ВЕЛИК САПЬОР: " };
	wchar_t saved_player[16]{ L"\0" };
	wchar_t saved_score[3]{ L"\0" };

	std::wifstream rec(record_file);
	rec >> result;
	wsprintf(saved_score, L"%d", result);
	for (int i = 0; i < 16; ++i)
	{
		int letter = 0;
		rec >> letter;
		saved_player[i] = static_cast<wchar_t>(letter);
	}
	rec.close();

	wcscat_s(rec_txt, saved_player);
	wcscat_s(rec_txt, L"\n\nСВЕТОВЕН РЕКОРД: ");
	wcscat_s(rec_txt, saved_score);

	result = 0;

	for (int i = 0; i < 100; ++i)
	{
		if (rec_txt[i] != '\0')++result;
		else break;
	}

	Draw->BeginDraw();
	Draw->Clear(D2D1::ColorF(D2D1::ColorF::Azure));
	if (midFormat && txtBrush)
		Draw->DrawTextW(rec_txt, result, midFormat, D2D1::RectF(150.0f, 80.0f, scr_width, scr_height), txtBrush);
	Draw->EndDraw();

	if (sound)mciSendString(L"play .\\res\\snd\\show_rec.wav", NULL, NULL, NULL);
	Sleep(4000);
}
void ShowHelp()
{
	int result{ 0 };
	CheckFile(help_file, &result);

	if (result == FILE_NOT_EXIST)
	{
		if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
		MessageBox(bHwnd, L"Липсва помощна информация за играта !\n\nСвържете се с разработчика !", L"Липсва файл",
			MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
		return;
	}

	wchar_t rec_txt[1000]{ L"\0" };
	
	std::wifstream help(help_file);
	help >> result;
	
	for (int i = 0; i < result; ++i)
	{
		int letter = 0;
		help >> letter;
		rec_txt[i] = static_cast<wchar_t>(letter);
	}
	help.close();

	Draw->BeginDraw();
	Draw->Clear(D2D1::ColorF(D2D1::ColorF::Azure));
	if (statBrush && b1BckgBrush && b2BckgBrush && b3BckgBrush && fieldBrush && txtBrush && hgltBrush && inactBrush
		&& nrmFormat)
	{
		Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), statBrush);
		Draw->FillRectangle(D2D1::RectF(0, 50.0f, scr_width, scr_height), fieldBrush);
		Draw->FillRoundedRectangle(D2D1::RoundedRect(b1Rect, 20.0f, 25.0f), b1BckgBrush);
		Draw->FillRoundedRectangle(D2D1::RoundedRect(b2Rect, 20.0f, 25.0f), b2BckgBrush);
		Draw->FillRoundedRectangle(D2D1::RoundedRect(b3Rect, 20.0f, 25.0f), b3BckgBrush);

		if (name_set)Draw->DrawTextW(L"ИМЕ НА САПЬОР", 14, nrmFormat, b1TxtRect, inactBrush);
		else
		{
			if (!b1Hglt)Draw->DrawTextW(L"ИМЕ НА САПЬОР", 14, nrmFormat, b1TxtRect, txtBrush);
			else Draw->DrawTextW(L"ИМЕ НА САПЬОР", 14, nrmFormat, b1TxtRect, hgltBrush);
		}
		if (!b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmFormat, b2TxtRect, txtBrush);
		else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmFormat, b2TxtRect, hgltBrush);
		if (!b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmFormat, b3TxtRect, txtBrush);
		else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmFormat, b3TxtRect, hgltBrush);
	}
	if (midFormat && txtBrush)
		Draw->DrawTextW(rec_txt, result, midFormat, D2D1::RectF(20.0f, 80.0f, scr_width, scr_height), txtBrush);
	Draw->EndDraw();

	if (sound)mciSendString(L"play .\\res\\snd\\help.wav", NULL, NULL, NULL);
}
void SaveGame()
{
	int result = 0;
	CheckFile(save_file, &result);
	if (result == FILE_EXIST)
	{
		if (sound)mciSendString(L"play .\\res\\data\\exclamation.wav", NULL, NULL, NULL);
		if (MessageBox(bHwnd, L"Има предишна записана игра !\n\nНаистина ли я презаписваш ?",
			L"Презапис !", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)return;
	}

	std::wofstream save(save_file);

	save << level << std::endl;
	save << score << std::endl;
	save << mins << std::endl;
	save << secs << std::endl;

	save << level_skipped << std::endl;
	save << bomb_exploded << std::endl;
	save << turn_the_game << std::endl;
	save << name_set << std::endl;

	for (int i = 0; i < 16; ++i)save << static_cast<int>(current_player[i]) << std::endl;

	for (int row = 0; row < current_level_rows; ++row)
	{
		for (int col = 0; col < current_level_cols; ++col)
		{
			save << Grid->ShowTileInfo(row, col) << std::endl;
			save << Grid->IsTileSelected(row, col) << std::endl;
		}
	}

	save.close();

	if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);

	MessageBox(bHwnd, L"Играта е запазена !", L"Запис !", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
}
void LoadGame()
{
	int result = 0;
	CheckFile(save_file, &result);
	if (result == FILE_EXIST)
	{
		if (sound)mciSendString(L"play .\\res\\data\\exclamation.wav", NULL, NULL, NULL);
		if (MessageBox(bHwnd, L"Настоящата игра ще бъде загубена !\n\nНаистина ли я презаписваш ?",
			L"Презапис !", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)return;
	}
	else
	{
		if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
		MessageBox(bHwnd, L"Липсва записана игра !\n\nПостарай се повече !", L"Липсва файл",
			MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
		return;
	}

	delete Grid;
	Grid = nullptr;
	vTiles.clear();

	std::wifstream save(save_file);

	save >> level;

	switch (level)
	{
	case 1:
		Grid = new dll::GRID(LEVEL1_ROWS, LEVEL1_COLS, 1);
		current_level_rows = LEVEL1_ROWS;
		current_level_cols = LEVEL1_COLS;
		break;

	case 2:
		Grid = new dll::GRID(LEVEL2_ROWS, LEVEL2_COLS, 2);
		current_level_rows = LEVEL2_ROWS;
		current_level_cols = LEVEL2_COLS;
		break;

	case 3:
		Grid = new dll::GRID(LEVEL3_ROWS, LEVEL3_COLS, 3);
		current_level_rows = LEVEL3_ROWS;
		current_level_cols = LEVEL3_COLS;
		break;

	case 4:
		Grid = new dll::GRID(LEVEL4_ROWS, LEVEL4_COLS, 4);
		current_level_rows = LEVEL4_ROWS;
		current_level_cols = LEVEL4_COLS;
		break;

	default: turn_the_game = true;
	}

	save >> score;
	save >> mins;
	save >> secs;

	save >> level_skipped;
	save >> bomb_exploded;
	save >> turn_the_game;
	
	if (turn_the_game || bomb_exploded)GameOver();
	
	save >> name_set;

	for (int i = 0; i < 16; ++i)
	{
		int letter = 0;
		save >> letter;
		current_player[i] = static_cast<wchar_t>(letter);
	}

	for (int row = 0; row < current_level_rows; ++row)
	{
		for (int col = 0; col < current_level_cols; ++col)
		{
			int tcontent = 0;
			bool tactive = 0;

			save >> tcontent;
			save >> tactive;

			Grid->SetTileInfo(row, col, tcontent, tactive);
		}
	}

	save.close();

	if (Grid)
		for (int rows = 0; rows < current_level_rows; ++rows)
		{
			for (int cols = 0; cols < current_level_cols; ++cols)
			{
				TILEINFO dummy{};
				FRECT temp{ Grid->GetTileDims(rows,cols) };
				dummy.dims.left = temp.left;
				dummy.dims.right = temp.right;
				dummy.dims.top = temp.up;
				dummy.dims.bottom = temp.down;
				dummy.number = rows * current_level_cols + cols;
				vTiles.push_back(dummy);
			}
		}

	if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);

	MessageBox(bHwnd, L"Играта е заредена !", L"Зареждане !", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
	switch (ReceivedMsg)
	{
	case WM_INITDIALOG:
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(mainIcon));
		return true;

	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;

		case IDOK:
			if (GetDlgItemText(hwnd, IDC_NAME, current_player, 16) < 1)
			{
				if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
				MessageBox(bHwnd, L"Ха, ха, ха ! Забрави си името !", L"Забраватор !", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
				wcscpy_s(current_player, L"TARLYO");
				EndDialog(hwnd, IDCANCEL);
			}
			EndDialog(hwnd, IDOK);
		}
		break;
	}

	return (INT_PTR)(FALSE);
}
LRESULT CALLBACK WinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
	switch (ReceivedMsg)
	{
	case WM_CREATE:
		if (bIns)
		{
			SetTimer(hwnd, bTimer, 1000, 0);

			bBar = CreateMenu();
			bMain = CreateMenu();
			bStore = CreateMenu();

			AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
			AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bStore), L"Меню за данни");

			AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
			AppendMenu(bMain, MF_STRING, mLvl, L"Следващо ниво");
			AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
			AppendMenu(bMain, MF_STRING, mExit, L"Изход");

			AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
			AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
			AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
			AppendMenu(bStore, MF_STRING, mHoF, L"Зала на славата");

			SetMenu(hwnd, bBar);
			InitGame();

		}
		break;

	case WM_CLOSE:
		pause = true;
		if (sound)mciSendString(L"play .\\res\\data\\exclamation.wav", NULL, NULL, NULL);
		if (MessageBox(hwnd, L"Ако излезеш, бомбата ще избухне !\n\nНаистина ли излизаш ?",
			L"Изход !", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
		{
			pause = false;
			break;
		}
		GameOver();
		break;

	case WM_PAINT:
		PaintDC = BeginPaint(hwnd, &bPaint);
		FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(10, 10, 10)));
		EndPaint(hwnd, &bPaint);
		break;

	case WM_TIMER:
		if (pause)break;
		++secs;
		mins = secs / 60;
		break;

	case WM_SETCURSOR:
		GetCursorPos(&cur_pos);
		ScreenToClient(hwnd, &cur_pos);
		if (LOWORD(lParam) == HTCLIENT)
		{
			if (!in_client)
			{
				in_client = true;
				pause = false;
			}

			if (cur_pos.y * scale_y <= 50.0f)
			{
				if (cur_pos.x * scale_x >= b1Rect.left && cur_pos.x * scale_x <= b1Rect.right)
				{
					if (!b1Hglt)
					{
						if (sound)mciSendString(L"play .\\res\\data\\click.wav", NULL, NULL, NULL);
						b1Hglt = true;
						b2Hglt = false;
						b3Hglt = false;
					}
				}
				else if (cur_pos.x * scale_x >= b2Rect.left && cur_pos.x * scale_x <= b2Rect.right)
				{
					if (!b2Hglt)
					{
						if (sound)mciSendString(L"play .\\res\\data\\click.wav", NULL, NULL, NULL);
						b1Hglt = false;
						b2Hglt = true;
						b3Hglt = false;
					}
				}
				else if (cur_pos.x * scale_x >= b3Rect.left && cur_pos.x * scale_x <= b3Rect.right)
				{
					if (!b3Hglt)
					{
						if (sound)mciSendString(L"play .\\res\\data\\click.wav", NULL, NULL, NULL);
						b1Hglt = false;
						b2Hglt = false;
						b3Hglt = true;
					}
				}
				else if (b1Hglt || b2Hglt || b3Hglt)
				{
					if (sound)mciSendString(L"play .\\res\\data\\click.wav", NULL, NULL, NULL);
					b1Hglt = false;
					b2Hglt = false;
					b3Hglt = false;
				}

				SetCursor(outCursor);

				return true;
			}
			else if (b1Hglt || b2Hglt || b3Hglt)
			{
				if (sound)mciSendString(L"play .\\res\\data\\click.wav", NULL, NULL, NULL);
				b1Hglt = false;
				b2Hglt = false;
				b3Hglt = false;
			}
			
			SetCursor(mainCursor);

			return true;
		}
		else
		{
			if (in_client)
			{
				in_client = false;
				pause = true;
			}

			if (b1Hglt || b2Hglt || b3Hglt)
			{
				if (sound)mciSendString(L"play .\\res\\data\\click.wav", NULL, NULL, NULL);
				b1Hglt = false;
				b2Hglt = false;
				b3Hglt = false;
			}

			SetCursor(LoadCursor(NULL,IDC_ARROW));

			return true;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case mNew:
			pause = true;
			if (sound)mciSendString(L"play .\\res\\data\\exclamation.wav", NULL, NULL, NULL);
			if (MessageBox(hwnd, L"Ако рестартираш, губиш точките до момента !\n\nНаистина ли рестартираш ?",
				L"Рестарт !", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
			{
				pause = false;
				break;
			}
			InitGame();
			break;

		case mLvl:
			pause = true;
			if (sound)mciSendString(L"play .\\res\\data\\exclamation.wav", NULL, NULL, NULL);
			if (MessageBox(hwnd, L"Ако прескочиш нивото, губиш бонуса за него !\n\nНаистина ли прескачаш ниво ?",
				L"Следващо ниво !", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
			{
				pause = false;
				break;
			}
			level_skipped = true;
			LevelUp();
			break;

		case mExit:
			SendMessage(hwnd, WM_CLOSE, NULL, NULL);
			break;

		case mSave:
			pause = true;
			SaveGame();
			pause = false;
			break;

		case mLoad:
			pause = true;
			LoadGame();
			pause = false;
			break;

		case mHoF:
			pause = true;
			HallOfFame();
			pause = false;
			break;
		}
		break;

	case WM_LBUTTONDOWN:
		if (HIWORD(lParam) * scale_y <= 50)
		{
			if (LOWORD(lParam) * scale_x >= b1Rect.left && LOWORD(lParam) * scale_x <= b1Rect.right)
			{
				if (name_set)
				{
					if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
					break;
				}
				if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
				if (DialogBox(bIns, MAKEINTRESOURCE(IDD_PLAYER), hwnd, &DlgProc) == IDOK)name_set = true;
				break;
			}
			if (LOWORD(lParam) * scale_x >= b2Rect.left && LOWORD(lParam) * scale_x <= b2Rect.right)
			{
				mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
				if (sound)
				{
					PlaySound(NULL, NULL, NULL);
					sound = false;
					break;
				}
				else
				{
					PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);
					sound = true;
					break;
				}
			}
			if (LOWORD(lParam) * scale_x >= b3Rect.left && LOWORD(lParam) * scale_x <= b3Rect.right)
			{
				if(sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
				if (!show_help)
				{
					show_help = true;
					pause = true;
					ShowHelp();
					break;
				}
				else
				{
					show_help = false;
					pause = false;
					break;
				}
			}

			break;
		}
		if (bomb_exploded)break;
		if (Grid && !vTiles.empty())
		{
			float tx = LOWORD(lParam) * scale_x;
			float ty = HIWORD(lParam) * scale_y;

			for (int rows = 0; rows < current_level_rows; ++rows)
			{
				bool found = false;
				
				for (int cols = 0; cols < current_level_cols; ++cols)
				{
					FRECT dummy = Grid->GetTileDims(rows, cols);

					if (tx >= dummy.left && tx <= dummy.right && ty >= dummy.up && ty <= dummy.down)
					{
						int tile_content = Grid->SelectTile(rows, cols);

						if (tile_content == MINE)
						{
							bomb_exploded = true;
							Explosion.left = dummy.left;
							Explosion.right = dummy.right;
							Explosion.top = dummy.up;
							Explosion.bottom = dummy.down;
							if (sound)mciSendString(L"play .\\res\\snd\\explosion.wav", NULL, NULL, NULL);
						}

						for (int count = 0; count < vTiles.size(); ++count)
						{
							if (vTiles[count].dims.left == dummy.left && vTiles[count].dims.right == dummy.right
								&& vTiles[count].dims.top == dummy.up && vTiles[count].dims.bottom == dummy.down)
							{
								vTiles[count].content = tile_content;
								vTiles[count].active = Grid->IsTileSelected(rows, cols);
								break;
							}
						}
						
						found = true;
					}

					if (found)break;
				}
				
				if (found)break;
			}
		}
		break;

	case WM_RBUTTONDOWN:
		if (bomb_exploded)break;
		if (Grid && !vTiles.empty())
		{
			float tx = LOWORD(lParam) * scale_x;
			float ty = HIWORD(lParam) * scale_y;

			for (int rows = 0; rows < current_level_rows; ++rows)
			{
				bool found = false;

				for (int cols = 0; cols < current_level_cols; ++cols)
				{
					FRECT dummy = Grid->GetTileDims(rows, cols);

					if (tx >= dummy.left && tx <= dummy.right && ty >= dummy.up && ty <= dummy.down)
					{	
						for (int count = 0; count < vTiles.size(); ++count)
						{
							if (vTiles[count].dims.left == dummy.left && vTiles[count].dims.right == dummy.right
								&& vTiles[count].dims.top == dummy.up && vTiles[count].dims.bottom == dummy.down)
							{
								if (vTiles[count].active)break;
								if (!vTiles[count].suspicious)Grid->MineMarked(rows, cols, true);
								else Grid->MineMarked(rows, cols, false);
								if (!vTiles[count].suspicious)
								{
									vTiles[count].flag_info.loc.left = dummy.left + 15.0f;
									vTiles[count].flag_info.loc.right = dummy.right;
									vTiles[count].flag_info.loc.top = dummy.up + 10.0f;
									vTiles[count].flag_info.loc.bottom = dummy.down;
									vTiles[count].flag_info.frame = 0;
								}
								vTiles[count].suspicious = !vTiles[count].suspicious;
								break;
							}
						}

						found = true;
					}

					if (found)break;
				}

				if (found)break;
			}
		}
		break;

	default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
	}

	return (LRESULT)(FALSE);
}

void CreateResources()
{
	int win_x = (int)(GetSystemMetrics(SM_CXSCREEN) / 2 - (int)(scr_width / 2.0f));
	int win_y = 100;

	if (GetSystemMetrics(SM_CXSCREEN) < win_x + (int)(scr_width) || GetSystemMetrics(SM_CYSCREEN) < win_y + (int)(scr_height))
		ErrExit(eScreen);

	mainIcon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 255, 255, LR_LOADFROMFILE));
	if (!mainIcon)ErrExit(eIcon);
	mainCursor = LoadCursorFromFileW(L".\\res\\main.ani");
	outCursor = LoadCursorFromFileW(L".\\res\\out.ani");
	if (!mainCursor || !outCursor)ErrExit(eCursor);

	bWinClass.lpszClassName = bWinClassName;
	bWinClass.hInstance = bIns;
	bWinClass.lpfnWndProc = &WinProc;
	bWinClass.hbrBackground = CreateSolidBrush(RGB(10, 10, 10));
	bWinClass.hIcon = mainIcon;
	bWinClass.hCursor = mainCursor;
	bWinClass.style = CS_DROPSHADOW;

	if (!RegisterClass(&bWinClass))ErrExit(eClass);

	bHwnd = CreateWindow(bWinClassName, L"БЕСНИЯТ САПЬОР", WS_CAPTION | WS_SYSMENU, win_x, win_y, (int)(scr_width),
		(int)(scr_height), NULL, NULL, bIns, NULL);
	if (!bHwnd)ErrExit(eWindow);
	else
	{
		ShowWindow(bHwnd, SW_SHOWDEFAULT);

		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
		if (hr != S_OK)
		{
			LogErr(L"Error creating D2D1 Factory !");
			ErrExit(eD2D);
		}

		if (iFactory)
		{
			hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
				D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
			if (hr != S_OK)
			{
				LogErr(L"Error creating D2D1 HWNDRenderTarget !");
				ErrExit(eD2D);
			}
		}
		if (Draw)
		{
			RECT DPIRect{};
			D2D1_SIZE_F DIPSize{ Draw->GetSize() };

			GetClientRect(bHwnd, &DPIRect);

			scale_x = DIPSize.width / (DPIRect.right - DPIRect.left);
			scale_y = DIPSize.height / (DPIRect.bottom - DPIRect.top);

			D2D1_GRADIENT_STOP gStops[2]{};
			ID2D1GradientStopCollection* gColl{ nullptr };

			gStops[0].position = 0;
			gStops[0].color = D2D1::ColorF(D2D1::ColorF::OrangeRed);
			gStops[1].position = 1.0f;
			gStops[1].color = D2D1::ColorF(D2D1::ColorF::Orange);

			hr = Draw->CreateGradientStopCollection(gStops, 2, &gColl);
			if (hr != S_OK)
			{
				LogErr(L"Error creating D2D1 GradientStopCollection !");
				ErrExit(eD2D);
			}
			if (gColl)
			{
				hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b1Rect.left +
					(b1Rect.right - b1Rect.left) / 2.0f), D2D1::Point2F(0, 0), (b1Rect.right - b1Rect.left) / 2.0f, 25.0f),
					gColl, &b1BckgBrush);
				hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b2Rect.left +
					(b2Rect.right - b2Rect.left) / 2.0f), D2D1::Point2F(0, 0), (b2Rect.right - b2Rect.left) / 2.0f, 25.0f),
					gColl, &b2BckgBrush);
				hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b3Rect.left +
					(b3Rect.right - b3Rect.left) / 2.0f), D2D1::Point2F(0, 0), (b3Rect.right - b3Rect.left) / 2.0f, 25.0f),
					gColl, &b3BckgBrush);
				if (hr != S_OK)
				{
					LogErr(L"Error creating D2D1 RadialGradientBrushes !");
					ErrExit(eD2D);
				}

				FreeMem(&gColl);
			}


			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Maroon), &statBrush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSlateGray), &fieldBrush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Navy), &txtBrush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Cyan), &hgltBrush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DodgerBlue), &inactBrush);

			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &N1Brush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gold), &N2Brush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::ForestGreen), &N3Brush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Teal), &N4Brush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SpringGreen), &N5Brush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Cyan), &N6Brush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DeepSkyBlue), &N7Brush);
			hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSeaGreen), &N8Brush);

			if (hr != S_OK)
			{
				LogErr(L"Error creating D2D1 SolidColorBrushes !");
				ErrExit(eD2D);
			}

			bmpLogo = Load(L".\\res\\img\\logo.png", Draw);
			if (!bmpLogo)
			{
				LogErr(L"Error loading bmpLogo !");
				ErrExit(eD2D);
			}
			bmpLoose = Load(L".\\res\\img\\game_over.png", Draw);
			if (!bmpLoose)
			{
				LogErr(L"Error loading bmpLoose !");
				ErrExit(eD2D);
			}
			bmpWin = Load(L".\\res\\img\\game_win.png", Draw);
			if (!bmpWin)
			{
				LogErr(L"Error loading bmpWin !");
				ErrExit(eD2D);
			}
			bmpRecord = Load(L".\\res\\img\\game_record.png", Draw);
			if (!bmpRecord)
			{
				LogErr(L"Error loading bmpRecord !");
				ErrExit(eD2D);
			}

			for (int i = 0; i < 17; ++i)
			{
				wchar_t name[50]{ L".\\res\\img\\intro\\" };
				wchar_t add[5]{ L"\0" };

				wsprintf(add, L"%d", i);
				wcscat_s(name, add);
				wcscat_s(name, L".png");

				bmpIntro[i] = Load(name, Draw);

				if (!bmpIntro[i])
				{
					LogErr(L"Error loading bmpIntro !");
					ErrExit(eD2D);
				}
			}
			for (int i = 0; i < 100; ++i)
			{
				wchar_t name[50]{ L".\\res\\img\\flag\\" };
				wchar_t add[5]{ L"\0" };

				wsprintf(add, L"%d", i);
				wcscat_s(name, add);
				wcscat_s(name, L".png");

				bmpFlag[i] = Load(name, Draw);

				if (!bmpFlag[i])
				{
					LogErr(L"Error loading bmpFlag !");
					ErrExit(eD2D);
				}
			}
			for (int i = 0; i < 24; ++i)
			{
				wchar_t name[50]{ L".\\res\\img\\explosion\\" };
				wchar_t add[5]{ L"\0" };

				wsprintf(add, L"%d", i);
				wcscat_s(name, add);
				wcscat_s(name, L".png");

				bmpExplosion[i] = Load(name, Draw);

				if (!bmpExplosion[i])
				{
					LogErr(L"Error loading bmpExplosion !");
					ErrExit(eD2D);
				}
			}
		}

		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&iWriteFactory));
		if (hr != S_OK)
		{
			LogErr(L"Error creating D2D1 WriteFactory !");
			ErrExit(eD2D);
		}
		if (iWriteFactory)
		{
			hr = iWriteFactory->CreateTextFormat(L"Cascadia Code", NULL, DWRITE_FONT_WEIGHT_EXTRA_BOLD,
				DWRITE_FONT_STYLE_ITALIC, DWRITE_FONT_STRETCH_NORMAL, 16.0f, L"", &nrmFormat);
			hr = iWriteFactory->CreateTextFormat(L"Cascadia Code", NULL, DWRITE_FONT_WEIGHT_EXTRA_BOLD,
				DWRITE_FONT_STYLE_ITALIC, DWRITE_FONT_STRETCH_NORMAL, 24.0f, L"", &midFormat);
			hr = iWriteFactory->CreateTextFormat(L"Cascadia Code", NULL, DWRITE_FONT_WEIGHT_EXTRA_BOLD,
				DWRITE_FONT_STYLE_ITALIC, DWRITE_FONT_STRETCH_NORMAL, 72.0f, L"", &bigFormat);
			if (hr != S_OK)
			{
				LogErr(L"Error creating D2D1 WriteFactory TextFormats !");
				ErrExit(eD2D);
			}
		}
	}

	if (Draw)
	{
		mciSendString(L"play .\\res\\snd\\intro.wav", NULL, NULL, NULL);

		for (int i = 0; i < 100; ++i)
		{
			Draw->BeginDraw();
			Draw->Clear(D2D1::ColorF(D2D1::ColorF::LightSlateGray));
			Draw->DrawBitmap(bmpIntro[IntroFrame()], D2D1::RectF(0, 0, scr_width, scr_height));
			Draw->DrawBitmap(bmpLogo, D2D1::RectF(0, 0, scr_width, scr_height));
			Draw->EndDraw();
		}
	}

	Sleep(2500);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	_wsetlocale(LC_ALL, L"");
	bIns = hInstance;
	if (!bIns)
	{
		LogErr(L"Error obtaining hInstance from Windows !");
		ErrExit(eClass);
	}
	else
	{
		int result{ 0 };
		CheckFile(Ltmp_file, &result);
		if (result == FILE_EXIST)ErrExit(eStarted);
		else
		{
			std::wofstream start(Ltmp_file);
			start << L"Game started at: " << std::chrono::system_clock::now();
			start.close();
		}
	}

	CreateResources();

	PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);

	while (bMsg.message != WM_QUIT)
	{
		if ((bRet = PeekMessage(&bMsg, nullptr, NULL, NULL, PM_REMOVE)) != 0)
		{
			if (bRet == -1)ErrExit(eMsg);

			TranslateMessage(&bMsg);
			DispatchMessage(&bMsg);
		}

		if (pause)
		{
			if (show_help)continue;
			Draw->BeginDraw();
			Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkSlateGray));
			Draw->DrawBitmap(bmpIntro[IntroFrame()], D2D1::RectF(0, 0, scr_width, scr_height));
			if (txtBrush && bigFormat)Draw->DrawTextW(L"ПАУЗА", 6, bigFormat,
				D2D1::RectF(scr_width / 2.0f - 100.0f, scr_height / 2.0f - 50.0f, scr_width, scr_height), txtBrush);
			Draw->EndDraw();
			continue;
		}
	/////////////////////////////////////////////////////////////





	// DRAW THINGS *********************************************

		Draw->BeginDraw();
		if (statBrush && b1BckgBrush && b2BckgBrush && b3BckgBrush && fieldBrush && txtBrush && hgltBrush && inactBrush
			&& nrmFormat)
		{
			Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), statBrush);
			Draw->FillRectangle(D2D1::RectF(0, 50.0f, scr_width, scr_height), fieldBrush);
			Draw->FillRoundedRectangle(D2D1::RoundedRect(b1Rect, 20.0f, 25.0f), b1BckgBrush);
			Draw->FillRoundedRectangle(D2D1::RoundedRect(b2Rect, 20.0f, 25.0f), b2BckgBrush);
			Draw->FillRoundedRectangle(D2D1::RoundedRect(b3Rect, 20.0f, 25.0f), b3BckgBrush);

			if (name_set)Draw->DrawTextW(L"ИМЕ НА САПЬОР", 14, nrmFormat, b1TxtRect, inactBrush);
			else
			{
				if(!b1Hglt)Draw->DrawTextW(L"ИМЕ НА САПЬОР", 14, nrmFormat, b1TxtRect, txtBrush);
				else Draw->DrawTextW(L"ИМЕ НА САПЬОР", 14, nrmFormat, b1TxtRect, hgltBrush);
			}
			if (!b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmFormat, b2TxtRect, txtBrush);
			else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmFormat, b2TxtRect, hgltBrush);
			if (!b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmFormat, b3TxtRect, txtBrush);
			else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmFormat, b3TxtRect, hgltBrush);
		}
		
		if (!vTiles.empty() && statBrush &&midFormat)
		{
			for (int i = 0; i < vTiles.size(); ++i)
			{
				Draw->DrawRectangle(vTiles[i].dims, statBrush, 2.0f);
				
				if (!vTiles[i].active && !vTiles[i].suspicious)continue;
				else if (vTiles[i].suspicious)
				{
					Draw->DrawBitmap(bmpFlag[vTiles[i].flag_info.frame], vTiles[i].flag_info.loc);
					++vTiles[i].flag_info.frame;
					if (vTiles[i].flag_info.frame > 99)vTiles[i].flag_info.frame = 0;
					continue;
				}

				switch (vTiles[i].content)
				{
				
				case 0:
					Draw->FillRectangle(D2D1::RectF(vTiles[i].dims.left + 15.0f, vTiles[i].dims.top + 10.0f, 
						vTiles[i].dims.right - 10.0f, vTiles[i].dims.bottom - 10.0f), N8Brush);
					break;

				case 1:
					Draw->DrawTextW(L"1", 1, midFormat, D2D1::RectF(vTiles[i].dims.left + 15.0f, vTiles[i].dims.top + 10.0f, 
						vTiles[i].dims.right, vTiles[i].dims.bottom), N1Brush);
					break;
				
				case 2:
					Draw->DrawTextW(L"2", 1, midFormat, D2D1::RectF(vTiles[i].dims.left + 15.0f, vTiles[i].dims.top + 10.0f, 
						vTiles[i].dims.right, vTiles[i].dims.bottom), N2Brush);
					break;

				case 3:
					Draw->DrawTextW(L"3", 1, midFormat, D2D1::RectF(vTiles[i].dims.left + 15.0f, vTiles[i].dims.top + 10.0f, 
						vTiles[i].dims.right, vTiles[i].dims.bottom), N3Brush);
					break;

				case 4:
					Draw->DrawTextW(L"4", 1, midFormat, D2D1::RectF(vTiles[i].dims.left + 15.0f, vTiles[i].dims.top + 10.0f, 
						vTiles[i].dims.right, vTiles[i].dims.bottom), N4Brush);
					break;

				case 5:
					Draw->DrawTextW(L"5", 1, midFormat, D2D1::RectF(vTiles[i].dims.left + 15.0f, vTiles[i].dims.top + 10.0f, 
						vTiles[i].dims.right, vTiles[i].dims.bottom), N5Brush);
					break;

				case 6:
					Draw->DrawTextW(L"1", 1, midFormat, D2D1::RectF(vTiles[i].dims.left + 15.0f, vTiles[i].dims.top + 10.0f, 
						vTiles[i].dims.right, vTiles[i].dims.bottom), N6Brush);
					break;
				
				case 7:
					Draw->DrawTextW(L"1", 1, midFormat, D2D1::RectF(vTiles[i].dims.left + 15.0f, vTiles[i].dims.top + 10.0f, 
						vTiles[i].dims.right, vTiles[i].dims.bottom), N7Brush);
					break;

				case 8:
					Draw->DrawTextW(L"1", 1, midFormat, D2D1::RectF(vTiles[i].dims.left + 15.0f, vTiles[i].dims.top + 10.0f, 
						vTiles[i].dims.right, vTiles[i].dims.bottom), N8Brush);
					break;
				
				
				
				default:
					break;
				}
			}
		}

	////////////////////////////////////////////////////////////

		if (bomb_exploded)
		{
			static int explosion_frame = 0;
			static int explosion_delay = 3;
			if (explosion_frame <= 23)Draw->DrawBitmap(bmpExplosion[explosion_frame], Explosion);
			--explosion_delay;
			if (explosion_delay <= 0)
			{
				explosion_delay = 3;
				++explosion_frame;

				if (explosion_frame > 23)
				{
					Draw->EndDraw();
					GameOver();
				}
			}
		}

		if (Grid)
		{
			if (Grid->MinesRemaining() == 0)
			{
				Draw->EndDraw();
				LevelUp();
			}
		}

		if (txtBrush && midFormat)
		{
			wchar_t stat_txt[150]{ L"сапьор: " };
			wchar_t add[5]{ L"\0" };
			int stat_size = 0;
		
			wcscat_s(stat_txt, current_player);

			wcscat_s(stat_txt, L", ниво: ");
			wsprintf(add, L"%d", level);
			wcscat_s(stat_txt, add);

			wcscat_s(stat_txt, L", резултат: ");
			wsprintf(add, L"%d", score);
			wcscat_s(stat_txt, add);

			wcscat_s(stat_txt, L", време: ");
			if (mins < 10)wcscat_s(stat_txt, L"0");
			wsprintf(add, L"%d", mins);
			wcscat_s(stat_txt, add);
			if (mins < 10)wcscat_s(stat_txt, L" : ");
			if (secs - mins * 60 < 10)wcscat_s(stat_txt, L"0");
			wsprintf(add, L"%d", secs - mins * 60);
			wcscat_s(stat_txt, add);

			for (int i = 0; i < 150; ++i)
			{
				if (stat_txt[i] != '\0')++stat_size;
				else break;
			}

			Draw->DrawTextW(stat_txt, stat_size, midFormat, D2D1::RectF(2.0f, ground + 5.0f, scr_width, scr_height), txtBrush);
		}


		//////////////////////////

		Draw->EndDraw();

	}

	ClearResources();
	std::remove(tmp_file);

	return (int) bMsg.wParam;
}