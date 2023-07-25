#include "defs.h"

// nnue
#include "nnue_eval.h"


const int IsolatedPawn = -10; // Penalty for isolate pawn
const int DoubledPawns = -10; // Penalty for doubled pawns
const int PassedPawn[8] = { 0, 5, 10, 20, 35, 60, 100, 200 }; // Bonus for passed pawn indexed by rank
const int RookOpenFile = 5; // Bonus for rook on open file
const int RookSemiOpenFile = 5;
const int QueenOpenFile = 5;
const int QueenSemiOpenFile = 3;
const int BishopPair = 30;

// Endgame material for king activation
#define ENDGAME_MAT (1 * PieceVal[wR] + 2 * PieceVal[wN] + 2 * PieceVal[wP] + PieceVal[wK])


/* PIECE SQUARE TABLES in pawns/100 */

const int PawnTable[64] = {
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,
10	,	10	,	-5	,	-10	,	-10	,	-5	,	10	,	10	,
5	,	0	,	0	,	5	,	5	,	0	,	0	,	5	,
0	,	0	,	10	,	20	,	20	,	10	,	0	,	0	,
5	,	5	,	5	,	10	,	10	,	5	,	5	,	5	,
10	,	10	,	10	,	20	,	20	,	10	,	10	,	10	,
20	,	20	,	20	,	30	,	30	,	20	,	20	,	20	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
};

const int KnightTable[64] = {
-10	,	-10	,	0	,	0	,	0	,	0	,	-10	,	-10	,
-5	,	0	,	0	,	5	,	5	,	0	,	0	,	-5	,
-5	,	0	,	10	,	10	,	10	,	10	,	0	,	-5	,
-5	,	0	,	10	,	20	,	20	,	10	,	5	,	-5	,
-5	,	11	,	15	,	20	,	20	,	15	,	11	,	-5	,
5	,	10	,	10	,	20	,	20	,	10	,	10	,	5	,
-3	,	0	,	5	,	10	,	10	,	5	,	0	,	-3	,
-10	,	-5	,	-3	,	-2	,	-2	,	-3	,	-5	,	-10
};

const int BishopTable[64] = {
0	,	0	,	-10	,	0	,	0	,	-10	,	0	,	0	,
0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
};

const int RookTable[64] = {
0	,	0	,	5	,	10	,	11	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0
};

const int KingE[64] = {
	-50	,	-10	,	0	,	0	,	0	,	0	,	-10	,	-50	,
	-10,	0	,	10	,	10	,	10	,	10	,	0	,	-10	,
	0	,	10	,	20	,	20	,	20	,	20	,	10	,	0	,
	0	,	10	,	20	,	30	,	30	,	20	,	10	,	0	,
	0	,	10	,	20	,	30	,	30	,	20	,	10	,	0	,
	0	,	10	,	20	,	20	,	20	,	20	,	10	,	0	,
	-10,	0	,	10	,	10	,	10	,	10	,	0	,	-10	,
	-50	,	-10	,	0	,	0	,	0	,	0	,	-10	,	-50
};

const int KingO[64] = {
	0	,	5	,	5	,	-10	,	-10	,	0	,	10	,	5	,
	-30	,	-30	,	-30	,	-31	,	-31	,	-30	,	-30	,	-30	,
	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70
};


// Draw based on material, assumed best play
int MaterialDraw(const S_BOARD* pos) {

	ASSERT(CheckBoard(pos));

	if (!pos->pceNum[wR] && !pos->pceNum[bR] && !pos->pceNum[wQ] && !pos->pceNum[bQ]) {
		if (!pos->pceNum[bB] && !pos->pceNum[wB]) {
			if (pos->pceNum[wN] < 3 && pos->pceNum[bN] < 3) { return TRUE; }
		}
		else if (!pos->pceNum[wN] && !pos->pceNum[bN]) {
			if (abs(pos->pceNum[wB] - pos->pceNum[bB]) < 2) { return TRUE; }
		}
		else if ((pos->pceNum[wN] < 3 && !pos->pceNum[wB]) || (pos->pceNum[wB] == 1 && !pos->pceNum[wN])) {
			if ((pos->pceNum[bN] < 3 && !pos->pceNum[bB]) || (pos->pceNum[bB] == 1 && !pos->pceNum[bN])) { return TRUE; }
		}
	}
	else if (!pos->pceNum[wQ] && !pos->pceNum[bQ]) {
		if (pos->pceNum[wR] == 1 && pos->pceNum[bR] == 1) {
			if ((pos->pceNum[wN] + pos->pceNum[wB]) < 2 && (pos->pceNum[bN] + pos->pceNum[bB]) < 2) { return TRUE; }
		}
		else if (pos->pceNum[wR] == 1 && !pos->pceNum[bR]) {
			if ((pos->pceNum[wN] + pos->pceNum[wB] == 0) && (((pos->pceNum[bN] + pos->pceNum[bB]) == 1) || ((pos->pceNum[bN] + pos->pceNum[bB]) == 2))) { return TRUE; }
		}
		else if (pos->pceNum[bR] == 1 && !pos->pceNum[wR]) {
			if ((pos->pceNum[bN] + pos->pceNum[bB] == 0) && (((pos->pceNum[wN] + pos->pceNum[wB]) == 1) || ((pos->pceNum[wN] + pos->pceNum[wB]) == 2))) { return TRUE; }
		}
	}
	return FALSE;
}

