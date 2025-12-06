// MoveQueueWidget.h
// Widget for displaying and interacting with the move queue

/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Deadly Rooms of Death.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef MOVEQUEUWIDGET_H
#define MOVEQUEUWIDGET_H

#include <FrontEndLib/FocusWidget.h>
#include "MoveQueue.h"
#include "../DRODLib/GameConstants.h"

// Widget tags for move queue controls (1044-1049 range, used only on GameScreen)
static const UINT TAG_MOVEQUEUE_PANEL = 1044;
static const UINT TAG_QUEUE_PLAY = 1045;
static const UINT TAG_QUEUE_STOP = 1046;
static const UINT TAG_QUEUE_STEP = 1047;
static const UINT TAG_QUEUE_RESET = 1048;
static const UINT TAG_QUEUE_CLEAR = 1049;

// Move pool commands available for queuing
static const int MOVE_POOL_COMMANDS[] = {
	CMD_NW, CMD_N, CMD_NE,
	CMD_W, CMD_WAIT, CMD_E,
	CMD_SW, CMD_S, CMD_SE,
	CMD_C, CMD_CC
};
static const UINT MOVE_POOL_COUNT = 11;

// Display text for each move command
const WCHAR* GetMoveDisplayText(int command);

//******************************************************************************
class CMoveQueueWidget : public CFocusWidget
{
public:
	CMoveQueueWidget(UINT dwSetTagNo, int nSetX, int nSetY, UINT wSetW, UINT wSetH);
	virtual ~CMoveQueueWidget();

	// Queue access
	CMoveQueue* GetMoveQueue() { return &moveQueue; }
	const CMoveQueue* GetMoveQueue() const { return &moveQueue; }

	// State
	bool IsAutoExecuting() const { return moveQueue.IsExecuting(); }
	void SetAutoExecuting(bool bVal);
	
	// Scrolling
	void EnsureCurrentMoveVisible();

	// Painting
	virtual void Paint(bool bUpdateRect = true);

protected:
	virtual void HandleMouseDown(const SDL_MouseButtonEvent &Button);
	virtual void HandleMouseUp(const SDL_MouseButtonEvent &Button);
	virtual void HandleDrag(const SDL_MouseMotionEvent &Motion);
	virtual void HandleMouseWheel(const SDL_MouseWheelEvent &Wheel);
	virtual void HandleKeyDown(const SDL_KeyboardEvent &KeyboardEvent);

private:
	// Area calculations
	void CalcAreas();
	
	// Hit testing
	int GetMovePoolIndexAt(int x, int y) const;
	int GetQueueIndexAt(int x, int y) const;
	int GetButtonAt(int x, int y) const;
	int GetQueueItemButtonAt(int x, int y, int& outQueueIndex) const;
	
	// Drawing helpers
	void DrawMovePool(SDL_Surface* pDestSurface);
	void DrawQueue(SDL_Surface* pDestSurface);
	void DrawControlButtons(SDL_Surface* pDestSurface);
	void DrawMoveIcon(SDL_Surface* pDestSurface, int command, int x, int y, 
		bool bDimmed = false, bool bHighlight = false, UINT repeatCount = 1);
	void DrawButton(SDL_Surface* pDestSurface, const SDL_Rect& rect, 
		const WCHAR* text, bool bPressed);
	void DrawRunningIndicator(SDL_Surface* pDestSurface);
	
	// Context menu
	void ShowContextMenu(int queueIndex, int screenX, int screenY);
	void HideContextMenu();
	void DrawContextMenu(SDL_Surface* pDestSurface);
	void DrawInsertSubmenu(SDL_Surface* pDestSurface);
	int GetContextMenuItemAt(int x, int y) const;
	int GetInsertSubmenuItemAt(int x, int y) const;
	void HandleContextMenuClick(int itemIndex);
	void HandleInsertSubmenuClick(int commandIndex, bool bInsertAfter);
	
	// Scrolling
	void ScrollQueueUp();
	void ScrollQueueDown();
	
	// Data
	CMoveQueue moveQueue;
	
	// Layout rects
	SDL_Rect movePoolRect;
	SDL_Rect queueRect;
	SDL_Rect controlsRect;
	SDL_Rect playButtonRect;
	SDL_Rect stopButtonRect;
	SDL_Rect stepButtonRect;
	SDL_Rect resetButtonRect;
	SDL_Rect clearButtonRect;
	
	// Scroll state
	UINT wTopQueueIndex;
	UINT wVisibleQueueItems;
	
	// Drag state
	bool bDragging;
	int nDragCommand;        // Command being dragged from pool
	int nDragQueueIndex;     // Queue index being dragged for reorder
	int nDragX, nDragY;      // Current drag position
	int nDropTargetIndex;    // Where item would be dropped
	
	// Button state
	int nPressedButton;      // Which button is currently pressed (-1 = none)
	
	// Context menu state
	int nContextMenuIndex;   // Queue index for context menu (-1 = none)
	bool bContextMenuVisible;
	int nContextMenuX, nContextMenuY;
	SDL_Rect contextMenuRect;
	
	// Insert submenu state
	bool bInsertSubmenuVisible;
	bool bInsertAfter;       // true = Insert After, false = Insert Before
	SDL_Rect insertSubmenuRect;

	// Context menu item indices
	enum ContextMenuItem {
		CMI_DELETE = 0,
		CMI_SET_REPEAT,
		CMI_INSERT_BEFORE,
		CMI_INSERT_AFTER,
		CMI_COUNT
	};
};

#endif // MOVEQUEUWIDGET_H

