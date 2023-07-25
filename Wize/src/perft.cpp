#include "defs.h"
#include "stdio.h"

long leafNodes;

// Perft function
void Perft(int depth, S_BOARD* pos) {

	// Valid board
	ASSERT(CheckBoard(pos));

	// At terminal node
	if (depth == 0) {
		++leafNodes;
		return;
	}

	S_MOVELIST list[1];
	GenerateAllMoves(pos, list);

	for (int MoveNum = 0; MoveNum < list->count; ++MoveNum) {
		if (!MakeMove(pos, list->moves[MoveNum].move)) {
			continue;
		}
		Perft(depth - 1, pos);
		TakeMove(pos);
	}
	return;
}


// Perft function for testing
void PerfTest(int depth, S_BOARD* pos) {

	// Valid board
	ASSERT(CheckBoard(pos));

	PrintBoard(pos);
	printf("\nStarting perft to Depth:%d\n", depth);

	leafNodes = 0;
	long start = GetTimeMs();

	S_MOVELIST list[1];
	GenerateAllMoves(pos, list);

	int move;
	int MoveNum;
	for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
		move = list->moves[MoveNum].move;
		if (!MakeMove(pos, move)) {
			continue;
		}
		long cumnodes = leafNodes;
		Perft(depth - 1, pos);
		TakeMove(pos);
		long oldnodes = leafNodes - cumnodes;
		printf("move %d : %s : %ld\n", MoveNum + 1, PrMove(move), oldnodes);

	}

	printf("\nPerft Complete : %ld leaf nodes visited in %dms.\n", leafNodes, GetTimeMs() - start);

	return;
}

