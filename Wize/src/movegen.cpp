#include "defs.h"

// Define Macros
#define MOVE(f, t, ca, pro, fl) ( (f) | ((t) << 7) | ( (ca) << 14 ) | ( (pro) << 20 ) | (fl)) // Putting together a move
#define SQOFFBOARD(sq) (FilesBrd[(sq)]==OFFBOARD) // Is a square off board


// Array for sliding Pieces (rooks, bishops, queens)
const int LoopSlidePce[8] = { wB, wR, wQ, 0, bB, bR, bQ, 0 };

// Array for Non sliding Pieces (knights, kings)
const int LoopNonSlidePce[6] = { wN, wK, 0, bN, bK, 0 };

// Array for start index based on side
const int LoopSlideIndex[2] = { 0, 4 };
const int LoopNonSlideIndex[2] = { 0, 3 };

// Directions for pieces, indexed by piece
const int PceDir[13][8] = {
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ -8, -19,	-21, -12, 8, 19, 21, 12 },
	{ -9, -11, 11, 9, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ -8, -19,	-21, -12, 8, 19, 21, 12 },
	{ -9, -11, 11, 9, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 }
};

// Array to match piece type to number of directions
const int NumDir[13] = { 0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8 };


/*
PV move
cap -> MvvLVA  Most value victim least valuable attacker
LVA: P, N, B, R, Q
MVV: Q, R, B, N, P

Killers
History score
*/

const int VictimScore[13] = { 0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 }; // Victim scores

// MVV LVA [attacker][victim]
const int MvvLvaScores[12][12] = {
	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};

//
//// Initialization MvvLva
//void InitMvvLva() {
//	
//	int Attacker;
//	int Victim;
//	for (Attacker = wP; Attacker <= bK; ++Attacker) {
//		for (Victim = wP; Victim <= bK; ++Victim) {
//			MvvLvaScores[Victim][Attacker] = VictimScore[Victim] + 6 - (VictimScore[Attacker] / 100);
//		}
//	}
//}


// Check for if a move exists
int MoveExists(S_BOARD* pos, const int move) {
	
	S_MOVELIST list[1];
	GenerateAllMoves(pos, list);

	for (int MoveNum = 0; MoveNum < list->count; ++MoveNum) {
		
		if (!MakeMove(pos, list->moves[MoveNum].move)) {
			continue;
		}

		TakeMove(pos);
		if (list->moves[MoveNum].move == move) {
			return TRUE;
		}
	}
	
	return FALSE;
}


// Quiet move
static void AddQuietMove(const S_BOARD* pos, int move, S_MOVELIST* list) {

	// Valid squares
	ASSERT(SqOnBoard(FROMSQ(move)));
	ASSERT(SqOnBoard(TOSQ(move)));

	list->moves[list->count].move = move;
	
	// Score killer moves
	if (pos->searchKillers[0][pos->ply] == move) {
		list->moves[list->count].score = 900'000;
	}
	else if (pos->searchKillers[1][pos->ply] == move) {
		list->moves[list->count].score = 800'000;
	}
	else {
		list->moves[list->count].score = pos->searchHistory[pos->pieces[FROMSQ(move)]][TOSQ(move)];
	}

	++list->count; // Increment list
}

// Capture move
static void AddCaptureMove(const S_BOARD* pos, int move, S_MOVELIST* list) {

	// Valid squares and capture
	ASSERT(SqOnBoard(FROMSQ(move)));
	ASSERT(SqOnBoard(TOSQ(move)));
	ASSERT(PieceValid(CAPTURED(move)));

	list->moves[list->count].move = move;
	list->moves[list->count].score = MvvLvaScores[CAPTURED(move)][pos->pieces[FROMSQ(move)]] + 1'000'000;
	++list->count; // Increment list
}

// En Passant move
static void AddEnPassantMove(const S_BOARD* pos, int move, S_MOVELIST* list) {

	// Valid squares
	ASSERT(SqOnBoard(FROMSQ(move)));
	ASSERT(SqOnBoard(TOSQ(move)));

	list->moves[list->count].move = move;
	list->moves[list->count].score = 105 + 1000000;
	++list->count; // Increment list
}

