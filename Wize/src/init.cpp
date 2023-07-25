#include "defs.h"
#include "stdlib.h"

// include NNUE probe library wrapper header
#include "nnue_eval.h"

// Fill 64 bits with random numbers, I.e: 0000 000000000000000 000000000000000 000000000000000 000000000000000
#define RAND_64 (	(U64)rand() | \
					(U64)rand() << 15 | \
					(U64)rand() << 30 | \
					(U64)rand() << 45 | \
					((U64)rand() & 0xf) << 60	)

int Sq120ToSq64[BRD_SQ_NUM];
int Sq64ToSq120[64];

U64 SetMask[64];
U64 ClearMask[64];

// Keys for position hashkeys
U64 PieceKeys[13][120]; // Pieces indexed by squares
U64 SideKey; // Side to move
U64 CastleKeys[16]; // Castle Rights, I.e: 1 0 0 1

// Define Files and Ranks for conversion
int FilesBrd[BRD_SQ_NUM];
int RanksBrd[BRD_SQ_NUM];

// File and rank masks
U64 FileBBMask[8];
U64 RankBBMask[8];

// Pawn bit masks
U64 BlackPassedMask[64];
U64 WhitePassedMask[64];
U64 IsolatedMask[64];
U64 DoubledMask[64];

// Engine options
S_OPTIONS EngineOptions[1];

// Initlization of evaluation masks
static void InitEvalMasks() {

	// Definitions
	int sq, tsq, r, f;

	// Initialize file and rank masks
	for (sq = 0; sq < 8; ++sq) {
		FileBBMask[sq] = 0ULL;
		RankBBMask[sq] = 0ULL;
	}

	// Fill the masks
	for (r = RANK_8; r >= RANK_1; --r) {
		for (f = FILE_A; f <= FILE_H; ++f) {
			sq = r * 8 + f;
			FileBBMask[f] |= (1ULL << sq);
			RankBBMask[r] |= (1ULL << sq);
		}
	}
	// Clear the masks
	for (sq = 0; sq < 64; ++sq) {
		IsolatedMask[sq] = 0ULL;
		WhitePassedMask[sq] = 0ULL;
		BlackPassedMask[sq] = 0ULL;
		DoubledMask[sq] = 0ULL;
	}
	// Iterate in vertical direction
	for (sq = 0; sq < 64; ++sq) {
		tsq = sq + 8;
		while (tsq < 64) {
			WhitePassedMask[sq] |= (1ULL << tsq);
			DoubledMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)]];
			tsq += 8;
		}
		tsq = sq - 8;
		while (tsq >= 0) {
			BlackPassedMask[sq] |= (1ULL << tsq);
			DoubledMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)]];
			tsq -= 8;
		}
		// Iterate in horizontal direciton
		if (FilesBrd[SQ120(sq)] > FILE_A) {
			IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] - 1];
			tsq = sq + 7;
			while (tsq < 64) {
				WhitePassedMask[sq] |= (1ULL << tsq);
				tsq += 8;
			}
			tsq = sq - 9;
			while (tsq >= 0) {
				BlackPassedMask[sq] |= (1ULL << tsq);
				tsq -= 8;
			}
		}
		if (FilesBrd[SQ120(sq)] < FILE_H) {
			IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] + 1];
			tsq = sq + 9;
			while (tsq < 64) {
				WhitePassedMask[sq] |= (1ULL << tsq);
				tsq += 8;
			}
			tsq = sq - 7;
			while (tsq >= 0) {
				BlackPassedMask[sq] |= (1ULL << tsq);
				tsq -= 8;
			}
		}
	}

}



// Function to create arrays to convert index to file and rank
static void InitFilesRanksBrd() {
	
	// Define Indices
	int index = 0;
	int file = FILE_A;
	int rank = RANK_1;
	int sq = A1;

	// Set Arrays to "offboard"
	for (index = 0; index < BRD_SQ_NUM; ++index) {
		FilesBrd[index] = OFFBOARD;
		RanksBrd[index] = OFFBOARD;
	}

	// Setting by rank and file
	for (rank = RANK_1; rank <= RANK_8; ++rank) {
		for (file = FILE_A; file <= FILE_H; ++file) {
			sq = FR2SQ(file, rank);
			FilesBrd[sq] = file;
			RanksBrd[sq] = rank;
		}
	}

}

static void InitHashKeys() {
	
	// Define Indices
	int index = 0;
	int index2 = 0;
	// Fill PieceKeys with random numbers
	for (index = 0; index < 13; ++index) {
		for (index2 = 0; index2 < 120; ++index2) {
			PieceKeys[index][index2] = RAND_64;
		}
	}
	// Fill CastleKeys with random numbers
	SideKey = RAND_64;
	for (index = 0; index < 16; ++index) {
		CastleKeys[index] = RAND_64;
	}
}

// Initialize bitmasks to set and clear bits
static void InitBitMasks() {
	int index = 0;

	// Zero the values inside the array
	for (index = 0; index < 64; ++index) {
		SetMask[index] = 0ULL;
		ClearMask[index] = 0ULL;
	}

	// Set the values for the Masks
	for (index = 0; index < 64; ++index) {
		SetMask[index] |= (1ULL << index);
		ClearMask[index] = ~SetMask[index]; // Set to bitwise complement of SetMask
	}
}

// Init function for conversion from 120 to 64
static void InitSq120To64() {

	// Declare Variables
	int index = 0;
	int file = FILE_A;
	int rank = RANK_1;
	int sq = A1;
	int sq64 = 0;
	// Iterating through all the squares in the array
	for (index = 0; index < BRD_SQ_NUM; ++index) {
		Sq120ToSq64[index] = 65; // Setting the values all to 65, failsafe value
	}
	// Iterating through all the squares in the array
	for (index = 0; index < 64; ++index) {
		Sq64ToSq120[index] = 120; // Setting the values all to 120, failsafe value
	}
	// Iterate over rank and file
	for (rank = RANK_1; rank <= RANK_8; ++rank) {
		for (file = FILE_A; file <= FILE_H; ++file) {
			sq = FR2SQ(file, rank); // Get Square
			Sq64ToSq120[sq64] = sq;
			Sq120ToSq64[sq] = sq64;
			++sq64;
		}
	}
}

void AllInit() {
	InitSq120To64();
	InitBitMasks();
	InitHashKeys();
	InitFilesRanksBrd();
	if (not USENNUE) {
		InitEvalMasks();
	}
	// InitMvvLva(); Hard coded in
	InitPolyBook();


	// init NNUE
	if (USENNUE) {

		init_nnue((char*)"nn-9931db908a9b.nnue");
	}
}