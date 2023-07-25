#include "defs.h"

#include <map>
#include <string>

// convert ASCII character pieces to encoded constants

std::map<char, int> char_pieces = {
    {'P', wP}, {'N', wN}, {'B', wB}, {'R', wR}, {'Q', wQ}, {'K', wK}, {'p', bP},
    {'n', bN}, {'b', bB}, {'r', bR}, {'q', bQ}, {'k', bK}
};

//Map promoted piece to the corresponding ASCII character
std::map<int, char> promoted_pieces = { {wQ, 'q'}, {wR, 'r'}, {wB, 'b'},
                                       {wN, 'n'}, {bQ, 'q'}, {bR, 'r'},
                                       {bB, 'b'}, {bN, 'n'} };

char PceChar[] = ".PNBRQKpnbrqk"; // Define Array Piece to print board
char SideChar[] = "wb-"; // Define Array Side to print board
char RankChar[] = "12345678"; // Define Array Rank to print board
char FileChar[] = "abcdefgh"; // Define Array File to print board

int PieceBig[13] = { FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE }; // Piece Big
int PieceMaj[13] = { FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE }; // Piece Major
int PieceMin[13] = { FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE }; // Piece Minor
int PieceVal[13] = { 0, 100, 325, 325, 550, 1000, 50000, 100, 325, 325, 550, 1000, 50000 }; // Piece Evaluations
int PieceCol[13] = { BOTH, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK }; // Piece Color

int PiecePawn[13] = { FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE }; // Pawn index
int PieceKnight[13] = { FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE }; // Knight index
int PieceKing[13] = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE }; // King index
int PieceRookQueen[13] = { FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE }; // Rook index
int PieceBishopQueen[13] = { FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE }; // Bishop index
int PieceSlides[13] = { FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE }; // Slide index

// Array to mirror a square
int Mirror64[64] = {
56	,	57	,	58	,	59	,	60	,	61	,	62	,	63	,
48	,	49	,	50	,	51	,	52	,	53	,	54	,	55	,
40	,	41	,	42	,	43	,	44	,	45	,	46	,	47	,
32	,	33	,	34	,	35	,	36	,	37	,	38	,	39	,
24	,	25	,	26	,	27	,	28	,	29	,	30	,	31	,
16	,	17	,	18	,	19	,	20	,	21	,	22	,	23	,
8	,	9	,	10	,	11	,	12	,	13	,	14	,	15	,
0	,	1	,	2	,	3	,	4	,	5	,	6	,	7
};


