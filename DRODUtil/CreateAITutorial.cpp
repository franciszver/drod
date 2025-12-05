// Utility to create "AI Tutorial" hold with a simple straight path level
// Compile and run this to create the hold programmatically

#include "../DRODLib/DbHolds.h"
#include "../DRODLib/DbLevels.h"
#include "../DRODLib/DbRooms.h"
#include "../DRODLib/TileConstants.h"
#include "../DRODLib/EntranceData.h"
#include "../DRODLib/CurrentGame.h"
#include "../DRODLib/Db.h"
#include "../DRODLib/DbPlayers.h"
#include "../DROD/DrodBitmapManager.h"
#include <BackEndLib/Files.h>
#include <BackEndLib/Ports.h>
#include <stdio.h>

void GetAppPath(const char* argv0, WSTRING& wstrPath)
{
#ifdef WIN32
	WCHAR wszPath[MAX_PATH+1];
	GetModuleFileName(NULL, wszPath, MAX_PATH);
	wstrPath = wszPath;
	WSTRING::size_type slashPos = wstrPath.find_last_of(wszSlash);
	if (slashPos != WSTRING::npos)
		wstrPath.resize(slashPos + 1);
#else
	// Simplified for other platforms - would need proper path resolution
	wstrPath = L"./";
#endif
}

