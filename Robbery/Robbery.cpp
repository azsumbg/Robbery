#include "framework.h"
#include "Robbery.h"
#include <mmsystem.h>
#include <d2d1.h>
#include <dwrite.h>
#include "errh.h"
#include "FCheck.h"
#include "D2BMPLOADER.h"
#include "gifresizer.h"
#include "PointDistance.h"
#include "robeng.h"
#include <vector>
#include <ctime>
#include <chrono>
#include <random>
#include <fstream>

#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dwrite.lib")
#pragma comment (lib, "errh.lib")
#pragma comment (lib, "fcheck.lib")
#pragma comment (lib, "d2bmploader.lib")
#pragma comment (lib, "gifresizer.lib")
#pragma comment (lib, "pointdistance.lib")
#pragma comment (lib, "robeng.lib")

constexpr wchar_t bWinClassName[14]{ L"MyRobberyGame" };

constexpr char temp_file[20]{ ".\\res\\data\\temp.dat" };
constexpr wchar_t Ltemp_file[20]{ L".\\res\\data\\temp.dat" };
constexpr wchar_t sound_file[19]{ L".\\res\\snd\\main.wav" };
constexpr wchar_t save_file[20]{ L".\\res\\data\\save.dat" };
constexpr wchar_t help_file[20]{ L".\\res\\data\\help.dat" };
constexpr wchar_t record_file[22]{ L".\\res\\data\\record.dat" };

constexpr int mNew = 1001;
constexpr int mSpeed = 1002;
constexpr int mExit = 1003;
constexpr int mSave = 1004;
constexpr int mLoad = 1005;
constexpr int mHoF = 1006;

constexpr int no_record = 2001;
constexpr int first_record = 2002;
constexpr int record = 2003;

WNDCLASS bWinClass{};
HINSTANCE bIns = nullptr;
HWND bHwnd = nullptr;
HMENU bBar = nullptr;
HMENU bMain = nullptr;
HMENU bStore = nullptr;
HICON mainIcon = nullptr;
HCURSOR mainCursor = nullptr;
HCURSOR outCursor = nullptr;
HDC PaintDC = nullptr;
PAINTSTRUCT bPaint{};
MSG bMsg{};
BOOL bRet = 0;
UINT bTimer = -1;
POINT cur_pos{};

D2D1_RECT_F b1Rect{ 10.0f, 10.0f, scr_width / 3 - 10.0f, 50.0f };
D2D1_RECT_F b2Rect{ scr_width / 3 + 10.0f, 10.0f, scr_width * 2 / 3 - 10.0f, 50.0f };
D2D1_RECT_F b3Rect{ scr_width * 2 / 3 - 10.0f, 10.0f, scr_width, 50.0f };

bool pause = false;
bool in_client = true;
bool show_help = false;
bool sound = true;
bool name_set = false;
bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;

float game_speed = 1.0f;
int score = 0;
int mins = 0;
int secs = 0;

wchar_t current_player[16] = L"ONE ROBBER";

std::random_device rand_device{};
std::seed_seq* seed_sequencer = nullptr;
std::mt19937* twister = nullptr;

///////////////////////////////////////////////////////////////////////////

ID2D1Factory* iFactory = nullptr;
ID2D1HwndRenderTarget* Draw = nullptr;
ID2D1RadialGradientBrush* BckgBrush = nullptr;
ID2D1SolidColorBrush* TextBrush = nullptr;
ID2D1SolidColorBrush* HgltBrush = nullptr;
ID2D1SolidColorBrush* InactBrush = nullptr;

IDWriteFactory* iWriteFactory = nullptr;
IDWriteTextFormat* nrmText = nullptr;
IDWriteTextFormat* bigText = nullptr;

ID2D1Bitmap* bmpClay = nullptr;
ID2D1Bitmap* bmpDesert = nullptr;
ID2D1Bitmap* bmpDistracter = nullptr;
ID2D1Bitmap* bmpGold = nullptr;
ID2D1Bitmap* bmpGrain = nullptr;
ID2D1Bitmap* bmpGrass = nullptr;
ID2D1Bitmap* bmpSea = nullptr;
ID2D1Bitmap* bmpTreasure = nullptr;
ID2D1Bitmap* bmpWood = nullptr;

ID2D1Bitmap* bmpHeroL[10]{ nullptr };
ID2D1Bitmap* bmpHeroR[10]{ nullptr };

ID2D1Bitmap* bmpFatOffL[10]{ nullptr };
ID2D1Bitmap* bmpFatOffR[10]{ nullptr };

ID2D1Bitmap* bmpLazyOffL[2]{ nullptr };
ID2D1Bitmap* bmpLazyOffR[2]{ nullptr };

ID2D1Bitmap* bmpMotoOffL[20]{ nullptr };
ID2D1Bitmap* bmpMotoOffR[20]{ nullptr };

ID2D1Bitmap* bmpSergeL[8]{ nullptr };
ID2D1Bitmap* bmpSergeR[8]{ nullptr };
/////////////////////////////////////////////////////////////

//GAME VARS ************************************

dll::FIELD Field{};

