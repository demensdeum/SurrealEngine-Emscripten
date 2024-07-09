
#include "Precomp.h"
#include "Exception.h"
#include "GameApp.h"
#include "CommandLine.h"
#include "GameFolder.h"
#include "Engine.h"
#include "UI/WidgetResourceData.h"
#include "File.h"
#include <stdexcept>
#include <zwidget/core/theme.h>
#include <zwidget/window/window.h>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

#ifdef EMSCRIPTEN
#include <emscripten.h>
Engine *EMSCRIPTEN_GLOBAL_GAME_ENGINE = nullptr;
void emscripten_game_loop_step() {
	EMSCRIPTEN_GLOBAL_GAME_ENGINE->Run();
}
#endif

int GameApp::main(std::vector<std::string> args)
{

	{
    	std::string path = "."; // current directory

    	try {
        	for (const auto & entry : fs::directory_iterator(path)) {
            	std::cout << entry.path().filename().string() << std::endl;
        	}
    	} catch (const fs::filesystem_error& e) {
        	std::cerr << "Error: " << e.what() << std::endl;
    	}
	}

	std::cout << "GameApp main" << std::endl;	
	args.clear();
	args.push_back("SurrealEngine");
	args.push_back("UnrealTournament");
	//args.push_back("--url=DM-Cube-Test-Rendering.unr");
	args.push_back("--url=DM-Deck16][.unr");

	std::cout << "DisplayBackend::TryCreateSDL2()" << std::endl;	
	auto backend = DisplayBackend::TryCreateSDL2();
	std::cout << "DisplayBackend::Set(std::move(backend))" << std::endl;
	DisplayBackend::Set(std::move(backend));
	std::cout << "InitWidgetResources()" << std::endl;
	InitWidgetResources();
	std::cout << "WidgetTheme::SetTheme" << std::endl;
	WidgetTheme::SetTheme(std::make_unique<DarkWidgetTheme>());

	std::cout << "Args" << std::endl;

	CommandLine cmd(args);
	commandline = &cmd;

	GameLaunchInfo info = GameFolderSelection::GetLaunchInfo();

#ifdef EMSCRIPTEN
		EMSCRIPTEN_GLOBAL_GAME_ENGINE = new Engine(info);
		emscripten_set_main_loop(emscripten_game_loop_step, 0, 0);
#else
	if (!info.gameRootFolder.empty())
	{
		std::cout << "Engine" << std::endl;
		Engine engine(info);
		std::cout << "Run Engine" << std::endl;
		engine.Run();
	}

	DeinitWidgetResources();
#endif
	return 0;
}
