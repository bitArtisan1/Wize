#include "defs.h"

// Hasing Macros
#define HASH_PCE(pce,sq) (pos->posKey ^= (PieceKeys[(pce)][(sq)]))
#define HASH_CA (pos->posKey ^= (CastleKeys[(pos->castlePerm)]))
#define HASH_SIDE (pos->posKey ^= (SideKey))
#define HASH_EP (pos->posKey ^= (PieceKeys[EMPTY][(pos->enPas)]))


// Array of used to compare and update castle rights
const int CastlePerm[120] = {
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15,  7, 15, 15, 15,  3, 15, 15, 11, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};


// Clear a piece from the board
static void ClearPiece(const int sq, S_BOARD* pos) {
	// Valid square on board
	ASSERT(SqOnBoard(sq));
	ASSERT(CheckBoard(pos));

	// Get piece
	int pce = pos->pieces[sq];
	// Valid Piece
	ASSERT(PieceValid(pce));

	// Define indices
	int col = PieceCol[pce];
	int index = 0;
	int t_pceNum = -1;

	HASH_PCE(pce, sq);

	pos->pieces[sq] = EMPTY;
	pos->material[col] -= PieceVal[pce];

	// Adjust piece counters
	if (PieceBig[pce]) {
		--pos->bigPce[col];
		if (PieceMaj[pce]) {
			--pos->majPce[col];
		}
		else {
			--pos->minPce[col];
		}
	}
	else {
		CLRBIT(pos->pawns[col], SQ64(sq));
		CLRBIT(pos->pawns[BOTH], SQ64(sq));
	}

	// Find piece in list
	for (index = 0; index < pos->pceNum[pce]; ++index) {
		if (pos->pList[pce][index] == sq) {
			t_pceNum = index;
			break;
		}
	}

	// Valid index
	ASSERT(t_pceNum != -1);
	ASSERT(t_pceNum >= 0 && t_pceNum < 10);

	// Remove piece from piece list
	--pos->pceNum[pce];
	pos->pList[pce][t_pceNum] = pos->pList[pce][pos->pceNum[pce]];
}


// Add piece to board
static void AddPiece(const int sq, S_BOARD* pos, const int pce) {

	// Valid piece
	ASSERT(PieceValid(pce));
	ASSERT(SqOnBoard(sq));

	int col = PieceCol[pce];

	// Hash piece in
	HASH_PCE(pce, sq);

	// Pieces array
	pos->pieces[sq] = pce;

	// Adjust piece counters
	if (PieceBig[pce]) {
		++pos->bigPce[col];
		if (PieceMaj[pce]) {
			++pos->majPce[col];
		}
		else {
			++pos->minPce[col];
		}
	}
	else {
		SETBIT(pos->pawns[col], SQ64(sq));
		SETBIT(pos->pawns[BOTH], SQ64(sq));
	}

	// Add material value
	pos->material[col] += PieceVal[pce];
	pos->pList[pce][pos->pceNum[pce]++] = sq;

}


// Move piece on board
static void MovePiece(const int from, const int to, S_BOARD* pos) {

	// Valid move
	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));

	// Define indices
	int index = 0;
	int pce = pos->pieces[from];
	int col = PieceCol[pce];

#ifdef DEBUG
	int t_PieceNum = FALSE;
#endif

	// Hash piece out of from square
	HASH_PCE(pce, from);
	pos->pieces[from] = EMPTY;

	// Hash piece into to square
	HASH_PCE(pce, to);
	pos->pieces[to] = pce;

	// If piece is pawn, adjust bitboard
	if (!PieceBig[pce]) {
		CLRBIT(pos->pawns[col], SQ64(from));
		CLRBIT(pos->pawns[BOTH], SQ64(from));
		SETBIT(pos->pawns[col], SQ64(to));
		SETBIT(pos->pawns[BOTH], SQ64(to));
	}

	// Find index of piece
	for (index = 0; index < pos->pceNum[pce]; ++index) {
		if (pos->pList[pce][index] == from) {
			pos->pList[pce][index] = to;

#ifdef DEBUG
			t_PieceNum = TRUE;
#endif
			break;
		}
	}
	// Valid piece found
	ASSERT(t_PieceNum);
}


