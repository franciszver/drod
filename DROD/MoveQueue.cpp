// MoveQueue.cpp
// Implementation of Move Queue data structure

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

#include "MoveQueue.h"
#include <BackEndLib/Assert.h>

//******************************************************************************
CMoveQueue::CMoveQueue()
	: executionIndex(0)
	, currentRepeatIndex(0)
	, bExecuting(false)
{
}

//******************************************************************************
CMoveQueue::~CMoveQueue()
{
}

//******************************************************************************
void CMoveQueue::AddMove(int command)
{
	moves.push_back(QueuedMove(command, 1));
}

//******************************************************************************
void CMoveQueue::InsertMove(size_t index, int command)
{
	if (index > moves.size())
		index = moves.size();
	moves.insert(moves.begin() + index, QueuedMove(command, 1));
	
	// Adjust execution index if inserting before current position
	if (index <= executionIndex && executionIndex < moves.size())
		++executionIndex;
}

//******************************************************************************
void CMoveQueue::RemoveMove(size_t index)
{
	if (index >= moves.size())
		return;
	
	moves.erase(moves.begin() + index);
	
	// Adjust execution index
	if (index < executionIndex)
		--executionIndex;
	else if (index == executionIndex)
		currentRepeatIndex = 0;
	
	// Clamp execution index
	if (executionIndex > moves.size())
		executionIndex = moves.size();
}

//******************************************************************************
void CMoveQueue::MoveItem(size_t fromIndex, size_t toIndex)
{
	if (fromIndex >= moves.size() || toIndex >= moves.size())
		return;
	if (fromIndex == toIndex)
		return;

	QueuedMove move = moves[fromIndex];
	moves.erase(moves.begin() + fromIndex);
	moves.insert(moves.begin() + toIndex, move);
	
	// Adjust execution index
	if (fromIndex == executionIndex)
		executionIndex = toIndex;
	else if (fromIndex < executionIndex && toIndex >= executionIndex)
		--executionIndex;
	else if (fromIndex > executionIndex && toIndex <= executionIndex)
		++executionIndex;
}

//******************************************************************************
void CMoveQueue::SetRepeatCount(size_t index, UINT count)
{
	if (index >= moves.size())
		return;
	
	if (count < 1)
		count = 1;
	if (count > MAX_REPEAT_COUNT)
		count = MAX_REPEAT_COUNT;
	
	moves[index].repeatCount = count;
	
	// Reset repeat index if we're on this move and it's higher than new count
	if (index == executionIndex && currentRepeatIndex >= count)
		currentRepeatIndex = 0;
}

//******************************************************************************
void CMoveQueue::Clear()
{
	moves.clear();
	executionIndex = 0;
	currentRepeatIndex = 0;
	bExecuting = false;
}

//******************************************************************************
void CMoveQueue::ResetExecution()
{
	executionIndex = 0;
	currentRepeatIndex = 0;
	bExecuting = false;
	
	// Mark all moves as not executed
	for (size_t i = 0; i < moves.size(); ++i)
		moves[i].executed = false;
}

//******************************************************************************
int CMoveQueue::GetNextMove()
{
	if (!bExecuting || executionIndex >= moves.size())
		return 0; // CMD_UNSPECIFIED
	
	QueuedMove& move = moves[executionIndex];
	int command = move.command;
	
	// Advance repeat index
	++currentRepeatIndex;
	
	// Check if we've completed all repeats for this move
	if (currentRepeatIndex >= move.repeatCount)
	{
		move.executed = true;
		currentRepeatIndex = 0;
		++executionIndex;
	}
	
	return command;
}

//******************************************************************************
bool CMoveQueue::HasMoreMoves() const
{
	if (executionIndex >= moves.size())
		return false;
	
	// Check if current move has remaining repeats
	if (executionIndex < moves.size())
	{
		const QueuedMove& move = moves[executionIndex];
		if (currentRepeatIndex < move.repeatCount)
			return true;
	}
	
	return false;
}

//******************************************************************************
void CMoveQueue::StartExecution()
{
	bExecuting = true;
}

//******************************************************************************
void CMoveQueue::StopExecution()
{
	bExecuting = false;
}

//******************************************************************************
const QueuedMove& CMoveQueue::GetMove(size_t index) const
{
	ASSERT(index < moves.size());
	return moves[index];
}

