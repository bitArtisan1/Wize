#include "defs.h"
#include <cstring>

// Check if square is on the board
int SqOnBoard(const int sq) {
	return FilesBrd[sq] == OFFBOARD ? 0 : 1;
}

// Check if the side is valid
int SideValid(const int side) {
	return (side == WHITE || side == BLACK) ? 1 : 0;
}

// Check if the File and Rank is valid
int FileRankValid(const int fr) {
	return (fr >= 0 && fr <= 7) ? 1 : 0;
}

// Check if the piece value is valid including EMPTY
int PieceValidEmpty(const int pce) {
	return (pce >= EMPTY && pce <= bK) ? 1 : 0;
}

// Check if the piece valid excluding EMPTY
int PieceValid(const int pce) {
	return (pce >= wP && pce <= bK) ? 1 : 0;
}

// Test mirror function
void MirrorEvalTest(S_BOARD* pos) {
    FILE* file;
    file = fopen("mirror.epd", "r");
    char lineIn[1024];
    int ev1 = 0; int ev2 = 0;
    int positions = 0;
    if (file == NULL) {
        printf("File Not Found\n");
        return;
    }
    else {
        while (fgets(lineIn, 1024, file) != NULL) {
            ParseFen(lineIn, pos);
            positions++;
            ev1 = EvalPosition(pos);
            MirrorBoard(pos);
            ev2 = EvalPosition(pos);

            if (ev1 != ev2) {
                printf("\n\n\n");
                ParseFen(lineIn, pos);
                PrintBoard(pos);
                MirrorBoard(pos);
                PrintBoard(pos);
                printf("\n\nMirror Fail:\n%s\n", lineIn);
                getchar();
                return;
            }

            if ((positions % 1000) == 0) {
                printf("position %d\n", positions);
            }

            memset(&lineIn[0], 0, sizeof(lineIn));
        }
    }
}