// Return evaluation score from position
int EvalPositionHand(const S_BOARD* pos) {
	ASSERT(CheckBoard(pos));
	// Define indices
	int pce;
	int pceNum;
	int sq;

	// Material score
	int score = pos->material[WHITE] - pos->material[BLACK];

	// Draw Check
	if (!pos->pceNum[wP] && !pos->pceNum[bP] && MaterialDraw(pos) == TRUE) {
		return 0;
	}

	/* PSQT SCORES */
	
	// Pawns
	pce = wP;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score += PawnTable[SQ64(sq)];

		if ((IsolatedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
			//printf("wP Iso:%s\n",PrSq(sq));
			score += IsolatedPawn;
		}

		if ((DoubledMask[SQ64(sq)] & pos->pawns[WHITE]) != 0) {
			//printf("wP Iso:%s\n",PrSq(sq));
			score += DoubledPawns;
		}

		if ((WhitePassedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
			//printf("wP Passed:%s\n",PrSq(sq));
			score += PassedPawn[RanksBrd[sq]];
		}

	}

	pce = bP;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq)) >= 0 && MIRROR64(SQ64(sq)) <= 63);
		score -= PawnTable[MIRROR64(SQ64(sq))];

		if ((IsolatedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
			//printf("bP Iso:%s\n",PrSq(sq));
			score -= IsolatedPawn;
		}

		if ((DoubledMask[SQ64(sq)] & pos->pawns[BLACK]) != 0) {
			//printf("wP Iso:%s\n",PrSq(sq));
			score -= DoubledPawns;
		}

		if ((BlackPassedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
			//printf("bP Passed:%s\n",PrSq(sq));
			score -= PassedPawn[7 - RanksBrd[sq]];
		}
	}

	// Knights
	pce = wN;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score += KnightTable[SQ64(sq)];
	}

	pce = bN;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq)) >= 0 && MIRROR64(SQ64(sq)) <= 63);
		score -= KnightTable[MIRROR64(SQ64(sq))];
	}

	// Bishops
	pce = wB;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score += BishopTable[SQ64(sq)];
	}

	pce = bB;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq)) >= 0 && MIRROR64(SQ64(sq)) <= 63);
		score -= BishopTable[MIRROR64(SQ64(sq))];
	}

	// Rooks
	pce = wR;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score += RookTable[SQ64(sq)];

		ASSERT(FileRankValid(FilesBrd[sq]));

		if (!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			score += RookOpenFile;
		}
		else if (!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
			score += RookSemiOpenFile;
		}
	}

	pce = bR;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq)) >= 0 && MIRROR64(SQ64(sq)) <= 63);
		score -= RookTable[MIRROR64(SQ64(sq))];
		ASSERT(FileRankValid(FilesBrd[sq]));
		if (!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			score -= RookOpenFile;
		}
		else if (!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			score -= RookSemiOpenFile;
		}
	}

	// Queens
	pce = wQ;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		ASSERT(FileRankValid(FilesBrd[sq]));
		if (!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			score += QueenOpenFile;
		}
		else if (!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
			score += QueenSemiOpenFile;
		}
	}

	pce = bQ;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		ASSERT(FileRankValid(FilesBrd[sq]));
		if (!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			score -= QueenOpenFile;
		}
		else if (!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			score -= QueenSemiOpenFile;
		}
	}

	pce = bQ;
	for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq)); // Valid square

		// Open file
		if (!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			score -= QueenOpenFile; // Bonus for open file
		}
		else if (!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			score -= QueenSemiOpenFile; // Bonus for semi open file
		}
	}

	// Kings, score based on opening vs endgame
	pce = wK;
	sq = pos->pList[pce][0];

	if ((pos->material[BLACK] <= ENDGAME_MAT)) {
		score += KingE[SQ64(sq)];
	}
	else {
		score += KingO[SQ64(sq)];
	}

	pce = bK;
	sq = pos->pList[pce][0];
	if ((pos->material[WHITE] <= ENDGAME_MAT)) {
		score -= KingE[MIRROR64(SQ64(sq))];
	}
	else {
		score -= KingO[MIRROR64(SQ64(sq))];
	}

	// Bishop pair bonus
	if (pos->pceNum[wB] >= 2) score += BishopPair;
	if (pos->pceNum[bB] >= 2) score -= BishopPair;

	// Negate score for negamax search
	if (pos->side == WHITE) {
		return score;
	}
	else {
		return -score;
	}
}


// NNUE piece encoding
int encode_nnue[13] = { 0, 6, 5, 4, 3, 2, 1, 12, 11, 10, 9, 8, 7 };



// Evaluate position with NNUE
int EvalNNUE(const S_BOARD* pos) {

	// NNUE probe arrays
	int pieces[33];
	int squares[33];

	// NNUE probe arrays index
	int index = 2;

	// loops over pieces
	for (int piece = 1; piece < 13; ++piece) {

		// Loop over squares
		for (int pceNum = 0; pceNum < pos->pceNum[piece]; ++pceNum) {

			// White king
			if (piece == wK) {

				// pieces and squares array
				pieces[0] = encode_nnue[piece];
				squares[0] = SQ64(pos->pList[piece][pceNum]);
			}
			else if (piece == bK) {

				// pieces and squares array
				pieces[1] = encode_nnue[piece];
				squares[1] = SQ64(pos->pList[piece][pceNum]);
			}
			else {
				// pieces and squares array
				pieces[index] = encode_nnue[piece];
				squares[index] = SQ64(pos->pList[piece][pceNum]);

				// Increment index
				++index;
			}
		}
	}

	// end square, piece arrays with zero terminating characters
	pieces[index] = 0;
	squares[index] = 0;

	// Penalty for 50 move rule

	return evaluate_nnue(pos->side, pieces, squares) * (100 - pos->fiftyMove) / 100;
}

int EvalPosition(const S_BOARD* pos) {
	if (USENNUE) {
		return EvalNNUE(pos);
	}
	else {
		return EvalPositionHand(pos);
	}
}





