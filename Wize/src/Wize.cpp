#include <iostream>
#include "defs.h"
#include <cstring>

int main(int argc, char *argv[])
{

    std::cout << R"(
 ╭───────────────────────────────────────────────╮
 │                                     (\=,      │
 │                                    //  .\     │
 │                                   (( \_  \    │
 │                                    ))  `\_)   │
 │   ██╗    ██╗██╗███████╗███████╗   (/     \    │
 │   ██║    ██║██║╚══███╔╝██╔════╝    | _.-'|    │
 │   ██║ █╗ ██║██║  ███╔╝ █████╗       )___(     │
 │   ██║███╗██║██║ ███╔╝  ██╔══╝      (=====)    │
 │   ╚███╔███╔╝██║███████╗███████╗    }====={    │
 │    ╚══╝╚══╝ ╚═╝╚══════╝╚══════╝   (_______)   │
 │            by: yanpuri                        │
 ╰───────────────────────────────────────────────╯
    )" << std::endl;
	AllInit();
	// Start board
	S_BOARD pos[1];
	S_SEARCHINFO info[1];

	info->quit = FALSE;
	info->threadNum = 4; // Default 1 thread


	HashTable->pTable = NULL;
	InitHashTable(HashTable, 1024);
	std::string input;
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
    std::cout << "> Wize started successfully. Enter '\033[38;5;129muci\033[0m' to get started...\n";
    char line[256];
    while (TRUE) {
        memset(&line[0], 0, sizeof(line));

        fflush(stdout);
        if (!fgets(line, 256, stdin))
            continue;
        if (line[0] == '\n')
            continue;
        if (!strncmp(line, "uci",3)) {
            Uci_Loop(pos, info);
            if(info->quit == TRUE) break;
            continue;
        } else if(!strncmp(line, "quit",4))	{
            break;
        }
          else {
            std::cout << "Unknown command: " << line << std::endl;
            continue;
        }
    }
	// Clean allocated memory
	free(HashTable->pTable);
	CleanPolyBook();
	return 0;
};



