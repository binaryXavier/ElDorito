#include "GameConsole.h"

// TODO: why does pressing shift or caps lock break keyboard input?
// TODO: why is all input in capital letters?

void writeToMemory(uint8_t* const destination, const uint8_t bytesToPatch[], const size_t numOfBytes) { // TODO: use eldewrito method instead of this
	DWORD origProtect;
	VirtualProtect(destination, numOfBytes, PAGE_EXECUTE_READWRITE, &origProtect);
	memcpy(destination, bytesToPatch, numOfBytes);
	VirtualProtect(destination, numOfBytes, origProtect, 0);
}

GameConsole::GameConsole()
{
	for (int i = 0; i < numOfLines; i++) {
		queue.push_back("");
	}

	initPlayerName();
	DirectXHook directXHook(this);
	KeyboardHook hookKeyboard(this);
	IRCBackend ircBackEnd(this);

	while (true)
	{
		Sleep(60000); // TODO: remove; the reason I have this is because I don't want the items to be deleted from the stack because we have static hooks that access these instances
	}
}

bool GameConsole::isConsoleShown() {
	return boolShowConsole;
}

int GameConsole::getMsSinceLastConsoleOpen()
{
	return GetTickCount() - lastTimeConsoleShown;
}

void GameConsole::peekConsole()
{
	lastTimeConsoleShown = GetTickCount();
}

void GameConsole::hideConsole()
{
	lastTimeConsoleShown = GetTickCount();
	boolShowConsole = false;
	inputLine.clear();
	enableGameKeyboardInput();
}

void GameConsole::showConsole()
{
	boolShowConsole = true;
	disableGameKeyboardInput();
}

void GameConsole::enableGameKeyboardInput()
{
	uint8_t byteArrayBuffer[] = { 0x83, 0xEC, 0x14, 0x80, 0x3D };
	writeToMemory((uint8_t*) 0x512693, byteArrayBuffer, 5);
}

void GameConsole::disableGameKeyboardInput()
{
	// TODO: this only works 99.99% of the time
	uint8_t byteArrayBuffer[] = { 0xE9, 0x05, 0x03, 0x00, 0x00 }; // jmp 51299D
	writeToMemory((uint8_t*)0x512693, byteArrayBuffer, 5);
}

void GameConsole::virtualKeyCallBack(USHORT vKey)
{
	if (!isConsoleShown())
	{
		if (vKey == VK_RETURN)
		{
			showConsole();
		}
		return;
	}

	switch (vKey)
	{
	case VK_RETURN:
		if (!inputLine.empty())
		{
			pushLineFromKeyboardToGame(inputLine);
		}
		hideConsole();
		break;

	case VK_ESCAPE:
		hideConsole();
		break;

	case VK_BACK:
		if (!inputLine.empty())
		{
			inputLine.pop_back();
		}
		break;

	default:
		inputLine += MapVirtualKey(vKey, MAPVK_VK_TO_CHAR);
		break;
	}
}

std::string GameConsole::getInputLine()
{
	return inputLine;
}

void GameConsole::pushLineFromKeyboardToGame(std::string line)
{
	if (boost::algorithm::starts_with(line, "/"))
	{
		pushLineFromGameToUI(line);
		// TODO: sendToElDewrito();
	}
	else
	{
		std::string preparedLineForIRC = playerName;
		preparedLineForIRC += ": ";
		preparedLineForIRC += line;
		sendThisLineToIRCServer = preparedLineForIRC;
		pushLineFromGameToUI(preparedLineForIRC);
	}
}

void GameConsole::initPlayerName()
{
	wchar_t* inGameName = (wchar_t*)0x19A03E8; // unicode
	std::wstring toWstr(inGameName);
	std::string toStr(toWstr.begin(), toWstr.end());
	playerName = toStr;
}

void GameConsole::pushLineFromGameToUI(std::string line)
{
	if (queue.size() < (size_t)numOfLines)
	{
		queue.push_back(line);
	}
	else
	{
		for (int i = 0; i < numOfLines - 1; i++)
		{
			queue.at(i) = queue.at(i + 1);
		}
		queue.at(numOfLines - 1) = line;
	}

	peekConsole();
}

std::string GameConsole::getPlayerName()
{
	return playerName;
}

int GameConsole::getNumOfLines()
{
	return numOfLines;
}

std::string GameConsole::at(int i)
{
	return queue.at(i);
}