dll::hero_ptr Robber = nullptr;
bool robber_moving = false;
float robber_dest_x = 0;
float robber_dest_y = 0;
float go_up = false;
float go_right = false;

std::vector<dll::ATOM> vTreasures;
std::vector<dll::police_ptr> vPolice;

int distracter_lifes = 1000;
dll::ATOM* distracter = nullptr;

//////////////////////////////////////////////////////////////////////
template<typename T> concept CanBeReleased = requires (T parameter)
{
    parameter.Release();
};
template<CanBeReleased PAR> bool ClearMem(PAR** param)
{
    if ((*param))
    {
        (*param)->Release();
        return true;
    }
    return false;
}
void LogError(LPCWSTR what)
{
    std::wofstream log(L".\\res\\data\\error.log", std::ios::app);
    log << what << L", time stamp of error: " << std::chrono::system_clock::now() << std::endl;
    log.close();
}
void ReleaseResources()
{
    ClearMem(&iFactory);
    ClearMem(&Draw);
    ClearMem(&BckgBrush);
    ClearMem(&TextBrush);
    ClearMem(&HgltBrush);
    ClearMem(&InactBrush);

    ClearMem(&iWriteFactory);
    ClearMem(&nrmText);
    ClearMem(&bigText);

    ClearMem(&bmpClay);
    ClearMem(&bmpDesert);
    ClearMem(&bmpDistracter);
    ClearMem(&bmpGold);
    ClearMem(&bmpGrain);
    ClearMem(&bmpGrass);
    ClearMem(&bmpSea);
    ClearMem(&bmpTreasure);
    ClearMem(&bmpWood);

    for (int i = 0; i < 10; i++)ClearMem(&bmpHeroL[i]);
    for (int i = 0; i < 10; i++)ClearMem(&bmpHeroR[i]);

    for (int i = 0; i < 10; i++)ClearMem(&bmpFatOffL[i]);
    for (int i = 0; i < 10; i++)ClearMem(&bmpFatOffR[i]);

    for (int i = 0; i < 2; i++)ClearMem(&bmpLazyOffL[i]);
    for (int i = 0; i < 2; i++)ClearMem(&bmpLazyOffR[i]);

    for (int i = 0; i < 20; i++)ClearMem(&bmpMotoOffL[i]);
    for (int i = 0; i < 20; i++)ClearMem(&bmpMotoOffR[i]);

    for (int i = 0; i < 8; i++)ClearMem(&bmpSergeL[i]);
    for (int i = 0; i < 8; i++)ClearMem(&bmpSergeR[i]);

}
void ErrExit(int what)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(what), L"Критична грешка !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

    ReleaseResources();
    std::remove(temp_file);
    exit(1);
}
BOOL CheckRecord()
{
    if (score < 1)return no_record;
    int result = 0;
    
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

    switch (CheckRecord())
    {
    case no_record:
        if (bigText && TextBrush)
        {
            Draw->BeginDraw();
            Draw->Clear(D2D1::ColorF(D2D1::ColorF::Khaki));
            Draw->DrawTextW(L"ОКОШАРИХА ТЕ !", 15, bigText, D2D1::RectF(100.0f, 250.0f, scr_width, scr_height), TextBrush);
            Draw->EndDraw();
            if (sound)PlaySound(L".\\res\\snd\\loose.wav", NULL, SND_SYNC);
            else Sleep(2500);
        }
        break;

    case first_record:
        if (bigText && TextBrush)
        {
            Draw->BeginDraw();
            Draw->Clear(D2D1::ColorF(D2D1::ColorF::Khaki));
            Draw->DrawTextW(L"ПЪРВИ РЕКОРД НА ИГРАТА !", 25, bigText, D2D1::RectF(10.0f, 250.0f, 
                scr_width, scr_height), TextBrush);
            Draw->EndDraw();
            if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_SYNC);
            else Sleep(2500);
        }
        break;

    case record:
        if (bigText && TextBrush)
        {
            Draw->BeginDraw();
            Draw->Clear(D2D1::ColorF(D2D1::ColorF::Khaki));
            Draw->DrawTextW(L"НОВ СВЕТОВЕН РЕКОРД !", 22, bigText, D2D1::RectF(10.0f, 250.0f,
                scr_width, scr_height), TextBrush);
            Draw->EndDraw();
            if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_SYNC);
            else Sleep(2500);
        }
        break;
    }

    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}