// White pawn capture move, check for promote
static void AddWhitePawnCapMove(const S_BOARD* pos, const int from, const int to, const int cap, S_MOVELIST* list) {
	// Validate move
	ASSERT(PieceValidEmpty(cap));
	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));

	if (RanksBrd[from] == RANK_7) {
		AddCaptureMove(pos, MOVE(from, to, cap, wQ, 0), list);
		AddCaptureMove(pos, MOVE(from, to, cap, wR, 0), list);
		AddCaptureMove(pos, MOVE(from, to, cap, wB, 0), list);
		AddCaptureMove(pos, MOVE(from, to, cap, wN, 0), list);
	}
	else {
		AddCaptureMove(pos, MOVE(from, to, cap, EMPTY, 0), list);
	}
}

// Black pawn capture move, check for promote
static void AddBlackPawnCapMove(const S_BOARD* pos, const int from, const int to, const int cap, S_MOVELIST* list) {
	// Validate move
	ASSERT(PieceValidEmpty(cap));
	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));

	if (RanksBrd[from] == RANK_2) {
		AddCaptureMove(pos, MOVE(from, to, cap, bQ, 0), list);
		AddCaptureMove(pos, MOVE(from, to, cap, bR, 0), list);
		AddCaptureMove(pos, MOVE(from, to, cap, bB, 0), list);
		AddCaptureMove(pos, MOVE(from, to, cap, bN, 0), list);
	}
	else {
		AddCaptureMove(pos, MOVE(from, to, cap, EMPTY, 0), list);
	}
}

// White pawn move, check for promote
static void AddWhitePawnMove(const S_BOARD* pos, const int from, const int to, S_MOVELIST* list) {
	// Validate move
	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));

	if (RanksBrd[from] == RANK_7) {
		AddQuietMove(pos, MOVE(from, to, EMPTY, wQ, 0), list);
		AddQuietMove(pos, MOVE(from, to, EMPTY, wR, 0), list);
		AddQuietMove(pos, MOVE(from, to, EMPTY, wB, 0), list);
		AddQuietMove(pos, MOVE(from, to, EMPTY, wN, 0), list);
	}
	else {
		AddQuietMove(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
	}
}

// Black pawn move, check for promote
static void AddBlackPawnMove(const S_BOARD* pos, const int from, const int to, S_MOVELIST* list) {
	// Validate move
	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));

	if (RanksBrd[from] == RANK_2) {
		AddQuietMove(pos, MOVE(from, to, EMPTY, bQ, 0), list);
		AddQuietMove(pos, MOVE(from, to, EMPTY, bR, 0), list);
		AddQuietMove(pos, MOVE(from, to, EMPTY, bB, 0), list);
		AddQuietMove(pos, MOVE(from, to, EMPTY, bN, 0), list);
	}
	else {
		AddQuietMove(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
	}
}

