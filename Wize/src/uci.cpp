#include "stdio.h"
#include "defs.h"
#include "tinycthread.h"
#include "string.h"
#include <iostream>


thrd_t mainSearchThread;

thrd_t LaunchSearchThread(S_BOARD* pos, S_SEARCHINFO* info, S_HASHTABLE* table) {
    S_SEARCH_THREAD_DATA* pSearchData = (S_SEARCH_THREAD_DATA *) malloc(sizeof(S_SEARCH_THREAD_DATA));

    pSearchData->originalPosition = pos;
    pSearchData->info = info;
    pSearchData->ttable = table;

    thrd_t th;
    thrd_create(&th, &SearchPosition_Thread, (void*)pSearchData);

    return th;
}


// Function to join threads
void JoinSearchThread(S_SEARCHINFO* info) {
    info->stopped = TRUE;
    thrd_join(mainSearchThread, NULL);
}

// parses the moves part of a fen string and plays all the moves included
static void parse_moves(const std::string moves, S_BOARD* pos)
{
    std::vector<std::string> move_tokens = split_command(moves);
    // loop over moves within a move string
    for (size_t i = 0; i < move_tokens.size(); ++i) {
        // parse next move
        int move = ParseMove(move_tokens[i], pos);
        // make move on the chess board
        MakeMove(pos, move);
    }
}


// parse UCI "go" command, returns true if we have to search afterwards and false otherwise
static void ParseGo(const std::string& line, S_SEARCHINFO* info, S_BOARD* pos, S_HASHTABLE *table) {


    int depth = -1, movetime = -1; int movestogo = 25;
    int time = -1; int inc = 0;
    info->timeset = FALSE;
    bool movestogoset = false;

    std::vector<std::string> tokens = split_command(line);

    //loop over all the tokens and parse the commands
    for (size_t i = 1; i < tokens.size(); ++i) {

        if (tokens.at(1) == "infinite") {
            continue;
        }

        if (tokens.at(i) == "binc" && pos->side == BLACK) {
            inc = std::stoi(tokens[i + 1]);
        }

        if (tokens.at(i) == "winc" && pos->side == WHITE) {
            inc = std::stoi(tokens[i + 1]);
        }

        if (tokens.at(i) == "wtime" && pos->side == WHITE) {
            time = std::stoi(tokens[i + 1]);
            info->timeset = TRUE;
        }
        if (tokens.at(i) == "btime" && pos->side == BLACK) {
            time = std::stoi(tokens[i + 1]);
            info->timeset = TRUE;
        }

        if (tokens.at(i) == "movestogo") {
            movestogo = std::stoi(tokens[i + 1]);
            if (movestogo > 0)
                info->movestogo = movestogo;
            movestogoset = true;
        }

        if (tokens.at(i) == "movetime") {
            movetime = std::stoi(tokens[i + 1]);
            time = movetime;
            info->timeset = TRUE;
        }

        if (tokens.at(i) == "depth") {
            depth = std::stoi(tokens[i + 1]);
        }
    }

    info->starttime = GetTimeMs();
    info->depth = depth;

    int safety_overhead = 50;

    //calculate time allocation for the move
    if (info->timeset and movetime != -1) {
        time -= safety_overhead;
        info->stoptime = info->starttime + time + inc;
        info->optstoptime = info->starttime + time + inc;
    }
    else if (info->timeset && movestogoset)
    {
        time -= safety_overhead;
        int time_slot = time / info->movestogo;
        int basetime = (time_slot);
        info->stoptime = info->starttime + basetime;
        info->optstoptime = info->starttime + basetime;
    }
    else if (info->timeset)
    {
        time -= safety_overhead;
        int time_slot = time / movestogo + inc / 2;
        int basetime = (time_slot);
        //optime is the time we use to stop if we just cleared a depth
        int optime = basetime * 0.6;
        //maxtime is the absolute maximum time we can spend on a search
        int maxtime = std::min(time, basetime * 2);
        info->stoptime = info->starttime + maxtime;
        info->optstoptime = info->starttime + optime;
    }

    if (depth == -1) {
        info->depth = MAXDEPTH;
    }

    std::cout << "info ";
    std::cout << std::dec << "time: " << time << " ";
    std::cout << std::dec << "start: " << info->starttime << " ";
    std::cout << std::dec << "stop: " << info->stoptime << " ";
    std::cout << std::dec << "depth: " << info->depth << " \n";

    mainSearchThread = LaunchSearchThread(pos, info, table);
    //SearchPosition(pos, info, table);
}