void InitGame()
{
    mins = 0;
    secs = 0;
    game_speed = 1.0f;
    score = 0;

    wcscpy_s(current_player, L"ONE ROBBER");
    name_set = false;

    ClearMem(&Robber);
    Robber = reinterpret_cast<dll::hero_ptr>(dll::CreatureFactory(creatures::hero, 20.0f, 650.0f));
    
    if (distracter)
    {
        delete distracter;
        distracter = nullptr;
    }
    if (!vPolice.empty())
    {
        for (int i = 0; i < vPolice.size(); i++)ClearMem(&vPolice[i]);

    }
    vPolice.clear();

    vTreasures.clear();

    std::uniform_int_distribution distrib(0, 1);

    for (int i = 8; i >= 0; i--)
    {
        if (distrib(*twister) == 1)
        {
            std::uniform_int_distribution col_distrib(0, 19);
            int temp_col = col_distrib(*twister);
            dll::SPOT OneSpot = Field.GetSpotDims(i, temp_col);
            vTreasures.push_back(dll::ATOM(OneSpot.x, OneSpot.y, 50.0f, 47.0f));

            std::uniform_int_distribution pol_distrib(0, 3);
            creatures police_officer = static_cast<creatures>(pol_distrib(*twister));

            vPolice.push_back(reinterpret_cast<dll::police_ptr>(dll::CreatureFactory(police_officer,
                vTreasures.back().x - 50.0f, vTreasures.back().y)));

            if (distrib(*twister) == 0)vPolice.back()->dir = dirs::right;
            else vPolice.back()->dir = dirs::up;

            vPolice.back()->now_patroling = true;
        }
    }

}
void LevelUp()
{
    int bonus = 180 - secs;
    if (bonus <= 0)bonus = 0;

    Draw->BeginDraw();
    Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkBlue));
    if (bigText && HgltBrush)
        Draw->DrawTextW(L"ОТКРАДНА ВСИЧКО В НИВОТО !", 27, bigText, D2D1::RectF(30.0f, 300.0f, scr_width, scr_height), TextBrush);
    Draw->EndDraw();
    if (sound)PlaySound(L".\\res\\snd\\levelup.wav", NULL, SND_SYNC);
    else Sleep(2500);

    if (bonus > 0)
    {
        for (int i = 0; i <= bonus; i++)
        {
            wchar_t bonus_txt[50] = L"БОНУС ЗА ВРЕМЕ: ";
            wchar_t add[5] = L"\0";
            int txt_size = 0;

            score += 10 * (int)(game_speed);
            wsprintf(add, L"%d", score);
            wcscat_s(bonus_txt, add);

            txt_size = 0;
            for (int i = 0; i < 50; i++)
            {
                if (bonus_txt[i] != '\0')txt_size++;
                else break;
            }
            
            Draw->BeginDraw();
            Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkBlue));
            if (bigText && HgltBrush)
                Draw->DrawTextW(bonus_txt, txt_size, bigText, D2D1::RectF(100.0f, 300.0f, scr_width, scr_height), TextBrush);
            Draw->EndDraw();
            if (sound)mciSendString(L"play .\\res\\snd\\tick.wav", NULL, NULL,NULL);
            else Sleep(100);
        }
        Sleep(1500);
    }

    mins = 0;
    secs = 0;
    game_speed++;

    ClearMem(&Robber);
    Robber = reinterpret_cast<dll::hero_ptr>(dll::CreatureFactory(creatures::hero, 20.0f, 650.0f));

    if (distracter)
    {
        delete distracter;
        distracter = nullptr;
    }
    if (!vPolice.empty())
    {
        for (int i = 0; i < vPolice.size(); i++)ClearMem(&vPolice[i]);

    }
    vPolice.clear();

    vTreasures.clear();

    dll::FIELD new_field{};
    Field = new_field;

    std::uniform_int_distribution distrib(0, 1);

    for (int i = 8; i >= 0; i--)
    {
        if (distrib(*twister) == 1)
        {
            std::uniform_int_distribution col_distrib(0, 19);
            int temp_col = col_distrib(*twister);
            dll::SPOT OneSpot = Field.GetSpotDims(i, temp_col);
            vTreasures.push_back(dll::ATOM(OneSpot.x, OneSpot.y, 50.0f, 47.0f));

            std::uniform_int_distribution pol_distrib(0, 3);
            creatures police_officer = static_cast<creatures>(pol_distrib(*twister));

            vPolice.push_back(reinterpret_cast<dll::police_ptr>(dll::CreatureFactory(police_officer,
                vTreasures.back().x - 50.0f, vTreasures.back().y)));

            if (distrib(*twister) == 0)vPolice.back()->dir = dirs::right;
            else vPolice.back()->dir = dirs::up;

            vPolice.back()->now_patroling = true;
        }
    }
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_INITDIALOG:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, LPARAM(mainIcon));
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
            if (GetDlgItemTextW(hwnd, IDC_NAME, current_player, 16) < 1)
            {
                wcscpy_s(current_player, L"ONE ROBBER");
                name_set = false;
                if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
                MessageBox(bHwnd, L"Ха, ха, хааа !\n\nЗабрави си името !", L"Забраватор", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
                EndDialog(hwnd, IDCANCEL);
                break;
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
        {
            std::vector<int> vSeed;
            for (int i = 0; i <= 500; i++)vSeed.push_back(rand_device());
            seed_sequencer = new std::seed_seq(vSeed.begin(), vSeed.end());
            twister = new std::mt19937(*seed_sequencer);

            SetTimer(hwnd, bTimer, 1000, NULL);
            
            bBar = CreateMenu();
            bMain = CreateMenu();
            bStore = CreateMenu();

            AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
            AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bStore), L"Меню за данни");

            AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
            AppendMenu(bMain, MF_STRING, mSpeed, L"Турбо режим");
            AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
            AppendMenu(bMain, MF_STRING, mExit, L"Изход");
    
            AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
            AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
            AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
            AppendMenu(bStore, MF_STRING, mHoF, L"Зала на славата !");

            SetMenu(hwnd, bBar);
            InitGame();
        }
        break;

    case WM_CLOSE:
        pause = true;
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(hwnd, L"Ако излезеш, ще загубиш тази игра !\n\nНаистина ли излизаш ?",
            L"Изход !", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
        {
            pause = false;
            break;
        }
        GameOver();
        break;

    case WM_PAINT:
        PaintDC = BeginPaint(hwnd, &bPaint);
        FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(20, 20, 20)));
        EndPaint(hwnd, &bPaint);
        break;

    case WM_TIMER:
        if (pause)break;
        secs++;
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

            if (cur_pos.y <= 50)
            {
                if (cur_pos.x >= b1Rect.left && cur_pos.x <= b1Rect.right)
                {
                    if (!b1Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = true;
                        b2Hglt = false;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b2Rect.left && cur_pos.x <= b2Rect.right)
                {
                    if (!b2Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = true;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b3Rect.left && cur_pos.x <= b3Rect.right)
                {
                    if (!b3Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = false;
                        b3Hglt = true;
                    }
                }

                SetCursor(outCursor);
                return true;
            }
            else 
            {
                if (b1Hglt || b2Hglt || b3Hglt)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                    b1Hglt = false;
                    b2Hglt = false;
                    b3Hglt = false;
                }
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
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }

            LoadCursor(NULL, IDC_ARROW);
            return true;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case mNew:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ако рестартираш, ще загубиш тази игра !\n\nНаистина ли рестартираш ?",
                L"Рестарт !", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            InitGame();
            pause = false;
            break;

        case mSpeed:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Наистина ли да увелича скоростта на играта ?",
                L"Турбо !", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            ++game_speed;
            pause = false;
            break;

        case mExit:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;

        }
        break;

    case WM_LBUTTONDOWN:
        if (HIWORD(lParam) <= 50)
        {
            if (LOWORD(lParam) >= b1Rect.left && LOWORD(lParam) <= b1Rect.right)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                if (DialogBox(bIns, MAKEINTRESOURCE(IDD_PLAYER), hwnd, &DlgProc) == IDOK)name_set = true;
                break;
            }
            if (LOWORD(lParam) >= b2Rect.left && LOWORD(lParam) <= b2Rect.right)
            {
                mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                if (sound)
                {
                    sound = false;
                    PlaySound(NULL, NULL, NULL);
                    break;
                }
                else
                {
                    sound = true;
                    PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);
                    break;
                }
            }
            
            break;
        }
        else if (Robber)
        {
            robber_moving = true;
            robber_dest_x = LOWORD(lParam);
            robber_dest_y = HIWORD(lParam);
            if (Robber->x < robber_dest_x)go_right = true;
            else if (Robber->x > robber_dest_x)go_right = false;
            if (Robber->y > robber_dest_y)go_up = true;
            else if (Robber->y < robber_dest_y)go_up = false;
            Robber->LineSetup(robber_dest_x, robber_dest_y);
        }
        break;

    case WM_RBUTTONDOWN:
        if (distracter)
        {
            if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
            break;
        }
        if (Robber)
        {
            distracter = new dll::ATOM(Robber->ex, Robber->y, 25.0f, 50.0f);
            distracter_lifes = 1000;
        }
        break;


    default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return (LRESULT)(FALSE);
}

