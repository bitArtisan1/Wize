#include "defs.h"
#include "stdio.h"

const int KnDir[8] = { -8, -19, -21, -12, 8, 19, 21, 12 }; // Definitions for the movement of knights
const int RkDir[4] = { -1, -10, 1, 10 }; // Definitions for the movement of rooks
const int BiDir[4] = { -9, -11, 11, 9 }; // Definitions for the movement of bishops
const int KiDir[8] = { -1, -10, 1 ,10, -9, -11, 11, 9 }; // Definitions for the movement of kings

// Is a square under attack
int SqAttacked(const int sq, const int side, const S_BOARD* pos) {

	// Variable definitions
	int pce, index, t_sq, dir;

	// Checking if the given parameters are valid
	ASSERT(SqOnBoard(sq));
	ASSERT(SideValid(side));
	ASSERT(CheckBoard(pos));

	// Pawns
	if (side == WHITE) {
		if (pos->pieces[sq - 11] == wP || pos->pieces[sq - 9] == wP) {
			return TRUE;
		}
	}
	else {
		if (pos->pieces[sq + 11] == bP || pos->pieces[sq + 9] == bP) {
			return TRUE;
		}
	}

	// Knights
	for (index = 0; index < 8; ++index) {
		pce = pos->pieces[sq + KnDir[index]];
		if (pce != OFFBOARD && IsKN(pce) && PieceCol[pce] == side) {
			return TRUE;
		}
	}

	// Rooks and Queens
	for (index = 0; index < 4; ++index) {
		dir = RkDir[index];
		t_sq = sq + dir;
		pce = pos->pieces[t_sq];
		while (pce != OFFBOARD) {
			if (pce != EMPTY) {
				if (IsRQ(pce) && PieceCol[pce] == side) {
					return TRUE;
				}
				break;
			}
			t_sq += dir;
			pce = pos->pieces[t_sq];
		}
	}

	// Bishops and Queens
	for (index = 0; index < 4; ++index) {
		dir = BiDir[index];
		t_sq = sq + dir;
		pce = pos->pieces[t_sq];
		while (pce != OFFBOARD) {
			if (pce != EMPTY) {
				if (IsBQ(pce) && PieceCol[pce] == side) {
					return TRUE;
				}
				break;
			}
			t_sq += dir;
			pce = pos->pieces[t_sq];	
		}
	}

	// Kings
	for (index = 0; index < 8; ++index) {
		pce = pos->pieces[sq + KiDir[index]];
		if (pce != OFFBOARD && IsKI(pce) && PieceCol[pce] == side) {
			return TRUE;
		}
	}

	return FALSE;
}