// Generate all legal moves
void GenerateAllMoves(const S_BOARD* pos, S_MOVELIST* list) {

	// Check if board is valid
	ASSERT(CheckBoard(pos));

	list->count = 0;

	// Define Variables
	int pce = EMPTY;
	int side = pos->side;
	int sq = 0; int t_sq = 0;
	int pceNum = 0;
	int dir = 0;
	int index = 0;
	int pceIndex = 0;

	// Generate white pawn moves
	if (side == WHITE) {
		for (pceNum = 0; pceNum < pos->pceNum[wP]; ++pceNum) {
			sq = pos->pList[wP][pceNum];
			ASSERT(SqOnBoard(sq)); // Validate square on board

			// Generate Noncapture moves
			if (pos->pieces[sq + 10] == EMPTY) {
				AddWhitePawnMove(pos, sq, sq + 10, list);
				if (RanksBrd[sq] == RANK_2 && pos->pieces[sq + 20] == EMPTY) {
					AddQuietMove(pos, MOVE(sq, (sq + 20), EMPTY, EMPTY, MFLAGPS), list);
				}
			}

			// Generate Capture move
			if (!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq + 9, pos->pieces[sq + 9], list);
			}
			if (!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq + 11, pos->pieces[sq + 11], list);
			}

			// Generate en Passant captures
			if (pos->enPas != NO_SQ) {
				if (sq + 9 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq, sq + 9, EMPTY, EMPTY, MFLAGEP), list);
				}
				else if (sq + 11 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq, sq + 11, EMPTY, EMPTY, MFLAGEP), list);
				}
			}
		}

		// Generate White Kingside Castling
		if (pos->castlePerm & WKCA) {
			if (pos->pieces[F1] == EMPTY && pos->pieces[G1] == EMPTY) {
				if (!SqAttacked(E1, BLACK, pos) && !SqAttacked(F1, BLACK, pos)) {
					AddQuietMove(pos, MOVE(E1, G1, EMPTY, EMPTY, MFLAGCA), list);
				}
			}
		}

		// Generate White Queenside Castling
		if (pos->castlePerm & WQCA) {
			if (pos->pieces[D1] == EMPTY && pos->pieces[C1] == EMPTY && pos->pieces[B1] == EMPTY) {
				if (!SqAttacked(E1, BLACK, pos) && !SqAttacked(D1, BLACK, pos)) {
					AddQuietMove(pos, MOVE(E1, C1, EMPTY, EMPTY, MFLAGCA), list);
				}
			}
		}
	}
	else {
		for (pceNum = 0; pceNum < pos->pceNum[bP]; ++pceNum) {
			sq = pos->pList[bP][pceNum];
			ASSERT(SqOnBoard(sq)); // Validate square on board

			// Generate Noncapture moves
			if (pos->pieces[sq - 10] == EMPTY) {
				AddBlackPawnMove(pos, sq, sq - 10, list);
				if (RanksBrd[sq] == RANK_7 && pos->pieces[sq - 20] == EMPTY) {
					AddQuietMove(pos, MOVE(sq, (sq - 20), EMPTY, EMPTY, MFLAGPS), list);
				}
			}

			// Generate Capture move
			if (!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq - 9, pos->pieces[sq - 9], list);
			}
			if (!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq - 11, pos->pieces[sq - 11], list);
			}

			// Generate en Passant captures
			if (pos->enPas != NO_SQ) {
				if (sq - 9 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq, sq - 9, EMPTY, EMPTY, MFLAGEP), list);
				}
				else if (sq - 11 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq, sq - 11, EMPTY, EMPTY, MFLAGEP), list);
				}
			}
		}

		// Generate Black Kingside Castling
		if (pos->castlePerm & BKCA) {
			if (pos->pieces[F8] == EMPTY && pos->pieces[G8] == EMPTY) {
				if (!SqAttacked(E8, WHITE, pos) && !SqAttacked(F8, WHITE, pos)) {
					AddQuietMove(pos, MOVE(E8, G8, EMPTY, EMPTY, MFLAGCA), list);
				}
			}
		}

		// Generate Black Queenside Castling
		if (pos->castlePerm & BQCA) {
			if (pos->pieces[D8] == EMPTY && pos->pieces[C8] == EMPTY && pos->pieces[B8] == EMPTY) {
				if (!SqAttacked(E8, WHITE, pos) && !SqAttacked(D8, WHITE, pos)) {
					AddQuietMove(pos, MOVE(E8, C8, EMPTY, EMPTY, MFLAGCA), list);
				}
			}
		}
	}

	// Sliding Pieces
	pceIndex = LoopSlideIndex[side];
	pce = LoopSlidePce[pceIndex++];

	while (pce != 0) {
		// Valid piece
		ASSERT(PieceValid(pce));

		for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			sq = pos->pList[pce][pceNum];
			ASSERT(SqOnBoard(sq)); // Validate square on board

			// Loop through all directions
			for (index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;
				
				// Iterate until off the board or on a piece
				while (!SQOFFBOARD(t_sq)) {
					// BLACK ^ 1 == WHITE, WHITE ^ 1 == BLACK
					// Check if square not empty
					if (pos->pieces[t_sq] != EMPTY) {
						// Check if color of piece is opposite
						if (PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
							AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
						}
						break;
					}
					AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
					t_sq += dir;
				}
			}
		}

		pce = LoopSlidePce[pceIndex++];
	}

	// Non sliding pieces
	pceIndex = LoopNonSlideIndex[side];
	pce = LoopNonSlidePce[pceIndex++];

	while (pce != 0) {
		// Valid piece
		ASSERT(PieceValid(pce));

		for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			sq = pos->pList[pce][pceNum];
			ASSERT(SqOnBoard(sq)); // Validate square on board

			// loop through all directions
			for (index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				// Check if square offboard
				if (SQOFFBOARD(t_sq)) {
					continue;
				}

				// BLACK ^ 1 == WHITE, WHITE ^ 1 == BLACK
				// Check if square not empty
				if (pos->pieces[t_sq] != EMPTY) { 	
					// Check if color of piece is opposite
					if (PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
						AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
					}
					continue;
				}
				AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
			}
		}

		pce = LoopNonSlidePce[pceIndex++];
	}
}


