#include "ShowGameUI.h"

#include "../ElDorito.h"
#include "../BlamTypes.h"
#include "../Utils/String.h"

#include <Windows.h>
#include <iostream>
#include <iostream>

ShowGameUI::ShowGameUI()
{
}

ShowGameUI::~ShowGameUI()
{
}

std::string ShowGameUI::Info(const std::string& Topic) const
{
	std::string Info = "Show Game UI dialogs/widgets"
		"\nUsage: show_ui (stringID)\n"
		"stringID should be the stringID of the UI element you want to show, in hexadecimal.";
	return Info;
}

std::string ShowGameUI::Suggest(const std::vector<std::string>& Arguments) const
{
	return "";
}

void ShowGameUI::Tick(const std::chrono::duration<double>& Delta)
{
}

bool ShowGameUI::Run(const std::vector<std::string>& Args)
{
	static uint8_t UIData[0x40];

	if (Args.size() < 2)
		return false;

	unsigned int stringID = std::stoul(Args[1], nullptr, 16);

	typedef void*(__thiscall * OpenUIDialogByIdFunc)(void* a1, unsigned int a2, int a3, int a4, int a5);

	// fill UIData with proper data
	OpenUIDialogByIdFunc openui = (OpenUIDialogByIdFunc)0xA92780;
	openui(&UIData, stringID, 0, 0, 0x1000C);

	// send UI notification
	uint32_t eax = (uint32_t)&UIData;
	uint32_t ecx = *(uint32_t*)0x5260254;
	*(DWORD*)(ecx + 8) = eax;

	eax = *(uint32_t*)0x5260254;
	eax = *(uint32_t*)eax;
	*(uint32_t*)0x5260254 = eax;

	return true;
}