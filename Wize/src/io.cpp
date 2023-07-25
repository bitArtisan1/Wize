#include "stdio.h"
#include "defs.h"

// Print Square
char* PrSq(const int sq) {

	static char SqStr[3];

	int file = FilesBrd[sq];
	int rank = RanksBrd[sq];

	sprintf(SqStr, "%c%c", ('a' + file), ('1' + rank));

	return SqStr; // Return pointer to string
}

// Print Move
char* PrMove(const int move) {

	static char MvStr[6];

	int ff = FilesBrd[FROMSQ(move)];
	int rf = RanksBrd[FROMSQ(move)];
	int ft = FilesBrd[TOSQ(move)];
	int rt = RanksBrd[TOSQ(move)];

	int promoted = PROMOTED(move);

	if (promoted) {
		char pchar = 'q';
		if (IsKN(promoted)) {
			pchar = 'n';
		}
		else if (IsRQ(promoted) && !IsBQ(promoted)) {
			pchar = 'r';
		}
		else if (!IsRQ(promoted) && IsBQ(promoted)) {
			pchar = 'b';
		}
		sprintf(MvStr, "%c%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt), pchar);
	}					
	else {
		sprintf(MvStr, "%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt));
	}

	return MvStr; // Return pointer to array
}

// Print the move list
void PrintMoveList(const S_MOVELIST* list) {
	// Define the indices
	int index = 0;
	int score = 0;
	int move = 0;
	
	printf("MoveList:\n");

	// Print each move in movelist
	for (index = 0; index < list->count; ++index) {
		move = list->moves[index].move;
		score = list->moves[index].score;

		printf("Move:%d > %s (score:%d)\n", index + 1, PrMove(move), score);

	}
	printf("MoveList Total %d Moves:\n\n", list->count);

}


//// Find a move string and return an integer
//int ParseMove(char* ptrChar, S_BOARD* pos) {
//
//	if (ptrChar[1] > '8' || ptrChar[1] < '1') return NOMOVE;
//	if (ptrChar[3] > '8' || ptrChar[3] < '1') return NOMOVE;
//	if (ptrChar[0] > 'h' || ptrChar[0] < 'a') return NOMOVE;
//	if (ptrChar[2] > 'h' || ptrChar[2] < 'a') return NOMOVE;
//
//	int from = FR2SQ(ptrChar[0] - 'a', ptrChar[1] - '1');
//	int to = FR2SQ(ptrChar[2] - 'a', ptrChar[3] - '1');
//
//	ASSERT(SqOnBoard(from) && SqOnBoard(to));
//
//	S_MOVELIST list[1];
//	GenerateAllMoves(pos, list);
//	int MoveNum = 0;
//	int Move = 0;
//	int PromPce = EMPTY;
//
//	// Search for move in movelist
//	for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
//		Move = list->moves[MoveNum].move;
//		if (FROMSQ(Move) == from && TOSQ(Move) == to) {
//			PromPce = PROMOTED(Move);
//			if (PromPce != EMPTY) {
//				if (IsRQ(PromPce) && !IsBQ(PromPce) && ptrChar[4] == 'r') {
//					return Move;
//				} 
//				else if (!IsRQ(PromPce) && IsBQ(PromPce) && ptrChar[4] == 'b') {
//					return Move;
//				}
//				else if (IsRQ(PromPce) && IsBQ(PromPce) && ptrChar[4] == 'q') {
//					return Move;
//				}
//				else if (IsKN(PromPce) && ptrChar[4] == 'n') {
//					return Move;
//				}
//				continue;
//			}
//			return Move;
//		}
//	}
//
//	return NOMOVE;
//}


//
////Prints the uci output
//void PrintUciOutput(const int score, const int depth, const S_SEARCHINFO* td, const  S_OPTIONS* options) {
//
//	//This handles the basic console output
//	long  time = GetTimeMs() - td->info.starttime;
//	uint64_t nodes = td->info.nodes;
//
//	uint64_t nps = nodes / (time + !time) * 1000;
//	if (print_uci)
//	{
//		if (score > -mate_value && score < -mate_score)
//			std::cout << "info score mate " << -(score + mate_value) / 2 << " depth " << depth << " nodes " << nodes <<
//			" nps " << nps << " time " << GetTimeMs() - td->info.starttime << " pv ";
//
//		else if (score > mate_score && score < mate_value)
//			std::cout << "info score mate " << (mate_value - score) / 2 + 1 << " depth " << depth << " nodes " << nodes <<
//			" nps " << nps << " time " << GetTimeMs() - td->info.starttime << " pv ";
//
//		else
//			std::cout << "info score cp " << score << " depth " << depth << " nodes " << nodes <<
//			" nps " << nps << " time " << GetTimeMs() - td->info.starttime << " pv ";
//
//		// loop over the moves within a PV line
//		for (int count = 0; count < td->pv_table.pvLength[0]; count++) {
//			// print PV move
//			PrintMove(td->pv_table.pvArray[0][count]);
//			printf(" ");
//		}
//
//		// print new line
//		std::cout << std::endl;
//	}
//	else {
//
//		//Convert time in seconds if possible
//		std::string time_unit = "ms";
//		float parsed_time = time;
//		if (parsed_time >= 1000) {
//			parsed_time = parsed_time / 1000;
//			time_unit = 's';
//			if (parsed_time >= 60)
//			{
//				parsed_time = parsed_time / 60;
//				time_unit = 'm';
//			}
//		}
//
//		// convert time to string
//		std::stringstream time_stream;
//		time_stream << std::setprecision(3) << parsed_time;
//		std::string time_string = time_stream.str() + time_unit;
//
//		//Convert score to a decimal format or to a mate string
//		float parsed_score = 0;
//		std::string score_unit = "";
//		if (score > -mate_value && score < -mate_score) {
//			parsed_score = std::abs((score + mate_value) / 2);
//			score_unit = "-M";
//		}
//		else if (score > mate_score && score < mate_value) {
//			parsed_score = (mate_value - score) / 2 + 1;
//			score_unit = "+M";
//		}
//		else {
//			parsed_score = static_cast<float>(score) / 100;
//			if (parsed_score > 0)   score_unit = '+';
//		}
//		// convert score to string
//		std::stringstream score_stream;
//		score_stream << std::fixed << std::setprecision(2) << parsed_score;
//		std::string score_color = Pick_color(score);
//		std::string color_reset = "\033[0m";
//		std::string score_string = score_color + score_unit + score_stream.str() + color_reset;
//		int node_precision = 0;
//		//convert nodes into string
//		std::string node_unit = "n";
//		float parsed_nodes = static_cast<float>(nodes);
//		if (parsed_nodes >= 1000) {
//			parsed_nodes = parsed_nodes / 1000;
//			node_unit = "Kn";
//			node_precision = 2;
//			if (parsed_nodes >= 1000)
//			{
//				parsed_nodes = parsed_nodes / 1000;
//				node_unit = "Mn";
//			}
//		}
//
//		std::stringstream node_stream;
//		node_stream << std::fixed << std::setprecision(node_precision) << parsed_nodes;
//		std::string node_string = node_stream.str() + node_unit;
//
//		//Pretty print search info
//		std::cout << std::setw(3) << depth << "/";
//		std::cout << std::left << std::setw(3) << td->info.seldepth;
//
//		std::cout << std::right << std::setw(8) << time_string;
//		std::cout << std::right << std::setw(10) << node_string;
//		std::cout << std::setw(7) << std::right << " " << score_string;
//		std::cout << std::setw(7) << std::right << std::fixed << std::setprecision(0) << nps / 1000.0 << "kn/s" << " ";
//
//		// loop over the moves within a PV line
//		for (int count = 0; count < td->pv_table.pvLength[0]; count++)
//		{
//			// print PV move
//			PrintMove(td->pv_table.pvArray[0][count]);
//			std::cout << " ";
//		}
//
//		// print new line
//		std::cout << "\n";
//	}
//
//	return;
//}



