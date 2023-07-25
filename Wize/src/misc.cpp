#include "stdio.h"


#include "io.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <vector>


#ifdef WIN32
#include "windows.h"
#else
#include "sys/time.h"
#endif

// Get time elapsed in ms
int GetTimeMs() {
#ifdef WIN32
	return GetTickCount();
#else
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec * 1000 + t.tv_usec / 1000;
#endif
}


std::vector<std::string> split_command(const std::string& command)
{

	std::stringstream stream(command);
	std::string intermediate;
	std::vector<std::string> tokens;

	while (std::getline(stream, intermediate, ' '))
	{
		tokens.push_back(intermediate);
	}

	return tokens;
}