void CreateResources()
{
    int result = 0;
    CheckFile(Ltemp_file, &result);
    if (result == FILE_EXIST)ErrExit(eStarted);
    else
    {
        std::wofstream start(Ltemp_file);
        start << L"Game is started: " << std::chrono::system_clock::now();
        start.close();
    }
    
    int win_start_x = (int)(GetSystemMetrics(SM_CXSCREEN) / 2.0 - scr_width / 2);
    if (GetSystemMetrics(SM_CXSCREEN) < win_start_x + scr_width 
        || GetSystemMetrics(SM_CYSCREEN) < scr_height + 10.0f)ErrExit(eScreen);
    
    mainIcon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 256, 256, LR_LOADFROMFILE));
    if (!mainIcon)ErrExit(eIcon);
    mainCursor = LoadCursorFromFile(L".\\res\\main.ani");
    outCursor = LoadCursorFromFile(L".\\res\\out.ani");
    if (!mainCursor || !outCursor)ErrExit(eCursor);

    bWinClass.lpszClassName = bWinClassName;
    bWinClass.hInstance = bIns;
    bWinClass.lpfnWndProc = &WinProc;
    bWinClass.hbrBackground = CreateSolidBrush(RGB(20, 20, 20));
    bWinClass.hIcon = mainIcon;
    bWinClass.hCursor = mainCursor;
    bWinClass.style = CS_DROPSHADOW;

    if (!RegisterClass(&bWinClass))ErrExit(eClass);

    bHwnd = CreateWindowW(bWinClassName, L"БЪРЗИЯТ ОБИРДЖИЯ !", WS_CAPTION | WS_SYSMENU, win_start_x, 10, (int)(scr_width),
        (int)(scr_height), NULL, NULL, bIns, NULL);
    if (!bHwnd)ErrExit(eWindow);
    else
    {
        ShowWindow(bHwnd, SW_SHOWDEFAULT);

        HRESULT hr = S_OK;

        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
        if (hr != S_OK)
        {
            LogError(L"Error creating iFactory !");
            ErrExit(eD2D);
        }

        if (iFactory)
        {
            hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
                D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
            if (hr != S_OK)
            {
                LogError(L"Error creating Draw object !");
                ErrExit(eD2D);
            }

            if (Draw)
            {
                D2D1_GRADIENT_STOP gStops[2]{};
                ID2D1GradientStopCollection* gColl = nullptr;

                gStops[0].position = 0;
                gStops[0].color = D2D1::ColorF(D2D1::ColorF::Beige);
                gStops[1].position = 1.0f;
                gStops[1].color = D2D1::ColorF(D2D1::ColorF::DarkOrange);

                hr = Draw->CreateGradientStopCollection(gStops, 2, &gColl);
                if (hr != S_OK)
                {
                    LogError(L"Error creating GradientStopCollection object !");
                    ErrExit(eD2D);
                }
                if (gColl)
                {
                    hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(scr_width / 2, 25.0f),
                        D2D1::Point2F(0, 0), scr_width / 2, 25.0f), gColl, &BckgBrush);
                    if (hr != S_OK)
                    {
                        LogError(L"Error creating BckgBrusg object !");
                        ErrExit(eD2D);
                    }

                    ClearMem(&gColl);
                }

                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &TextBrush);
                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &HgltBrush);
                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkSlateGray), &InactBrush);
                if (hr != S_OK)
                {
                    LogError(L"Error creating Text brushes  !");
                    ErrExit(eD2D);
                }

                bmpClay = Load(L".\\res\\img\\field\\clay.png", Draw);
                if (!bmpClay)
                {
                    LogError(L"Error loading bmpClay object !");
                    ErrExit(eD2D);
                }
                bmpDesert = Load(L".\\res\\img\\field\\desert.png", Draw);
                if (!bmpDesert)
                {
                    LogError(L"Error loading bmpDesert object !");
                    ErrExit(eD2D);
                }
                bmpDistracter = Load(L".\\res\\img\\field\\distracter.png", Draw);
                if (!bmpDistracter)
                {
                    LogError(L"Error loading bmpDistracter object !");
                    ErrExit(eD2D);
                }
                bmpGold = Load(L".\\res\\img\\field\\gold.png", Draw);
                if (!bmpGold)
                {
                    LogError(L"Error loading bmpGold object !");
                    ErrExit(eD2D);
                }
                bmpGrain = Load(L".\\res\\img\\field\\grain.png", Draw);
                if (!bmpGrain)
                {
                    LogError(L"Error loading bmpGrain object !");
                    ErrExit(eD2D);
                }
                bmpGrass = Load(L".\\res\\img\\field\\grass.png", Draw);
                if (!bmpGrass)
                {
                    LogError(L"Error loading bmpGrass object !");
                    ErrExit(eD2D);
                }
                bmpSea = Load(L".\\res\\img\\field\\sea.png", Draw);
                if (!bmpSea)
                {
                    LogError(L"Error loading bmpSea object !");
                    ErrExit(eD2D);
                }
                bmpTreasure = Load(L".\\res\\img\\field\\treasure.png", Draw);
                if (!bmpTreasure)
                {
                    LogError(L"Error loading bmpTreasure object !");
                    ErrExit(eD2D);
                }
                bmpWood = Load(L".\\res\\img\\field\\wood.png", Draw);
                if (!bmpWood)
                {
                    LogError(L"Error loading bmpWood object !");
                    ErrExit(eD2D);
                }

                for (int i = 0; i < 10; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\hero\\l\\";
                    wchar_t add[5] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpHeroL[i] = Load(name, Draw);

                    if (!bmpHeroL[i])
                    {
                        LogError(L"Error loading bmpHeroL object !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 10; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\hero\\r\\";
                    wchar_t add[5] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpHeroR[i] = Load(name, Draw);

                    if (!bmpHeroR[i])
                    {
                        LogError(L"Error loading bmpHeroR object !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 10; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\police\\l\\1\\";
                    wchar_t add[5] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpFatOffL[i] = Load(name, Draw);

                    if (!bmpFatOffL[i])
                    {
                        LogError(L"Error loading bmpFatOffL object !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 10; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\police\\r\\1\\";
                    wchar_t add[5] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpFatOffR[i] = Load(name, Draw);

                    if (!bmpFatOffR[i])
                    {
                        LogError(L"Error loading bmpFatOffR object !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 2; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\police\\l\\2\\";
                    wchar_t add[5] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpLazyOffL[i] = Load(name, Draw);

                    if (!bmpLazyOffL[i])
                    {
                        LogError(L"Error loading bmpLazyOffL object !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 2; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\police\\r\\2\\";
                    wchar_t add[5] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpLazyOffR[i] = Load(name, Draw);

                    if (!bmpLazyOffR[i])
                    {
                        LogError(L"Error loading bmpLazyOffR object !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 20; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\police\\l\\3\\";
                    wchar_t add[5] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpMotoOffL[i] = Load(name, Draw);

                    if (!bmpMotoOffL[i])
                    {
                        LogError(L"Error loading bmpMotoOffL object !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 20; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\police\\r\\3\\";
                    wchar_t add[5] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpMotoOffR[i] = Load(name, Draw);

                    if (!bmpMotoOffR[i])
                    {
                        LogError(L"Error loading bmpMotoOffR object !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 8; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\police\\l\\4\\";
                    wchar_t add[5] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpSergeL[i] = Load(name, Draw);

                    if (!bmpSergeL[i])
                    {
                        LogError(L"Error loading bmpSergeL object !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 8; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\police\\r\\4\\";
                    wchar_t add[5] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpSergeR[i] = Load(name, Draw);

                    if (!bmpSergeR[i])
                    {
                        LogError(L"Error loading bmpSergeR object !");
                        ErrExit(eD2D);
                    }
                }
            }

            hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(&iWriteFactory));
            if (hr != S_OK)
            {
                LogError(L"Error creating iWriteFactory !");
                ErrExit(eD2D);
            }

            if (iWriteFactory)
            {
                hr = iWriteFactory->CreateTextFormat(L"Sitka", NULL, DWRITE_FONT_WEIGHT_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                    DWRITE_FONT_STRETCH_NORMAL, 24.0f, L"", &nrmText);
                hr = iWriteFactory->CreateTextFormat(L"Sitka", NULL, DWRITE_FONT_WEIGHT_BLACK, DWRITE_FONT_STYLE_ITALIC,
                    DWRITE_FONT_STRETCH_NORMAL, 64.0f, L"", &bigText);
                if (hr != S_OK)
                {
                    LogError(L"Error creating iWrite Text Formats !");
                    ErrExit(eD2D);
                }
            }
        }

        std::uniform_int_distribution distrib(0, 3);

        if (Draw && bigText && HgltBrush)
        {
            for (int i = 0; i < 80; i++)
            {
                Draw->BeginDraw();
                Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkBlue));
                if (distrib(*twister) == 1)
                {
                    mciSendString(L"play .\\res\\snd\\buzz.wav", NULL, NULL, NULL);
                    Draw->DrawTextW(L"БЕСЕН ОБИРДЖИЯ !", 17, bigText, D2D1::RectF(200.0f, 300.0f,
                        scr_width, scr_height), HgltBrush);
                    Sleep(100);
                    Draw->EndDraw();
                }
                else Draw->EndDraw();
            }

            mciSendString(L"play .\\res\\snd\\boom.wav", NULL, NULL, NULL);

            Draw->BeginDraw();
            Draw->DrawTextW(L"БЕСЕН ОБИРДЖИЯ !", 17, bigText, D2D1::RectF(200.0f, 300.0f,
                scr_width, scr_height), HgltBrush);
            Draw->DrawTextW(L"dev. Daniel !", 14, bigText, D2D1::RectF(400.0f, 600.0f, scr_width, scr_height), HgltBrush);
            Draw->EndDraw();

            Sleep(1500);
        }
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,_In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;
    if (!bIns)
    {
        LogError(L"Error obtaining hInstance from OS !");
        ErrExit(eClass);
    }
    CreateResources();

    while (bMsg.message != WM_QUIT)
    {
        if ((bRet = PeekMessage(&bMsg, bHwnd, NULL, NULL, PM_REMOVE)) != 0)
        {
            if (bRet == -1)ErrExit(eMsg);
            TranslateMessage(&bMsg);
            DispatchMessageW(&bMsg);
        }

        if (pause)
        {
            if (show_help)continue;
            Draw->BeginDraw();
            Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkCyan));
            Draw->DrawTextW(L"ПАУЗА !", 8, bigText, D2D1::RectF(scr_width / 2 - 100.0f, 400.0f,
                scr_width, scr_height), HgltBrush);
            Draw->EndDraw();
            continue;
        }
        ///////////////////////////////////////////

        //ROBBER **********************************

        if (Robber)
        {
            if (robber_moving)
            {
                int current_col = (int)(Robber->x / 50);
                int current_row = (int)((Robber->y - 50.0f) / 50);
                dll::SPOT current_spot = Field.GetSpotDims(current_row, current_col);
                float move_speed = game_speed / 10 - current_spot.speed_modifier;
      
                Robber->Move(move_speed);
                if (go_up)
                {
                    if (go_right)
                    {
                        if (Robber->y <= robber_dest_y && Robber->ex >= robber_dest_x)robber_moving = false;
                    }
                    else
                    {
                        if(Robber->y <= robber_dest_y && Robber->x <= robber_dest_x)robber_moving = false;
                    }
                }
                else
                {
                    if (go_right)
                    {
                        if (Robber->ey >= robber_dest_y && Robber->ex >= robber_dest_x)robber_moving = false;
                    }
                    else
                    {
                        if (Robber->ey >= robber_dest_y && Robber->x <= robber_dest_x)robber_moving = false;
                    }
                }
            }
        }

        if (!vPolice.empty() && Robber)
        {
            for (int i = 0; i < vPolice.size(); ++i)
            {
                dll::AI_INPUT PoliceInfo{};

                if (!vTreasures.empty())
                {
                    std::vector<distance::OnePoint>vTreasurePoints;

                    for (int j = 0; j < vTreasures.size(); j++)
                        vTreasurePoints.push_back(distance::OnePoint{ vTreasures[j].x, vTreasures[j].y });

                    distance::OnePoint current_police_officer{ vPolice[i]->x,vPolice[i]->y };
                    distance::OnePoint nearest_treasure_point{};
                    
                    distance::SortPoints(vTreasurePoints, vTreasurePoints.size(),
                        current_police_officer, nearest_treasure_point);
                    
                    PoliceInfo.guard_obj_x = nearest_treasure_point.x;
                    PoliceInfo.guard_obj_y = nearest_treasure_point.y;
                }
                else
                {
                    PoliceInfo.guard_obj_x = -1.0f;
                    PoliceInfo.guard_obj_y = -1.0f;
                }
            
                if (distracter)
                {
                    PoliceInfo.distracter_x = distracter->x;
                    PoliceInfo.distracter_y = distracter->y;
                    PoliceInfo.distract_on = true;
                }
                else
                {
                    PoliceInfo.distracter_x = -1;
                    PoliceInfo.distracter_y = -1;
                    PoliceInfo.distract_on = false;
                }

                PoliceInfo.hero_x = Robber->x;
                PoliceInfo.hero_y = Robber->y;

                if (vPolice[i]->dir == dirs::right || vPolice[i]->dir == dirs::left)PoliceInfo.horizontal_patrol = true;
                else PoliceInfo.horizontal_patrol = false;
                
                if (abs(vPolice[i]->x - PoliceInfo.guard_obj_x) > 150.0f || abs(vPolice[i]->y - PoliceInfo.guard_obj_y) > 150.0f)
                    PoliceInfo.need_to_set_patrol = true;
                else PoliceInfo.need_to_set_patrol = false;
                
                int current_col = (int)(vPolice[i]->x / 50);
                int current_row = (int)((vPolice[i]->y - 50.0f) / 50);
                dll::SPOT current_spot = Field.GetSpotDims(current_row, current_col);
                PoliceInfo.move_gear = game_speed + current_spot.speed_modifier;

                vPolice[i]->AIDispatcher(PoliceInfo);   
            }
        }

        if (!vTreasures.empty() && Robber)
        {
            for (std::vector<dll::ATOM>::iterator gold = vTreasures.begin(); gold < vTreasures.end(); gold++)
            {
                if (!(Robber->x > gold->ex || Robber->ex<gold->x || Robber->y>gold->ey || Robber->ey < gold->y))
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\gold.wav", NULL, NULL, NULL);
                    score += 50 + (int)(game_speed);
                    vTreasures.erase(gold);
                    break;
                }
            }
        }

        if (distracter)
        {
            if (sound)mciSendString(L"play .\\res\\snd\\distracter.wav", NULL, NULL, NULL);
            distracter_lifes--;
            if (distracter_lifes <= 0)
            {
                delete distracter;
                distracter = nullptr;
            }
        }

        if (!vPolice.empty() && Robber)
        {
            for (std::vector<dll::police_ptr>::iterator police = vPolice.begin(); police < vPolice.end(); ++police)
            {
                if (!((*police)->x > Robber->ex || (*police)->ex<Robber->x
                    || (*police)->y>Robber->ey || (*police)->ey < Robber->y))
                {
                    PlaySound(NULL, NULL, NULL);
                    if (sound)PlaySound(L".\\res\\snd\\busted.wav", NULL, SND_SYNC);
                    GameOver();
                }
            }
        }

        if (vTreasures.empty())LevelUp();

        //Draw Things *******************************

        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::Khaki));

        if (nrmText && TextBrush && HgltBrush && InactBrush && BckgBrush)
        {
            Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), BckgBrush);
            if (name_set)
                Draw->DrawTextW(L"ИМЕ НА КРАДЕЦ", 14, nrmText, b1Rect, InactBrush);
            else
            {
                if(!b1Hglt)Draw->DrawTextW(L"ИМЕ НА КРАДЕЦ", 14, nrmText, b1Rect, TextBrush);
                else Draw->DrawTextW(L"ИМЕ НА КРАДЕЦ", 14, nrmText, b1Rect, HgltBrush);

            }
            if (!b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmText, b2Rect, TextBrush);
            else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmText, b2Rect, HgltBrush);
            if (!b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, b3Rect, TextBrush);
            else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, b3Rect, HgltBrush);
        }

        for (int rows = 0; rows < 14; rows++)
        {
            for (int cols = 0; cols < 20; cols++)
            {
                D2D1_RECT_F spotR{};
                dll::SPOT current_spot = Field.GetSpotDims(rows, cols);
                spotR.left = current_spot.x;
                spotR.right = current_spot.ex;
                spotR.top = current_spot.y;
                spotR.bottom = current_spot.ey;

                switch (Field.GetSpotType(rows, cols))
                {
                case fields::clay:
                    Draw->DrawBitmap(bmpClay, spotR);
                    break;

                case fields::desert:
                    Draw->DrawBitmap(bmpDesert, spotR);
                    break;

                case fields::gold:
                    Draw->DrawBitmap(bmpGold, spotR);
                    break;

                case fields::grain:
                    Draw->DrawBitmap(bmpGrain, spotR);
                    break;

                case fields::grass:
                    Draw->DrawBitmap(bmpGrass, spotR);
                    break;

                case fields::sea:
                    Draw->DrawBitmap(bmpSea, spotR);
                    break;

                case fields::wood:
                    Draw->DrawBitmap(bmpWood, spotR);
                    break;
                }
            }
        }
 
        if (nrmText && TextBrush)
        {
            wchar_t status_txt[150] = L"\0";
            wchar_t add[5] = L"\0";
            int txt_size = 0;

            wcscpy_s(status_txt, current_player);
            wcscat_s(status_txt, L", ВРЕМЕ: ");
            if (mins % 10 < 10)wcscat_s(status_txt, L"0");
            wsprintf(add, L"%d", mins);
            wcscat_s(status_txt, add);
            wcscat_s(status_txt, L" : ");
            if (secs - mins * 10 < 10)wcscat_s(status_txt, L"0");
            wsprintf(add, L"%d", secs - mins * 60);
            wcscat_s(status_txt, add);
            wcscat_s(status_txt, L", СКОРОСТ: ");
            wsprintf(add, L"%d", (int)(game_speed));
            wcscat_s(status_txt, add);
            wcscat_s(status_txt, L", ОТКРАДНАТО ЗЛАТО: ");
            wsprintf(add, L"%d", score);
            wcscat_s(status_txt, add);

            for (int i = 0; i < 150; i++)
            {
                if (status_txt[i] != '\0')txt_size++;
                else break;
            }
            Draw->DrawTextW(status_txt, txt_size, nrmText, D2D1::RectF(10.0f, scr_height - 40.0f, 
                scr_width, scr_height),TextBrush);
        }
        
        //////////////////////////////////////////////

        if (Robber)
        {
            int aframe = Robber->GetFrame();
            switch (Robber->dir)
            {
            case dirs::left:
                Draw->DrawBitmap(bmpHeroL[aframe], Resizer(bmpHeroL[aframe], Robber->x, Robber->y));
                break;

            case dirs::right:
                Draw->DrawBitmap(bmpHeroR[aframe], Resizer(bmpHeroR[aframe], Robber->x, Robber->y));
                break;
            }
        }
        if (!vTreasures.empty())
        {
            for (int i = 0; i < vTreasures.size(); i++)
                Draw->DrawBitmap(bmpTreasure, D2D1::RectF(vTreasures[i].x, vTreasures[i].y, vTreasures[i].ex, vTreasures[i].ey));
        }
        if (!vPolice.empty() && Robber)
        {
            for (int i = 0; i < vPolice.size(); i++)
            {
                int current_frame = vPolice[i]->GetFrame();

                switch (vPolice[i]->GetType())
                {
                case creatures::fat:
                    if (Robber->x >= vPolice[i]->x)
                        Draw->DrawBitmap(bmpFatOffL[current_frame], Resizer(bmpFatOffL[current_frame], 
                            vPolice[i]->x, vPolice[i]->y)); 
                    else
                        Draw->DrawBitmap(bmpFatOffR[current_frame], Resizer(bmpFatOffR[current_frame],
                            vPolice[i]->x, vPolice[i]->y));
                    break;

                case creatures::lazy:
                    if (Robber->x >= vPolice[i]->x)
                        Draw->DrawBitmap(bmpLazyOffL[current_frame], Resizer(bmpLazyOffL[current_frame],
                            vPolice[i]->x, vPolice[i]->y));
                    else
                        Draw->DrawBitmap(bmpLazyOffR[current_frame], Resizer(bmpLazyOffR[current_frame],
                            vPolice[i]->x, vPolice[i]->y));
                    break;

                case creatures::moto:
                    if (Robber->x >= vPolice[i]->x)
                        Draw->DrawBitmap(bmpMotoOffL[current_frame], Resizer(bmpMotoOffL[current_frame],
                            vPolice[i]->x, vPolice[i]->y));
                    else
                        Draw->DrawBitmap(bmpMotoOffR[current_frame], Resizer(bmpMotoOffR[current_frame],
                            vPolice[i]->x, vPolice[i]->y));
                    break;

                case creatures::serge:
                    if (Robber->x >= vPolice[i]->x)
                        Draw->DrawBitmap(bmpSergeL[current_frame], Resizer(bmpSergeL[current_frame],
                            vPolice[i]->x, vPolice[i]->y));
                    else
                        Draw->DrawBitmap(bmpSergeR[current_frame], Resizer(bmpSergeR[current_frame],
                            vPolice[i]->x, vPolice[i]->y));
                    break;
                }
            }
        }
        if (distracter)
            Draw->DrawBitmap(bmpDistracter, D2D1::RectF(distracter->x, distracter->y, distracter->ex, distracter->ey));


        //////////////////////////////////////////

        Draw->EndDraw();
    }

    std::remove(temp_file);
    ReleaseResources();
    return (int) bMsg.wParam;
}