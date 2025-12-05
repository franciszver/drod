// MoveQueueWidget.cpp
// Implementation of Move Queue Widget

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

#include "MoveQueueWidget.h"
#include "DrodFontManager.h"
#include "DrodBitmapManager.h"

#include <FrontEndLib/FontManager.h>
#include <FrontEndLib/Colors.h>

#include <BackEndLib/Assert.h>

// Layout constants
static const UINT CX_MOVE_ICON = 36;
static const UINT CY_MOVE_ICON = 28;
static const UINT CX_MOVE_POOL_COLS = 3;
static const UINT CY_MOVE_POOL_ROWS = 4;
static const UINT CX_BUTTON = 28;
static const UINT CY_BUTTON = 22;
static const UINT CY_BUTTON_SPACING = 2;
static const UINT CY_SECTION_SPACING = 4;
static const UINT CY_QUEUE_ITEM = 24;

// Colors
static const SURFACECOLOR MovePoolBgColor = {40, 40, 50};
static const SURFACECOLOR QueueBgColor = {30, 30, 40};
static const SURFACECOLOR ButtonBgColor = {60, 60, 70};
static const SURFACECOLOR ButtonPressedColor = {40, 40, 50};
static const SURFACECOLOR HighlightColor = {100, 100, 150};
static const SURFACECOLOR DimColor = {80, 80, 80};
static const SURFACECOLOR RunningBorderColor = {50, 200, 50};
static const SURFACECOLOR CurrentMoveColor = {80, 120, 180};

//******************************************************************************
const WCHAR* GetMoveDisplayText(int command)
{
	static const WCHAR wszNW[] = {We('N'),We('W'),We(0)};
	static const WCHAR wszN[] = {We('N'),We(0)};
	static const WCHAR wszNE[] = {We('N'),We('E'),We(0)};
	static const WCHAR wszW[] = {We('W'),We(0)};
	static const WCHAR wszWait[] = {We('.'),We(0)};
	static const WCHAR wszE[] = {We('E'),We(0)};
	static const WCHAR wszSW[] = {We('S'),We('W'),We(0)};
	static const WCHAR wszS[] = {We('S'),We(0)};
	static const WCHAR wszSE[] = {We('S'),We('E'),We(0)};
	static const WCHAR wszCW[] = {We('C'),We('W'),We(0)};
	static const WCHAR wszCC[] = {We('C'),We('C'),We(0)};
	static const WCHAR wszUnknown[] = {We('?'),We(0)};

	switch (command)
	{
		case CMD_NW: return wszNW;
		case CMD_N: return wszN;
		case CMD_NE: return wszNE;
		case CMD_W: return wszW;
		case CMD_WAIT: return wszWait;
		case CMD_E: return wszE;
		case CMD_SW: return wszSW;
		case CMD_S: return wszS;
		case CMD_SE: return wszSE;
		case CMD_C: return wszCW;
		case CMD_CC: return wszCC;
		default: return wszUnknown;
	}
}

//******************************************************************************
// Context menu constants
static const UINT CX_CONTEXT_MENU = 100;
static const UINT CY_CONTEXT_MENU_ITEM = 20;
static const UINT CX_INSERT_SUBMENU = 50;
static const SURFACECOLOR ContextMenuBgColor = {50, 50, 60};
static const SURFACECOLOR ContextMenuHoverColor = {70, 70, 90};

