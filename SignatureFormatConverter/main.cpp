#include <iostream>
#include <string>
#include <regex>
#include <sstream>
#include <Windows.h>

// This converts other signature formats into the mod framework format and signatures in the mod framework format to the Cheat Engine format.

int main()
{
	bool showInfo = true;

	while (true)
	{
		if (showInfo)
		{
			std::cout << "Automatically converts the specified signature to the mod framework format or from the mod framework format to the Cheat Engine format." << std::endl;
			std::cout << "Enter signature:" << std::endl;
			showInfo = false;
		}

		std::string inputSignature = "";
		std::getline(std::cin, inputSignature);
		if (inputSignature == "")
		{
			continue;
		}

		// Removes leading, trailing and extra whitespaces
		inputSignature = std::regex_replace(inputSignature, std::regex("^ +| +$|( ) +"), "$1");

		std::stringstream stringStream(inputSignature);
		std::string token = "";
		std::vector<std::string> tokens;
		char delimiter = ' ';
		while (std::getline(stringStream, token, delimiter))
		{
			tokens.push_back(token);
		}

		std::string convertedSignature = "";
		for (auto token : tokens)
		{
			if (token[0] == '?')
			{
				token = "MASKED,";
			}
			else if (token == "MASKED,")
			{
				token = '?';
			}
			else if (token[0] == '0' and token[1] == 'x')
			{
				token = token.substr(2, 2);
			}
			else
			{
				token = "0x" + token + ",";
				std::transform(token.begin(), token.end(), token.begin(), ::tolower);
			}

			convertedSignature.append(token + " ");
		}

		if (convertedSignature[convertedSignature.length() - 2] == ',')
		{
			convertedSignature.erase(convertedSignature.length() - 1);
		}

		// Copy the converted signature to clipboard
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, convertedSignature.length());
		memcpy(GlobalLock(hMem), convertedSignature.c_str(), convertedSignature.length());
		GlobalUnlock(hMem);
		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();

		system("cls");
		std::cout << "Converted signature copied to clipboard. Convert another signature:" << std::endl;
	}

	return 0;
}