int main(int argc, char* argv[])
{
	// Initialize CFiles
	const WCHAR wszUniqueResFile[] = {
		We('d'), We('r'), We('o'), We('d'), We('5'), We('_'), We('0'), We('.'), We('d'), We('a'), We('t'), We(0) };
	WSTRING wstrPath;
	GetAppPath(argv[0], wstrPath);

	std::vector<string> datFiles;
	std::vector<string> playerDataSubDirs;
	playerDataSubDirs.push_back("Bitmaps");
	playerDataSubDirs.push_back("Homemade");
	playerDataSubDirs.push_back("Music");
	playerDataSubDirs.push_back("Sounds");
	CFiles::InitAppVars(wszUniqueResFile, datFiles, playerDataSubDirs);
	CFiles *pFiles = new CFiles(wstrPath.c_str(), wszDROD, wszDROD_VER, false);
	if (CFiles::bad_data_path_file) {
		printf("Error: Invalid data path file\n");
		delete pFiles;
		return 1;
	}

	// Initialize database
	CDb *pDb = g_pTheDB = new CDb;
	if (!pDb->Open()) {
		printf("Error: Failed to open database\n");
		delete pFiles;
		return 1;
	}

	// Get or create a player
	UINT dwPlayerID = pDb->GetPlayerID();
	if (!dwPlayerID) {
		// Create a default player if none exists
		CDbPlayer *pPlayer = pDb->Players.GetNew();
		if (!pPlayer) {
			printf("Error: Failed to create player\n");
			delete pDb;
			delete pFiles;
			return 1;
		}
		pPlayer->NameText = L"Player";
		if (!pPlayer->Update()) {
			printf("Error: Failed to save player\n");
			delete pPlayer;
			delete pDb;
			delete pFiles;
			return 1;
		}
		dwPlayerID = pPlayer->dwPlayerID;
		delete pPlayer;
		pDb->Commit();
	}

	printf("Creating AI Tutorial hold...\n");

	// Create hold
	CDbHold *pHold = g_pTheDB->Holds.GetNew();
	if (!pHold)
	{
		printf("Error: Failed to create hold\n");
		return 1;
	}

	pHold->NameText = L"AI Tutorial";
	pHold->DescriptionText = L"Tutorial level created by AI";
	pHold->dwPlayerID = dwPlayerID;

	if (!pHold->Update())
	{
		printf("Error: Failed to save hold\n");
		delete pHold;
		return 1;
	}

	const UINT dwHoldID = pHold->dwHoldID;
	printf("Created hold ID: %u\n", dwHoldID);

	// Create level
	CDbLevel *pLevel = g_pTheDB->Levels.GetNew();
	if (!pLevel)
	{
		printf("Error: Failed to create level\n");
		delete pHold;
		return 1;
	}

	pLevel->dwHoldID = dwHoldID;
	pLevel->dwPlayerID = dwPlayerID;
	pLevel->NameText = L"Level 1";

	if (!pLevel->Update())
	{
		printf("Error: Failed to save level\n");
		delete pLevel;
		delete pHold;
		return 1;
	}

	const UINT dwLevelID = pLevel->dwLevelID;
	printf("Created level ID: %u\n", dwLevelID);

	// Insert level into hold
	pHold->InsertLevel(pLevel);

	// Create room
	CDbRoom *pRoom = g_pTheDB->Rooms.GetNew();
	if (!pRoom)
	{
		printf("Error: Failed to create room\n");
		delete pLevel;
		delete pHold;
		return 1;
	}

	pRoom->dwLevelID = dwLevelID;
	pRoom->dwRoomX = 0;
	pRoom->dwRoomY = 0;
	pRoom->wRoomCols = CDrodBitmapManager::DISPLAY_COLS;  // 38
	pRoom->wRoomRows = CDrodBitmapManager::DISPLAY_ROWS;  // 32
	pRoom->style = L"Badlands";
	pRoom->bIsRequired = true;
	pRoom->bIsSecret = false;

	if (!pRoom->AllocTileLayers())
	{
		printf("Error: Failed to allocate tile layers\n");
		delete pRoom;
		delete pLevel;
		delete pHold;
		return 1;
	}

	// Initialize all tiles to floor first
	const UINT dwSquareCount = pRoom->CalcRoomArea();
	memset(pRoom->pszOSquares, T_FLOOR, dwSquareCount * sizeof(char));
	memset(pRoom->pszFSquares, T_EMPTY, dwSquareCount * sizeof(char));
	pRoom->ClearTLayer();

	pRoom->coveredOSquares.Init(pRoom->wRoomCols, pRoom->wRoomRows);
	pRoom->tileLights.Init(pRoom->wRoomCols, pRoom->wRoomRows);

	// Create the path: straight line down the middle
	const UINT centerX = pRoom->wRoomCols / 2;  // 19
	const UINT startY = 0;  // Top
	const UINT endY = pRoom->wRoomRows - 1;  // Bottom (31)

	// Set walls everywhere except the center path
	for (UINT y = 0; y < pRoom->wRoomRows; ++y)
	{
		for (UINT x = 0; x < pRoom->wRoomCols; ++x)
		{
			if (x != centerX)
			{
				// Set wall
				pRoom->pszOSquares[y * pRoom->wRoomCols + x] = T_WALL;
			}
			// else keep as floor (center path)
		}
	}

	// Set exit stairs at bottom center
	pRoom->pszOSquares[endY * pRoom->wRoomCols + centerX] = T_STAIRS;

	// Create exit (stairs lead to entrance 0, which means level completion)
	// For a tutorial level, stairs without a linked entrance will just complete the level
	CExitData *pExit = new CExitData(0, centerX, centerX, endY, endY);
	pRoom->Exits.push_back(pExit);

	// Save room
	if (!pRoom->Update())
	{
		printf("Error: Failed to save room\n");
		delete pRoom;
		delete pLevel;
		delete pHold;
		return 1;
	}

	const UINT dwRoomID = pRoom->dwRoomID;
	printf("Created room ID: %u\n", dwRoomID);

	// Create entrance (player starts at top center)
	const UINT entranceX = centerX;
	const UINT entranceY = startY;
	CEntranceData *pEntrance = new CEntranceData(0, 0, dwRoomID,
		entranceX, entranceY,
		SE, true, CEntranceData::DD_Always, 0);
	pEntrance->DescriptionText = L"Entrance 1";
	pHold->AddEntrance(pEntrance);

	// Update hold
	if (!pHold->Update())
	{
		printf("Error: Failed to update hold\n");
		delete pRoom;
		delete pLevel;
		delete pHold;
		return 1;
	}

	// Commit changes
	g_pTheDB->Commit();

	printf("Success! Created:\n");
	printf("  Hold: %u - AI Tutorial\n", dwHoldID);
	printf("  Level: %u - Level 1\n", dwLevelID);
	printf("  Room: %u - Start at (%u,%u), Exit at (%u,%u)\n", 
		dwRoomID, entranceX, entranceY, centerX, endY);

	delete pRoom;
	delete pLevel;
	delete pHold;
	delete pDb;
	delete pFiles;

	return 0;
}