CMoveQueueWidget::CMoveQueueWidget(
	UINT dwSetTagNo, int nSetX, int nSetY, UINT wSetW, UINT wSetH)
	: CFocusWidget(WT_Unspecified, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
	, wTopQueueIndex(0)
	, wVisibleQueueItems(0)
	, bDragging(false)
	, nDragCommand(0)
	, nDragQueueIndex(-1)
	, nDragX(0), nDragY(0)
	, nDropTargetIndex(-1)
	, nPressedButton(-1)
	, nContextMenuIndex(-1)
	, bContextMenuVisible(false)
	, nContextMenuX(0), nContextMenuY(0)
	, bInsertSubmenuVisible(false)
	, bInsertAfter(false)
{
	CalcAreas();
	CLEAR_RECT(contextMenuRect);
	CLEAR_RECT(insertSubmenuRect);
}

//******************************************************************************
CMoveQueueWidget::~CMoveQueueWidget()
{
}

//******************************************************************************
void CMoveQueueWidget::SetAutoExecuting(bool bVal)
{
	if (bVal)
		moveQueue.StartExecution();
	else
		moveQueue.StopExecution();
}

//******************************************************************************
void CMoveQueueWidget::CalcAreas()
{
	// Move pool at top: 3 columns x 4 rows
	movePoolRect.x = this->x + 2;
	movePoolRect.y = this->y + 2;
	movePoolRect.w = CX_MOVE_ICON * CX_MOVE_POOL_COLS;
	movePoolRect.h = CY_MOVE_ICON * CY_MOVE_POOL_ROWS;

	// Control buttons below move pool
	controlsRect.x = this->x + 2;
	controlsRect.y = movePoolRect.y + movePoolRect.h + CY_SECTION_SPACING;
	controlsRect.w = this->w - 4;
	controlsRect.h = CY_BUTTON;

	// Calculate individual button positions (5 buttons in a row)
	int buttonX = controlsRect.x;
	playButtonRect = {buttonX, controlsRect.y, CX_BUTTON, CY_BUTTON};
	buttonX += CX_BUTTON + CY_BUTTON_SPACING;
	stopButtonRect = {buttonX, controlsRect.y, CX_BUTTON, CY_BUTTON};
	buttonX += CX_BUTTON + CY_BUTTON_SPACING;
	stepButtonRect = {buttonX, controlsRect.y, CX_BUTTON, CY_BUTTON};
	buttonX += CX_BUTTON + CY_BUTTON_SPACING;
	resetButtonRect = {buttonX, controlsRect.y, CX_BUTTON, CY_BUTTON};
	buttonX += CX_BUTTON + CY_BUTTON_SPACING;
	clearButtonRect = {buttonX, controlsRect.y, CX_BUTTON, CY_BUTTON};

	// Queue display below controls
	queueRect.x = this->x + 2;
	queueRect.y = controlsRect.y + controlsRect.h + CY_SECTION_SPACING;
	queueRect.w = this->w - 4;
	queueRect.h = (this->y + this->h) - queueRect.y - 2;

	// Calculate visible queue items
	wVisibleQueueItems = queueRect.h / CY_QUEUE_ITEM;
}

//******************************************************************************
void CMoveQueueWidget::Paint(bool bUpdateRect)
{
	SDL_Surface* pDestSurface = GetDestSurface();

	// Draw panel background
	SURFACECOLOR bgColor = {25, 25, 35};
	SDL_Rect panelRect = {this->x, this->y, this->w, this->h};
	DrawFilledRect(panelRect, bgColor, pDestSurface);

	// Draw border (green if running, normal otherwise)
	if (moveQueue.IsExecuting())
		DrawRect(panelRect, RunningBorderColor, pDestSurface);
	else
	{
		SURFACECOLOR borderColor = {60, 60, 70};
		DrawRect(panelRect, borderColor, pDestSurface);
	}

	DrawMovePool(pDestSurface);
	DrawControlButtons(pDestSurface);
	DrawQueue(pDestSurface);

	if (moveQueue.IsExecuting())
		DrawRunningIndicator(pDestSurface);

	// Draw drag ghost if dragging
	if (bDragging && nDragCommand != 0)
	{
		DrawMoveIcon(pDestSurface, nDragCommand, 
			nDragX - CX_MOVE_ICON/2, nDragY - CY_MOVE_ICON/2, 
			false, true);
	}

	// Draw context menu if visible
	if (bContextMenuVisible)
	{
		DrawContextMenu(pDestSurface);
		if (bInsertSubmenuVisible)
			DrawInsertSubmenu(pDestSurface);
	}

	if (bUpdateRect)
		UpdateRect();
}

//******************************************************************************
void CMoveQueueWidget::DrawMovePool(SDL_Surface* pDestSurface)
{
	// Background
	DrawFilledRect(movePoolRect, MovePoolBgColor, pDestSurface);

	// Draw each move icon
	for (UINT i = 0; i < MOVE_POOL_COUNT; ++i)
	{
		int col = i % CX_MOVE_POOL_COLS;
		int row = i / CX_MOVE_POOL_COLS;
		int iconX = movePoolRect.x + col * CX_MOVE_ICON;
		int iconY = movePoolRect.y + row * CY_MOVE_ICON;

		DrawMoveIcon(pDestSurface, MOVE_POOL_COMMANDS[i], iconX, iconY);
	}
}

//******************************************************************************
void CMoveQueueWidget::DrawQueue(SDL_Surface* pDestSurface)
{
	// Background
	DrawFilledRect(queueRect, QueueBgColor, pDestSurface);

	const std::vector<QueuedMove>& moves = moveQueue.GetMoves();
	size_t currentIndex = moveQueue.GetCurrentIndex();

	UINT drawY = queueRect.y;
	for (UINT i = wTopQueueIndex; i < moves.size() && i < wTopQueueIndex + wVisibleQueueItems; ++i)
	{
		const QueuedMove& move = moves[i];
		
		// Highlight current execution position
		bool bHighlight = (i == currentIndex && moveQueue.IsExecuting());
		bool bDimmed = move.executed;

		// Draw background for current move
		if (bHighlight)
		{
			SDL_Rect itemRect = {queueRect.x, (int)drawY, queueRect.w, CY_QUEUE_ITEM};
			DrawFilledRect(itemRect, CurrentMoveColor, pDestSurface);
		}

		// Draw drop target indicator
		if (bDragging && (int)i == nDropTargetIndex)
		{
			SDL_Rect lineRect = {queueRect.x, (int)drawY, queueRect.w, 2};
			DrawFilledRect(lineRect, HighlightColor, pDestSurface);
		}

		// Draw move icon
		DrawMoveIcon(pDestSurface, move.command, 
			queueRect.x + 2, drawY + (CY_QUEUE_ITEM - CY_MOVE_ICON) / 2,
			bDimmed, bHighlight, move.repeatCount);

		drawY += CY_QUEUE_ITEM;
	}

	// Draw scroll indicators if needed
	if (wTopQueueIndex > 0)
	{
		// Draw up arrow indicator
		static const WCHAR wszUp[] = {We('^'),We(0)};
		g_pTheFM->DrawTextXY(wszUp, pDestSurface, 
			queueRect.x + queueRect.w - 15, queueRect.y + 2, F_Small);
	}
	if (wTopQueueIndex + wVisibleQueueItems < moves.size())
	{
		// Draw down arrow indicator
		static const WCHAR wszDown[] = {We('v'),We(0)};
		g_pTheFM->DrawTextXY(wszDown, pDestSurface,
			queueRect.x + queueRect.w - 15, queueRect.y + queueRect.h - 15, F_Small);
	}
}

//******************************************************************************
void CMoveQueueWidget::DrawControlButtons(SDL_Surface* pDestSurface)
{
	static const WCHAR wszPlay[] = {We('>'),We(0)};
	static const WCHAR wszStop[] = {We('|'),We('|'),We(0)};
	static const WCHAR wszStep[] = {We('>'),We('|'),We(0)};
	static const WCHAR wszReset[] = {We('<'),We('<'),We(0)};
	static const WCHAR wszClear[] = {We('X'),We(0)};

	DrawButton(pDestSurface, playButtonRect, wszPlay, nPressedButton == TAG_QUEUE_PLAY);
	DrawButton(pDestSurface, stopButtonRect, wszStop, nPressedButton == TAG_QUEUE_STOP);
	DrawButton(pDestSurface, stepButtonRect, wszStep, nPressedButton == TAG_QUEUE_STEP);
	DrawButton(pDestSurface, resetButtonRect, wszReset, nPressedButton == TAG_QUEUE_RESET);
	DrawButton(pDestSurface, clearButtonRect, wszClear, nPressedButton == TAG_QUEUE_CLEAR);
}

//******************************************************************************
void CMoveQueueWidget::DrawMoveIcon(SDL_Surface* pDestSurface, int command,
	int x, int y, bool bDimmed, bool bHighlight, UINT repeatCount)
{
	SDL_Rect iconRect = {x, y, CX_MOVE_ICON, CY_MOVE_ICON};

	// Background
	SURFACECOLOR bgColor = bHighlight ? HighlightColor : 
		(bDimmed ? DimColor : ButtonBgColor);
	DrawFilledRect(iconRect, bgColor, pDestSurface);

	// Border
	SURFACECOLOR borderColor = {80, 80, 90};
	DrawRect(iconRect, borderColor, pDestSurface);

	// Text
	const WCHAR* text = GetMoveDisplayText(command);
	UINT eFont = bDimmed ? F_Small : F_Small;
	g_pTheFM->DrawTextXY(text, pDestSurface, x + 4, y + 4, eFont);

	// Repeat count badge
	if (repeatCount > 1)
	{
		WCHAR wszCount[8];
		_itoW(repeatCount, wszCount, 10);
		g_pTheFM->DrawTextXY(wszCount, pDestSurface, 
			x + CX_MOVE_ICON - 12, y + CY_MOVE_ICON - 12, F_Small);
	}
}

//******************************************************************************
void CMoveQueueWidget::DrawButton(SDL_Surface* pDestSurface, const SDL_Rect& rect,
	const WCHAR* text, bool bPressed)
{
	SURFACECOLOR bgColor = bPressed ? ButtonPressedColor : ButtonBgColor;
	SDL_Rect buttonRect = {rect.x, rect.y, rect.w, rect.h};
	DrawFilledRect(buttonRect, bgColor, pDestSurface);

	SURFACECOLOR borderColor = {80, 80, 90};
	DrawRect(buttonRect, borderColor, pDestSurface);

	// Center text
	UINT textW, textH;
	g_pTheFM->GetTextWidthHeight(F_Small, text, textW, textH);
	int textX = rect.x + (rect.w - textW) / 2;
	int textY = rect.y + (rect.h - textH) / 2;
	g_pTheFM->DrawTextXY(text, pDestSurface, textX, textY, F_Small);
}

//******************************************************************************
void CMoveQueueWidget::DrawRunningIndicator(SDL_Surface* pDestSurface)
{
	// Draw "RUNNING" text at top of panel
	static const WCHAR wszRunning[] = {We('R'),We('U'),We('N'),We(0)};
	g_pTheFM->DrawTextXY(wszRunning, pDestSurface, 
		this->x + this->w - 35, this->y + 2, F_Small);
}

//******************************************************************************
int CMoveQueueWidget::GetMovePoolIndexAt(int x, int y) const
{
	if (!IS_IN_RECT(x, y, movePoolRect))
		return -1;

	int col = (x - movePoolRect.x) / CX_MOVE_ICON;
	int row = (y - movePoolRect.y) / CY_MOVE_ICON;
	int index = row * CX_MOVE_POOL_COLS + col;

	if (index >= 0 && (UINT)index < MOVE_POOL_COUNT)
		return index;
	return -1;
}

//******************************************************************************
int CMoveQueueWidget::GetQueueIndexAt(int x, int y) const
{
	if (!IS_IN_RECT(x, y, queueRect))
		return -1;

	int relY = y - queueRect.y;
	int index = wTopQueueIndex + relY / CY_QUEUE_ITEM;

	if (index >= 0 && (size_t)index < moveQueue.GetQueueSize())
		return index;
	return -1;
}

//******************************************************************************
int CMoveQueueWidget::GetButtonAt(int x, int y) const
{
	if (IS_IN_RECT(x, y, playButtonRect)) return TAG_QUEUE_PLAY;
	if (IS_IN_RECT(x, y, stopButtonRect)) return TAG_QUEUE_STOP;
	if (IS_IN_RECT(x, y, stepButtonRect)) return TAG_QUEUE_STEP;
	if (IS_IN_RECT(x, y, resetButtonRect)) return TAG_QUEUE_RESET;
	if (IS_IN_RECT(x, y, clearButtonRect)) return TAG_QUEUE_CLEAR;
	return -1;
}

//******************************************************************************
void CMoveQueueWidget::HandleMouseDown(const SDL_MouseButtonEvent &Button)
{
	int x = Button.x;
	int y = Button.y;

	// Handle insert submenu click first
	if (bInsertSubmenuVisible && Button.button == SDL_BUTTON_LEFT)
	{
		int cmdIndex = GetInsertSubmenuItemAt(x, y);
		if (cmdIndex >= 0)
		{
			HandleInsertSubmenuClick(cmdIndex, bInsertAfter);
			return;
		}
	}

	// Handle context menu click
	if (bContextMenuVisible && Button.button == SDL_BUTTON_LEFT)
	{
		int itemIndex = GetContextMenuItemAt(x, y);
		if (itemIndex >= 0)
		{
			HandleContextMenuClick(itemIndex);
			return;
		}
		// Click outside menu - close it
		HideContextMenu();
		RequestPaint();
		return;
	}

	// Right-click for context menu
	if (Button.button == SDL_BUTTON_RIGHT)
	{
		HideContextMenu(); // Hide any existing menu first
		int queueIndex = GetQueueIndexAt(x, y);
		if (queueIndex >= 0)
		{
			ShowContextMenu(queueIndex, x, y);
		}
		return;
	}

	// Left-click handling
	if (Button.button != SDL_BUTTON_LEFT)
		return;

	// Check move pool
	int poolIndex = GetMovePoolIndexAt(x, y);
	if (poolIndex >= 0)
	{
		// Start drag from pool
		bDragging = true;
		nDragCommand = MOVE_POOL_COMMANDS[poolIndex];
		nDragQueueIndex = -1;
		nDragX = x;
		nDragY = y;
		return;
	}

	// Check queue
	int queueIndex = GetQueueIndexAt(x, y);
	if (queueIndex >= 0)
	{
		// Start drag for reorder
		bDragging = true;
		nDragCommand = moveQueue.GetMove(queueIndex).command;
		nDragQueueIndex = queueIndex;
		nDragX = x;
		nDragY = y;
		return;
	}

	// Check buttons
	int button = GetButtonAt(x, y);
	if (button >= 0)
	{
		nPressedButton = button;
		RequestPaint();
	}
}

//******************************************************************************
void CMoveQueueWidget::HandleMouseUp(const SDL_MouseButtonEvent &Button)
{
	if (Button.button != SDL_BUTTON_LEFT)
		return;

	int x = Button.x;
	int y = Button.y;

	// Handle drag end
	if (bDragging)
	{
		// Check if dropped in queue area
		if (IS_IN_RECT(x, y, queueRect))
		{
			int dropIndex = GetQueueIndexAt(x, y);
			if (dropIndex < 0)
				dropIndex = moveQueue.GetQueueSize(); // Drop at end

			if (nDragQueueIndex >= 0)
			{
				// Reorder existing item
				moveQueue.MoveItem(nDragQueueIndex, dropIndex);
			}
			else
			{
				// Insert new item from pool
				moveQueue.InsertMove(dropIndex, nDragCommand);
			}
		}

		bDragging = false;
		nDragCommand = 0;
		nDragQueueIndex = -1;
		nDropTargetIndex = -1;
		RequestPaint();
		return;
	}

	// Handle button release
	if (nPressedButton >= 0)
	{
		int button = GetButtonAt(x, y);
		if (button == nPressedButton)
		{
			// Button was clicked
			CEventHandlerWidget* pEventHandler = GetEventHandlerWidget();
			if (pEventHandler)
			{
				// Notify parent of button click
				switch (button)
				{
					case TAG_QUEUE_PLAY:
						moveQueue.StartExecution();
						break;
					case TAG_QUEUE_STOP:
						moveQueue.StopExecution();
						break;
					case TAG_QUEUE_STEP:
						// Step is handled by parent
						break;
					case TAG_QUEUE_RESET:
						moveQueue.ResetExecution();
						break;
					case TAG_QUEUE_CLEAR:
						moveQueue.Clear();
						break;
				}
				pEventHandler->OnClick(button);
			}
		}
		nPressedButton = -1;
		RequestPaint();
	}
}

//******************************************************************************
void CMoveQueueWidget::HandleDrag(const SDL_MouseMotionEvent &Motion)
{
	if (!bDragging)
		return;

	nDragX = Motion.x;
	nDragY = Motion.y;

	// Update drop target
	if (IS_IN_RECT(Motion.x, Motion.y, queueRect))
	{
		nDropTargetIndex = GetQueueIndexAt(Motion.x, Motion.y);
		if (nDropTargetIndex < 0)
			nDropTargetIndex = moveQueue.GetQueueSize();
	}
	else
	{
		nDropTargetIndex = -1;
	}

	RequestPaint();
}

//******************************************************************************
void CMoveQueueWidget::HandleMouseWheel(const SDL_MouseWheelEvent &Wheel)
{
	if (Wheel.y > 0)
		ScrollQueueUp();
	else if (Wheel.y < 0)
		ScrollQueueDown();
}

//******************************************************************************
void CMoveQueueWidget::HandleKeyDown(const SDL_KeyboardEvent &KeyboardEvent)
{
	switch (KeyboardEvent.keysym.sym)
	{
		case SDLK_ESCAPE:
			// Close context menu if open
			if (bContextMenuVisible)
			{
				HideContextMenu();
				RequestPaint();
			}
			break;
		case SDLK_UP:
			ScrollQueueUp();
			break;
		case SDLK_DOWN:
			ScrollQueueDown();
			break;
		case SDLK_DELETE:
			// Delete selected item (if any context menu was open)
			if (nContextMenuIndex >= 0 && !bContextMenuVisible)
			{
				moveQueue.RemoveMove(nContextMenuIndex);
				nContextMenuIndex = -1;
				RequestPaint();
			}
			break;
	}
}

//******************************************************************************
void CMoveQueueWidget::ScrollQueueUp()
{
	if (wTopQueueIndex > 0)
	{
		--wTopQueueIndex;
		RequestPaint();
	}
}

//******************************************************************************
void CMoveQueueWidget::ScrollQueueDown()
{
	if (wTopQueueIndex + wVisibleQueueItems < moveQueue.GetQueueSize())
	{
		++wTopQueueIndex;
		RequestPaint();
	}
}

//******************************************************************************
void CMoveQueueWidget::EnsureCurrentMoveVisible()
{
	size_t currentIndex = moveQueue.GetCurrentIndex();
	if (currentIndex < wTopQueueIndex)
	{
		wTopQueueIndex = currentIndex;
	}
	else if (currentIndex >= wTopQueueIndex + wVisibleQueueItems)
	{
		wTopQueueIndex = currentIndex - wVisibleQueueItems + 1;
	}
}

//******************************************************************************
void CMoveQueueWidget::ShowContextMenu(int queueIndex, int screenX, int screenY)
{
	nContextMenuIndex = queueIndex;
	bContextMenuVisible = true;
	bInsertSubmenuVisible = false;
	nContextMenuX = screenX;
	nContextMenuY = screenY;

	// Calculate context menu rect
	contextMenuRect.x = screenX;
	contextMenuRect.y = screenY;
	contextMenuRect.w = CX_CONTEXT_MENU;
	contextMenuRect.h = CY_CONTEXT_MENU_ITEM * CMI_COUNT;

	// Keep menu on screen
	SDL_Surface* pScreen = GetDestSurface();
	if (pScreen)
	{
		if (contextMenuRect.x + contextMenuRect.w > pScreen->w)
			contextMenuRect.x = pScreen->w - contextMenuRect.w;
		if (contextMenuRect.y + contextMenuRect.h > pScreen->h)
			contextMenuRect.y = pScreen->h - contextMenuRect.h;
	}

	RequestPaint();
}

//******************************************************************************
void CMoveQueueWidget::HideContextMenu()
{
	bContextMenuVisible = false;
	bInsertSubmenuVisible = false;
	nContextMenuIndex = -1;
}

//******************************************************************************
void CMoveQueueWidget::DrawContextMenu(SDL_Surface* pDestSurface)
{
	// Menu text items
	static const WCHAR wszDelete[] = {We('D'),We('e'),We('l'),We('e'),We('t'),We('e'),We(0)};
	static const WCHAR wszSetRepeat[] = {We('R'),We('e'),We('p'),We('e'),We('a'),We('t'),We('.'),We('.'),We('.'),We(0)};
	static const WCHAR wszInsertBefore[] = {We('I'),We('n'),We('s'),We(' '),We('B'),We('e'),We('f'),We('o'),We('r'),We('e'),We(' '),We('>'),We(0)};
	static const WCHAR wszInsertAfter[] = {We('I'),We('n'),We('s'),We(' '),We('A'),We('f'),We('t'),We('e'),We('r'),We(' '),We('>'),We(0)};
	const WCHAR* menuItems[CMI_COUNT] = { wszDelete, wszSetRepeat, wszInsertBefore, wszInsertAfter };

	// Draw menu background
	DrawFilledRect(contextMenuRect, ContextMenuBgColor, pDestSurface);
	
	SURFACECOLOR borderColor = {80, 80, 90};
	DrawRect(contextMenuRect, borderColor, pDestSurface);

	// Draw menu items
	for (int i = 0; i < CMI_COUNT; ++i)
	{
		int itemY = contextMenuRect.y + i * CY_CONTEXT_MENU_ITEM;
		g_pTheFM->DrawTextXY(menuItems[i], pDestSurface, 
			contextMenuRect.x + 4, itemY + 2, F_Small);
	}
}

//******************************************************************************
void CMoveQueueWidget::DrawInsertSubmenu(SDL_Surface* pDestSurface)
{
	// Draw submenu background
	DrawFilledRect(insertSubmenuRect, ContextMenuBgColor, pDestSurface);
	
	SURFACECOLOR borderColor = {80, 80, 90};
	DrawRect(insertSubmenuRect, borderColor, pDestSurface);

	// Draw move options
	for (UINT i = 0; i < MOVE_POOL_COUNT; ++i)
	{
		int itemY = insertSubmenuRect.y + i * CY_CONTEXT_MENU_ITEM;
		const WCHAR* text = GetMoveDisplayText(MOVE_POOL_COMMANDS[i]);
		g_pTheFM->DrawTextXY(text, pDestSurface, 
			insertSubmenuRect.x + 4, itemY + 2, F_Small);
	}
}

//******************************************************************************
int CMoveQueueWidget::GetContextMenuItemAt(int x, int y) const
{
	if (!bContextMenuVisible)
		return -1;
	if (!IS_IN_RECT(x, y, contextMenuRect))
		return -1;

	int relY = y - contextMenuRect.y;
	int itemIndex = relY / CY_CONTEXT_MENU_ITEM;
	if (itemIndex >= 0 && itemIndex < CMI_COUNT)
		return itemIndex;
	return -1;
}

//******************************************************************************
int CMoveQueueWidget::GetInsertSubmenuItemAt(int x, int y) const
{
	if (!bInsertSubmenuVisible)
		return -1;
	if (!IS_IN_RECT(x, y, insertSubmenuRect))
		return -1;

	int relY = y - insertSubmenuRect.y;
	int itemIndex = relY / CY_CONTEXT_MENU_ITEM;
	if (itemIndex >= 0 && (UINT)itemIndex < MOVE_POOL_COUNT)
		return itemIndex;
	return -1;
}

//******************************************************************************
void CMoveQueueWidget::HandleContextMenuClick(int itemIndex)
{
	switch (itemIndex)
	{
		case CMI_DELETE:
			if (nContextMenuIndex >= 0)
				moveQueue.RemoveMove(nContextMenuIndex);
			HideContextMenu();
			break;

		case CMI_SET_REPEAT:
		{
			// Cycle repeat count: 1 -> 2 -> 3 -> 5 -> 10 -> 20 -> 50 -> 99 -> 1
			if (nContextMenuIndex >= 0 && (size_t)nContextMenuIndex < moveQueue.GetQueueSize())
			{
				const QueuedMove& move = moveQueue.GetMove(nContextMenuIndex);
				UINT newCount = move.repeatCount;
				if (newCount < 2) newCount = 2;
				else if (newCount < 3) newCount = 3;
				else if (newCount < 5) newCount = 5;
				else if (newCount < 10) newCount = 10;
				else if (newCount < 20) newCount = 20;
				else if (newCount < 50) newCount = 50;
				else if (newCount < 99) newCount = 99;
				else newCount = 1;
				moveQueue.SetRepeatCount(nContextMenuIndex, newCount);
			}
			HideContextMenu();
			break;
		}

		case CMI_INSERT_BEFORE:
		case CMI_INSERT_AFTER:
		{
			// Show insert submenu
			bInsertAfter = (itemIndex == CMI_INSERT_AFTER);
			bInsertSubmenuVisible = true;

			// Position submenu to the right of the context menu
			insertSubmenuRect.x = contextMenuRect.x + contextMenuRect.w;
			insertSubmenuRect.y = contextMenuRect.y + itemIndex * CY_CONTEXT_MENU_ITEM;
			insertSubmenuRect.w = CX_INSERT_SUBMENU;
			insertSubmenuRect.h = CY_CONTEXT_MENU_ITEM * MOVE_POOL_COUNT;

			// Keep submenu on screen
			SDL_Surface* pScreen = GetDestSurface();
			if (pScreen)
			{
				if (insertSubmenuRect.x + insertSubmenuRect.w > pScreen->w)
					insertSubmenuRect.x = contextMenuRect.x - insertSubmenuRect.w;
				if (insertSubmenuRect.y + insertSubmenuRect.h > pScreen->h)
					insertSubmenuRect.y = pScreen->h - insertSubmenuRect.h;
			}
			break;
		}
	}
	RequestPaint();
}

//******************************************************************************
void CMoveQueueWidget::HandleInsertSubmenuClick(int commandIndex, bool bAfter)
{
	if (nContextMenuIndex >= 0 && commandIndex >= 0 && (UINT)commandIndex < MOVE_POOL_COUNT)
	{
		int insertIndex = bAfter ? nContextMenuIndex + 1 : nContextMenuIndex;
		moveQueue.InsertMove(insertIndex, MOVE_POOL_COMMANDS[commandIndex]);
	}
	HideContextMenu();
	RequestPaint();
}

