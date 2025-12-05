// MoveQueue.h
// Move Queue data structure for queuing and executing player moves

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

#ifndef MOVEQUEUE_H
#define MOVEQUEUE_H

#include <BackEndLib/Types.h>
#include <vector>

//Maximum repeat count for a single move
static const UINT MAX_REPEAT_COUNT = 99;

//******************************************************************************
struct QueuedMove {
	int command;          // CMD_N, CMD_S, etc. from GameConstants.h
	UINT repeatCount;     // Number of times to repeat (1 = single)
	bool executed;        // Whether this move has been executed

	QueuedMove(int cmd = 0, UINT repeat = 1)
		: command(cmd), repeatCount(repeat), executed(false) {}
};

//******************************************************************************
class CMoveQueue {
public:
	CMoveQueue();
	~CMoveQueue();

	// Queue management
	void AddMove(int command);
	void InsertMove(size_t index, int command);
	void RemoveMove(size_t index);
	void MoveItem(size_t fromIndex, size_t toIndex);
	void SetRepeatCount(size_t index, UINT count);
	void Clear();
	void ResetExecution();

	// Execution
	int GetNextMove();          // Returns next command to execute, advances position
	bool HasMoreMoves() const;
	bool IsExecuting() const { return bExecuting; }
	void StartExecution();
	void StopExecution();

	// State
	size_t GetCurrentIndex() const { return executionIndex; }
	UINT GetCurrentRepeatIndex() const { return currentRepeatIndex; }
	size_t GetQueueSize() const { return moves.size(); }
	const QueuedMove& GetMove(size_t index) const;
	const std::vector<QueuedMove>& GetMoves() const { return moves; }
	bool IsEmpty() const { return moves.empty(); }

private:
	std::vector<QueuedMove> moves;
	size_t executionIndex;
	UINT currentRepeatIndex;
	bool bExecuting;
};

#endif // MOVEQUEUE_H