// Make move function
int MakeMove(S_BOARD* pos, int move) {

	// Valid board position
	ASSERT(CheckBoard(pos));

	// Get information about move
	int from = FROMSQ(move);
	int to = TOSQ(move);
	int side = pos->side;

	/* Check for flags */

	// Valid move
	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));
	ASSERT(SideValid(side));
	ASSERT(PieceValid(pos->pieces[from]));

	// Store hashkey
	pos->history[pos->hisPly].posKey = pos->posKey;

	// Is en Passant move
	if (move & MFLAGEP) {
		if (side == WHITE) {
			ClearPiece(to - 10, pos);
		}
		else {
			ClearPiece(to + 10, pos);
		}
	}
	// Is castling move
	else if (move & MFLAGCA) {
		switch (to) {
		case C1:
			MovePiece(A1, D1, pos);
			break;
		case C8:
			MovePiece(A8, D8, pos);
			break;
		case G1:
			MovePiece(H1, F1, pos);
			break;
		case G8:
			MovePiece(H8, F8, pos);
			break;
		default: ASSERT(FALSE); break;
		}
	}

	// Hash en Passant
	if (pos->enPas != NO_SQ) HASH_EP;

	// Hash out castle
	HASH_CA;

	// Store information in array
	pos->history[pos->hisPly].move = move;
	pos->history[pos->hisPly].fiftyMove = pos->fiftyMove;
	pos->history[pos->hisPly].enPas = pos->enPas;
	pos->history[pos->hisPly].castlePerm = pos->castlePerm;

	// Update castle rights
	pos->castlePerm &= CastlePerm[from];
	pos->castlePerm &= CastlePerm[to];
	pos->enPas = NO_SQ;


	// Hash in castle
	HASH_CA;

	// Fifty Move Rule
	int captured = CAPTURED(move);
	++pos->fiftyMove;

	// Clear captured piece off board
	if (captured != EMPTY) {
		ASSERT(PieceValid(captured));
		ClearPiece(to, pos);
		pos->fiftyMove = 0;
	}

	// Increment ply
	++pos->hisPly;
	++pos->ply;


	// Pawn moving from 1st square
	if (PiecePawn[pos->pieces[from]]) {
		pos->fiftyMove = 0;
		if (move & MFLAGPS) {
			if (side == WHITE) {
				pos->enPas = from + 10;
				ASSERT(RanksBrd[pos->enPas] == RANK_3)
			}
			else {
				pos->enPas = from - 10;
				ASSERT(RanksBrd[pos->enPas] == RANK_6)
			}
			// Hash in enpas
			HASH_EP;
		}
	}

	// Move piece on board
	MovePiece(from, to, pos);

	// Promotion
	int prPce = PROMOTED(move);
	if (prPce != EMPTY) {
		ASSERT(PieceValid(prPce) && !PiecePawn[prPce]);
		ClearPiece(to, pos);
		AddPiece(to, pos, prPce);
	}

	// Update king square
	if (PieceKing[pos->pieces[to]]) {
		pos->KingSq[pos->side] = to;
	}

	// Change side
	pos->side ^= 1;
	// Hash in side
	HASH_SIDE;

	// Valid Board
	ASSERT(CheckBoard(pos));

	// Not in check
	if (SqAttacked(pos->KingSq[side], pos->side, pos)) {
		TakeMove(pos); // Undo move
		return FALSE;
	}

	return TRUE;
}


