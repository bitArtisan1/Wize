#include "stdio.h"
#include "defs.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
// Function to check board
int CheckBoard(const S_BOARD* pos) {

	// Arrays hold information to mirror board position
	int t_pceNum[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int t_bigPce[2] = { 0, 0 };
	int t_majPce[2] = { 0, 0 };
	int t_minPce[2] = { 0, 0 };
	int t_material[2] = { 0, 0 };

	int sq64, t_piece, t_pce_num, sq120, color, pcount;

	U64 t_pawns[3] = { 0Ull, 0ULL, 0ULL };

	t_pawns[WHITE] = pos->pawns[WHITE];
	t_pawns[BLACK] = pos->pawns[BLACK];
	t_pawns[BOTH] = pos->pawns[BOTH];

	// Check piece lists
	for (t_piece = wP; t_piece <= bK; ++t_piece) {
		for (t_pce_num = 0; t_pce_num < pos->pceNum[t_piece]; ++t_pce_num) {
			sq120 = pos->pList[t_piece][t_pce_num];
			ASSERT(pos->pieces[sq120] == t_piece);
		}
	}

	// Check Piece count and other counters
	for (sq64 = 0; sq64 < 64; ++sq64) {
		sq120 = SQ120(sq64);
		t_piece = pos->pieces[sq120];
		++t_pceNum[t_piece];
		color = PieceCol[t_piece];
		if (PieceBig[t_piece] == TRUE) ++t_bigPce[color];
		if (PieceMin[t_piece] == TRUE) ++t_minPce[color];
		if (PieceMaj[t_piece] == TRUE) ++t_majPce[color];

		t_material[color] += PieceVal[t_piece];
	}

	// Check Piece number array
	for (t_piece = wP; t_piece <= bK; ++t_piece) {
		ASSERT(t_pceNum[t_piece] == pos->pceNum[t_piece]);
	}

	// Check bitboard counts
	pcount = CNT(t_pawns[WHITE]);
	ASSERT(pcount == pos->pceNum[wP]);
	pcount = CNT(t_pawns[BLACK]);
	ASSERT(pcount == pos->pceNum[bP]);
	pcount = CNT(t_pawns[BOTH]);
	ASSERT(pcount == pos->pceNum[bP] + pos->pceNum[wP]);

	// Check bitboards squares
	while (t_pawns[WHITE]) {
		sq64 = POP(&t_pawns[WHITE]);
		ASSERT(pos->pieces[SQ120(sq64)] == wP);
	}
	while (t_pawns[BLACK]) {
		sq64 = POP(&t_pawns[BLACK]);
		ASSERT(pos->pieces[SQ120(sq64)] == bP);
	}
	while (t_pawns[BOTH]) {
		sq64 = POP(&t_pawns[BOTH]);
		ASSERT( (pos->pieces[SQ120(sq64)] == wP)  || (pos->pieces[SQ120(sq64)] == bP));
	}

	// Check material
	ASSERT(t_material[WHITE] == pos->material[WHITE] && t_material[BLACK] == pos->material[BLACK]);
	ASSERT(t_minPce[WHITE] == pos->minPce[WHITE] && t_minPce[BLACK] == pos->minPce[BLACK]);
	ASSERT(t_majPce[WHITE] == pos->majPce[WHITE] && t_majPce[BLACK] == pos->majPce[BLACK]);
	ASSERT(t_bigPce[WHITE] == pos->bigPce[WHITE] && t_bigPce[BLACK] == pos->bigPce[BLACK]);

	// Check side
	ASSERT(pos->side == WHITE || pos->side == BLACK);
	ASSERT(GeneratePosKey(pos) == pos->posKey);

	// Check en Passant
	ASSERT(pos->enPas == NO_SQ || (RanksBrd[pos->enPas] == RANK_6 && pos->side == WHITE)
		|| (RanksBrd[pos->enPas] == RANK_3 && pos->side == BLACK));

	// Checking king square
	ASSERT(pos->pieces[pos->KingSq[WHITE]] == wK);
	ASSERT(pos->pieces[pos->KingSq[BLACK]] == bK);

	return TRUE;
}

// Update the material list
void UpdateListsMaterial(S_BOARD* pos) {

	int piece, sq, index, color;

	// Loop through every square on the board
	for (index = 0; index < BRD_SQ_NUM; ++index) {
		sq = index;
		piece = pos->pieces[index];
		if (piece != OFFBOARD && piece != EMPTY) {
			color = PieceCol[piece];
			if (PieceBig[piece] == TRUE) ++pos->bigPce[color];
			if (PieceMin[piece] == TRUE) ++pos->minPce[color];
			if (PieceMaj[piece] == TRUE) ++pos->majPce[color];

			// Set Material
			pos->material[color] += PieceVal[piece];

			pos->pList[piece][pos->pceNum[piece]] = sq;
			++pos->pceNum[piece];

			// Update king positions
			if (piece == wK) pos->KingSq[WHITE] = sq;
			if (piece == bK) pos->KingSq[BLACK] = sq;

			// Set bit for Pawn
			if (piece == wP) {
				SETBIT(pos->pawns[WHITE], SQ64(sq));
				SETBIT(pos->pawns[BOTH], SQ64(sq));
			}
			else if (piece == bP) {
				SETBIT(pos->pawns[BLACK], SQ64(sq));
				SETBIT(pos->pawns[BOTH], SQ64(sq));
			}
		}
	}
}


// Parse FEN string
void ParseFen(const std::string& command, S_BOARD* pos) {

	// Check that there is somethign to point at
	ASSERT(pos != NULL);

	// Declare variables
	int rank = RANK_8; // Start from 8th Rank
	int file = FILE_A; // Start from A file
	int piece = 0;
	int count = 0;
	int i = 0;
	int sq64 = 0;
	int sq120 = 0;

	// Reset the board
	ResetBoard(pos);

	std::vector<std::string> tokens = split_command(command);

	const std::string pos_string = tokens.at(0);
	const std::string turn = tokens.at(1);
	const std::string castle_perm = tokens.at(2);
	const std::string ep_square = tokens.at(3);
	std::string fifty_move;
	std::string HisPly;
	//Keep fifty move and Hisply arguments optional
	if (tokens.size() >= 5) {
		fifty_move = tokens.at(4);
		if (tokens.size() >= 6) {
			HisPly = tokens.at(5);
			HisPly = tokens.at(5);
		}
	}

	int fen_counter = 0;
	for (int rank = RANK_8; rank >= RANK_1; --rank) {
		// loop over board files
		for (int file = FILE_A; file <= FILE_H; ++file) {
			// init current square
			const int square = rank * 8 + file;
			const char current_char = pos_string[fen_counter];

			// match ascii pieces within FEN string
			if ((current_char >= 'a' && current_char <= 'z') || (current_char >= 'A' && current_char <= 'Z')) {
				// init piece type
				const int piece = char_pieces[current_char];
				if (piece != EMPTY) {
					sq64 = rank * 8 + file;
					sq120 = SQ120(sq64);
					pos->pieces[sq120] = piece;
				}
				++fen_counter;
			}

			// match empty square numbers within FEN string
			if (current_char >= '0' && current_char <= '9') {
				// init offset (convert char 0 to int 0)
				const int offset = current_char - '1';
				file += offset;
				// increment pointer to FEN string
				++fen_counter;
			}

			// match rank separator
			if (pos_string[fen_counter] == '/')
				// increment pointer to FEN string
				++fen_counter;
		}
	}
	//parse player turn
	(turn == "w") ? (pos->side = WHITE) : (pos->side = BLACK);

	//Parse castling rights
	for (const char c : castle_perm) {
		switch (c) {
		case 'K':
			(pos->castlePerm) |= WKCA;
			break;
		case 'Q':
			(pos->castlePerm) |= WQCA;
			break;
		case 'k':
			(pos->castlePerm) |= BKCA;
			break;
		case 'q':
			(pos->castlePerm) |= BQCA;
			break;
		case '-':
			break;
		}
	}

	// parse enpassant square
	if (ep_square != "-" && ep_square.size() == 2) {
		// parse enpassant file & rank
		const int file = ep_square[0] - 'a';
		const int rank = 8 - (ep_square[1] - '0');

		// init enpassant square
		pos->enPas = rank * 8 + file;
	}
	// no enpassant square
	else
		pos->enPas = NO_SQ;

	//Read fifty moves counter
	if (!fifty_move.empty()) {
		pos->fiftyMove = std::stoi(fifty_move);
	}
	//Read Hisply moves counter
	if (!HisPly.empty()) {

		pos->hisPly = std::stoi(HisPly);

	}
	UpdateListsMaterial(pos);

	pos->posKey = GeneratePosKey(pos);

}

// Reset the board
void ResetBoard(S_BOARD *pos) {
	
	int index = 0;

	// Set all squares to "offboard"
	for (index = 0; index < BRD_SQ_NUM; ++index) {
		pos->pieces[index] = OFFBOARD;
	}

	// Define squares on the board to empty
	for (index = 0; index < 64; ++index) {
		pos->pieces[SQ120(index)] = EMPTY;
	}

	// Reset arrays in board structure
	for (index = 0; index < 2; ++index) {
		pos->bigPce[index] = 0;
		pos->majPce[index] = 0;
		pos->minPce[index] = 0;
		pos->material[index] = 0;
	}

	// Reset Pawns
	for (index = 0; index < 3; ++index) {
		pos->pawns[index] = 0ULL; // U64 Type
	}

	// Reset piece number by piece type
	for (index = 0; index < 13; ++index) {
		pos->pceNum[index] = 0;
	}

	// Reset kingPosition for white and black
	pos->KingSq[WHITE] = pos->KingSq[BLACK] = NO_SQ;

	// Reset the side to both
	pos->side = BOTH;
	pos->enPas = NO_SQ;
	pos->fiftyMove = 0;

	// Reset the nubmer of halfmoves
	pos->ply = 0;
	pos->hisPly = 0;

	// Reset Castle rights
	pos->castlePerm = 0;

	// Reset Position Key
	pos->posKey = 0ULL;

}

// Print the board
std::map<int, std::string> id_to_unicode = {
        { 0, "." },
        { 1, "♙" }, { 2, "♘" }, { 3, "♗" }, { 4, "♖" }, { 5, "♕" }, { 6, "♔" },
        { 7, "♟" }, { 8, "♞" }, { 9, "♝" }, { 10, "♜" }, { 11, "♛" }, { 12, "♚" },
};

void PrintBoard(const S_BOARD* pos) {
    int sq, file, rank, piece;

    std::cout << "Game Board:\n\n";
    std::cout << "Uppercase: WHITE ♙ (P)\nLowercase: BLACK ♟ (p)\n\n";
    std::cout << "   ╭───┬───┬───┬───┬───┬───┬───┬───╮\n";

    for (rank = RANK_8; rank >= RANK_1; rank--) {
        std::cout << " " << rank + 1 << " │";
        for (file = FILE_A; file <= FILE_H; file++) {
            sq = FR2SQ(file, rank);
            piece = pos->pieces[sq];
            std::cout << " ";
            if (piece != NO_SQ) {
                std::cout << std::setw(1) << id_to_unicode[piece].c_str() << " ";
            } else {
                std::cout << ". ";
            }
            std::cout << "│";
        }
        std::cout << "\n";
        if (rank > RANK_1) {
            std::cout << "   ├───┼───┼───┼───┼───┼───┼───┼───┤\n";
        }
    }

    std::cout << "   ╰───┴───┴───┴───┴───┴───┴───┴───╯\n";
    std::cout << "     a   b   c   d   e   f   g   h\n\n";
    std::cout << " • Side: " << SideChar[pos->side] << "\n";
    std::cout << " • En Passant: " << std::dec << pos->enPas << "\n";
    std::cout << " • Castle: " << (pos->castlePerm & WKCA ? 'K' : '-')
              << (pos->castlePerm & WQCA ? 'Q' : '-')
              << (pos->castlePerm & BKCA ? 'k' : '-')
              << (pos->castlePerm & BQCA ? 'q' : '-') << "\n";
    std::cout << " • \033[38;5;208mPosition Key:\033[0m " << std::hex << std::uppercase << pos->posKey << "\n";
}


// Mirror the board
void MirrorBoard(S_BOARD* pos) {

	int tempPiecesArray[64]; // Hold the piece status of the 64 board squares
	int tempSide = pos->side ^ 1; // Switch side
	int SwapPiece[13] = { EMPTY, bP, bN, bB, bR, bQ, bK, wP, wN, wB, wR, wQ, wK }; // Switch the places of pieces
	int tempCastlePerm = 0; // Clear castle rights
	int tempEnPas = NO_SQ; // Clear en Passant square

	int sq;
	int tp;

	// Mirror castle rights
	if (pos->castlePerm & WKCA) tempCastlePerm |= BKCA;
	if (pos->castlePerm & WQCA) tempCastlePerm |= BQCA;

	if (pos->castlePerm & BKCA) tempCastlePerm |= WKCA;
	if (pos->castlePerm & BQCA) tempCastlePerm |= WQCA;

	// Mirror en Passant square
	if (pos->enPas != NO_SQ) {
		tempEnPas = SQ120(Mirror64[SQ64(pos->enPas)]);
	}

	// Mirror pieces
	for (sq = 0; sq < 64; sq++) {
		tempPiecesArray[sq] = pos->pieces[SQ120(Mirror64[sq])];
	}

	ResetBoard(pos); // Clear board

	// Mirror the board
	for (sq = 0; sq < 64; sq++) {
		tp = SwapPiece[tempPiecesArray[sq]];
		pos->pieces[SQ120(sq)] = tp;
	}

	// Mirror parameters
	pos->side = tempSide;
	pos->castlePerm = tempCastlePerm;
	pos->enPas = tempEnPas;

	// Update poskey and material
	pos->posKey = GeneratePosKey(pos);
	UpdateListsMaterial(pos);

	// Valid board
	ASSERT(CheckBoard(pos));
}