//convert a move to coordinate notation to internal notation
int ParseMove(const std::string& move_string, S_BOARD* pos) {

    // parse source square
    const int from = FR2SQ(move_string[0] - 'a', move_string[1] - '1');

    // parse target square
    const int to = FR2SQ(move_string[2] - 'a', move_string[3] - '1');

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);
    int MoveNum = 0;
    int Move = 0;
    int PromPce = EMPTY;

    // Search for move in movelist
    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        Move = list->moves[MoveNum].move;
        if (FROMSQ(Move) == from && TOSQ(Move) == to) {
            PromPce = PROMOTED(Move);
            if (PromPce != EMPTY) {
                if (IsRQ(PromPce) && !IsBQ(PromPce) && move_string[4] == 'r') {
                    return Move;
                }
                else if (!IsRQ(PromPce) && IsBQ(PromPce) && move_string[4] == 'b') {
                    return Move;
                }
                else if (IsRQ(PromPce) && IsBQ(PromPce) && move_string[4] == 'q') {
                    return Move;
                }
                else if (IsKN(PromPce) && move_string[4] == 'n') {
                    return Move;
                }
                continue;
            }
            return Move;
        }
    }

    return NOMOVE;
}


// parse UCI "position" command
static void ParsePosition(const std::string& command, S_BOARD* pos) {

    // parse UCI "startpos" command
    if (command.find("startpos") != std::string::npos) {
        // init chess board with start position
        ParseFen(START_FEN, pos);
    }

        // parse UCI "fen" command
    else {

        // if a "fen" command is available within command string
        if (command.find("fen") != std::string::npos) {
            // init chess board with position from FEN string
            ParseFen(command.substr(command.find("fen") + 4, std::string::npos), pos);
        }
        else {
            // init chess board with start position
            ParseFen(START_FEN, pos);
        }

    }

    // if there are moves to be played in the fen play them
    if (command.find("moves") != std::string::npos) {
        int string_start = command.find("moves") + 6;
        std::string moves_substr = command.substr(string_start, std::string::npos);
        parse_moves(moves_substr, pos);
    }
}


// Uci function
void Uci_Loop(S_BOARD *pos, S_SEARCHINFO *info) {

    EngineOptions->UseBook = FALSE;

    // Definitions
    bool parsed_position = false;
    int MB = 512;
    int ths = 1;


    // Start uci
    printf("id name %s\n", NAME);
    printf("id author Aamar\n");
    printf("option name Hash type spin default 512 min 4 max %d\n", MAX_HASH);
    printf("option name OwnBook type check default false\n");
    printf("option name Threads type spin default 1 min 1 max %d\n", MAXTHREADS);
    printf("uciok\n");

    // Main loop
    while (TRUE) {

        std::string input;


        // get user / GUI input
        if (!std::getline(std::cin, input)) {
            // continue the loop
            break;
        }

        // make sure input is available
        if (!input.length()) {
            // continue the loop
            continue;
        }

        //Split the string into tokens to make it easier to work with
        std::vector<std::string> tokens = split_command(input);

        // parse UCI "position" command
        if (tokens[0] == "position") {
            // call parse position function
            ParsePosition(input, pos);
            parsed_position = true;
        }

            // parse UCI "go" command
        else if (tokens[0] == "go") {
            if (!parsed_position) // call parse position function
            {
                ParsePosition("position startpos", pos);
            }
            // call parse go function
            ParseGo(input, info, pos, HashTable);
        }
            // parse UCI "run" command
        else if (tokens[0] == "run") {
            if (!parsed_position) // call parse position function
            {
                ParsePosition("position fen 8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - -", pos);
            }
            // call parse go function
            ParseGo("go infinite", info, pos, HashTable);
        }
        else if (tokens[0] == "print") {
            PrintBoard(pos);
        }
            // parse UCI "isready" command
        else if (input == "isready") {
            std::cout << "readyok\n";

            continue;
        }

        else if (input == "stop")
        {
            JoinSearchThread(info);
        }

            // parse UCI "ucinewgame" command
        else if (input == "ucinewgame") {
            ClearHashTable(HashTable);
            ParsePosition("position startpos\n", pos);
        }

            // parse perft
        else if (tokens[0] == "perft") {
            std::cout << std::stoi(tokens.at(1));
            PerfTest(std::stoi(tokens.at(1)), pos);
        }

        else if (input == "quit") {
            info->quit = TRUE;
            JoinSearchThread(info);
            break;
        }
        else if (tokens[0] == "setoption") {
            //check tokens for size to see if we have a value
            if (tokens.size() < 5) {
                std::cout << "Invalid setoption format" << "\n";
                continue;
            }
            if (tokens.at(2) == "Hash") {
                MB = std::stoi(tokens.at(4));
                if (MB < 4) MB = 4;
                if (MB > MAX_HASH) MB = MAX_HASH;
                printf("Set Hash to %d MB\n", MB);
                InitHashTable(HashTable, MB);
            }

            if (tokens.at(2) == "Threads") {
                ths = std::stoi(tokens.at(4));
                if (ths < 1) ths = 1;
                if (ths > MAXTHREADS) ths = MAXTHREADS;
                printf("Set Threads to %d\n", ths);
                info->threadNum = ths;
            }
            else if (tokens.at(2) == "OwnBook") {
                if (tokens.at(4) == "true") {
                    EngineOptions->UseBook = TRUE;
                }
                else {
                    EngineOptions->UseBook = FALSE;
                }
            }
            else std::cout << "Unknown command: " << input << std::endl;
        }
        else if (input == "uci") {
            printf("id name %s\n", NAME);
            printf("id author Aamar\n");
            printf("uciok\n");
        }

        if (info->quit) break;
    }
}


