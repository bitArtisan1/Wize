#include <iostream>
#include <sstream>
#include <chrono>
#include <vector>

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