// Undo the move
void TakeMove(S_BOARD* pos) {

	// Valid Board
	ASSERT(CheckBoard(pos));

	// Decrement ply
	--pos->hisPly;
	--pos->ply;

	// Retrieve move from history array
	int move = pos->history[pos->hisPly].move;
	int from = FROMSQ(move);
	int to = TOSQ(move);

	// Valid Move
	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));

	// Update Hashes
	if (pos->enPas != NO_SQ) HASH_EP;
	HASH_CA;

	// Retrieve previous information
	pos->castlePerm = pos->history[pos->hisPly].castlePerm;
	pos->fiftyMove = pos->history[pos->hisPly].fiftyMove;
	pos->enPas = pos->history[pos->hisPly].enPas;

	// Update Hashes
	if (pos->enPas != NO_SQ) HASH_EP;
	HASH_CA;

	// Flip side
	pos->side ^= 1;
	HASH_SIDE;

	// Undo en Passant and castle
	if (MFLAGEP & move) {
		if (pos->side == WHITE) {
			AddPiece(to - 10, pos, bP);
		}
		else {
			AddPiece(to + 10, pos, wP);
		}
	}
	else if (MFLAGCA & move) {
		switch (to) {
		case C1:
			MovePiece(D1, A1, pos);
			break;
		case C8:
			MovePiece(D8, A8, pos);
			break;
		case G1:
			MovePiece(F1, H1, pos);
			break;
		case G8:
			MovePiece(F8, H8, pos);
			break;
		default: ASSERT(FALSE); break;
		}
	}

	// Move the piece
	MovePiece(to, from, pos);

	// Updating king array
	if (PieceKing[pos->pieces[from]]) {
		pos->KingSq[pos->side] = from;
	}

	// Add captured piece
	int captured = CAPTURED(move);
	if (captured != EMPTY) {
		ASSERT(PieceValid(captured)); // Valid piece
		AddPiece(to, pos, captured);
	}

	// Undo promotion
	if (PROMOTED(move) != EMPTY) {
		ASSERT(PieceValid(PROMOTED(move)) && !PiecePawn[PROMOTED(move)]);
		ClearPiece(from, pos);
		AddPiece(from, pos, (PieceCol[PROMOTED(move)] == WHITE ? wP : bP));
	}

	// Valid Board
	ASSERT(CheckBoard(pos));

}


// Null move for faster searching
void MakeNullMove(S_BOARD* pos) {

	ASSERT(CheckBoard(pos));
	ASSERT(!SqAttacked(pos->KingSq[pos->side], pos->side ^ 1, pos));

	++pos->ply;
	pos->history[pos->hisPly].posKey = pos->posKey;

	if (pos->enPas != NO_SQ) HASH_EP;

	pos->history[pos->hisPly].move = NOMOVE;
	pos->history[pos->hisPly].fiftyMove = pos->fiftyMove;
	pos->history[pos->hisPly].enPas = pos->enPas;
	pos->history[pos->hisPly].castlePerm = pos->castlePerm;
	pos->enPas = NO_SQ;

	pos->side ^= 1;
	++pos->hisPly;
	HASH_SIDE;

	ASSERT(CheckBoard(pos));
	ASSERT(pos->hisPly >= 0 && pos->hisPly < MAXGAMEMOVES);
	ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);

	return;
} // MakeNullMove


// Undo null move
void TakeNullMove(S_BOARD* pos) {
	ASSERT(CheckBoard(pos));

	--pos->hisPly;
	--pos->ply;

	if (pos->enPas != NO_SQ) HASH_EP;

	pos->castlePerm = pos->history[pos->hisPly].castlePerm;
	pos->fiftyMove = pos->history[pos->hisPly].fiftyMove;
	pos->enPas = pos->history[pos->hisPly].enPas;

	if (pos->enPas != NO_SQ) HASH_EP;
	pos->side ^= 1;
	HASH_SIDE;

	ASSERT(CheckBoard(pos));
	ASSERT(pos->hisPly >= 0 && pos->hisPly < MAXGAMEMOVES);
	ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);
}




