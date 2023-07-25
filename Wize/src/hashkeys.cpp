#include "defs.h"

// Generate Position Key
U64 GeneratePosKey(const S_BOARD *pos) {
	// Variable definitions
	int sq = 0;
	U64 finalKey = 0; // Final Key to return
	int piece = EMPTY;

	// Pieces 
	for (sq = 0; sq < BRD_SQ_NUM; ++sq) {
		piece = pos->pieces[sq];
		if (piece != NO_SQ && piece != EMPTY && piece != OFFBOARD) {
			ASSERT(piece >= wP && piece <= bK);
			finalKey ^= PieceKeys[piece][sq]; // Hash in number at piece and square
		}
	}

	// Hash in side Key
	if (pos->side == WHITE) {
		finalKey ^= SideKey;
	}

	// Hash in en passant Key
	if (pos->enPas != NO_SQ) {
		ASSERT(pos->enPas >= 0 && pos->enPas < BRD_SQ_NUM);
		finalKey ^= PieceKeys[EMPTY][pos->enPas];
	}

	// Hash in castle Key
	ASSERT(pos->castlePerm >= 0 && pos->castlePerm <= 15);
	finalKey ^= CastleKeys[pos->castlePerm];

	return finalKey;
}