// Generate all capture moves
void GenerateAllCaptures(const S_BOARD* pos, S_MOVELIST* list) {

	// Check if board is valid
	ASSERT(CheckBoard(pos));

	list->count = 0;

	// Define Variables
	int pce = EMPTY;
	int side = pos->side;
	int sq = 0; int t_sq = 0;
	int pceNum = 0;
	int dir = 0;
	int index = 0;
	int pceIndex = 0;

	// Generate white pawn moves
	if (side == WHITE) {
		for (pceNum = 0; pceNum < pos->pceNum[wP]; ++pceNum) {
			sq = pos->pList[wP][pceNum];
			ASSERT(SqOnBoard(sq)); // Validate square on board

			// Generate Capture move
			if (!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq + 9, pos->pieces[sq + 9], list);
			}
			if (!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq + 11, pos->pieces[sq + 11], list);
			}

			// Generate en Passant captures
			if (pos->enPas != NO_SQ) {
				if (sq + 9 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq, sq + 9, EMPTY, EMPTY, MFLAGEP), list);
				}
				else if (sq + 11 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq, sq + 11, EMPTY, EMPTY, MFLAGEP), list);
				}
			}
		}
	}
	else {
		for (pceNum = 0; pceNum < pos->pceNum[bP]; ++pceNum) {
			sq = pos->pList[bP][pceNum];
			ASSERT(SqOnBoard(sq)); // Validate square on board

			// Generate Capture move
			if (!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq - 9, pos->pieces[sq - 9], list);
			}
			if (!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq - 11, pos->pieces[sq - 11], list);
			}

			// Generate en Passant captures
			if (pos->enPas != NO_SQ) {
				if (sq - 9 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq, sq - 9, EMPTY, EMPTY, MFLAGEP), list);
				}
				else if (sq - 11 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq, sq - 11, EMPTY, EMPTY, MFLAGEP), list);
				}
			}
		}
	}

	// Sliding Pieces
	pceIndex = LoopSlideIndex[side];
	pce = LoopSlidePce[pceIndex++];

	while (pce != 0) {
		// Valid piece
		ASSERT(PieceValid(pce));

		for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			sq = pos->pList[pce][pceNum];
			ASSERT(SqOnBoard(sq)); // Validate square on board

			// Loop through all directions
			for (index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				// Iterate until off the board or on a piece
				while (!SQOFFBOARD(t_sq)) {
					// BLACK ^ 1 == WHITE, WHITE ^ 1 == BLACK
					// Check if square not empty
					if (pos->pieces[t_sq] != EMPTY) {
						// Check if color of piece is opposite
						if (PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
							AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
						}
						break;
					}
					t_sq += dir;
				}
			}
		}

		pce = LoopSlidePce[pceIndex++];
	}

	// Non sliding pieces
	pceIndex = LoopNonSlideIndex[side];
	pce = LoopNonSlidePce[pceIndex++];

	while (pce != 0) {
		// Valid piece
		ASSERT(PieceValid(pce));

		for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			sq = pos->pList[pce][pceNum];
			ASSERT(SqOnBoard(sq)); // Validate square on board

			// loop through all directions
			for (index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				// Check if square offboard
				if (SQOFFBOARD(t_sq)) {
					continue;
				}

				// BLACK ^ 1 == WHITE, WHITE ^ 1 == BLACK
				// Check if square not empty
				if (pos->pieces[t_sq] != EMPTY) {
					// Check if color of piece is opposite
					if (PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
						AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
					}
					continue;
				}
			}
		}

		pce = LoopNonSlidePce[pceIndex++];
	}
}