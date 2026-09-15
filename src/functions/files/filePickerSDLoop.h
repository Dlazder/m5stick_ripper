// PID::FILE_PICKER_SD

int sdFileCount = 0;
MENU* sdFileMenu = nullptr;
String* sdFileFullPaths = nullptr;
String sdCurrentDir = "/";

void _sdBuildMenu() {
	if (sdFileMenu != nullptr) { delete[] sdFileMenu; sdFileMenu = nullptr; }
	if (sdFileFullPaths != nullptr) { delete[] sdFileFullPaths; sdFileFullPaths = nullptr; }
	sdFileCount = 0;

	if (!Storage::mountSD()) { centeredPrint("SD error", MEDIUM_TEXT); return; }

	String* names = nullptr;
	bool* isDir = nullptr;
	sdFileCount = Storage::list(sdCurrentDir, false, names, isDir);
	if (sdFileCount < 0) return;

	sdFileMenu = new MENU[sdFileCount + 2];
	sdFileFullPaths = new String[sdFileCount];

	sdFileMenu[0] = { PID::FILES_MENU, L->MENU_BACK, Icons::back };
	sdFileMenu[1] = { PID::FILE_CREATE, "create", Icons::create };

	for (int i = 0; i < sdFileCount; i++) {
		sdFileFullPaths[i] = _fpMakePath(sdCurrentDir, names[i]);
		int cmd = isDir[i] ? PID::FILE_PICKER_SD : PID::SELECTED_FILE_MENU;
		sdFileMenu[i + 2] = { cmd, isDir[i] ? "/" + names[i] : names[i], isDir[i] ? Icons::folder : Icons::file };
	}

	delete[] names;
	delete[] isDir;
}

void filePickerSDLoop() {
	if (isSetup()) {
		cursor = 0;
		sdCurrentDir = "/";
		_sdBuildMenu();
		if (sdFileMenu == nullptr) return;
		drawMenu(sdFileMenu, sdFileCount + 2);
	}

	if (sdFileMenu == nullptr) {
		checkExit(PID::FILES_MENU);
		return;
	}

	int totalItems = sdFileCount + 2;

	if (isBtnBWasPressed() || isKbDownPressed() || isWebControlDownWasPressed()) {
		cursor++;
		drawMenu(sdFileMenu, totalItems);
	}
	if (isBtnPWRWasPressed() || isKbUpPressed() || isWebControlUpWasPressed()) {
		cursor--;
		drawMenu(sdFileMenu, totalItems);
	}

	if (isBtnAWasPressed() || isKbEnterPressed()) {
		if (cursor == 0) {
			if (sdCurrentDir == "/") {
				changeProcess(PID::FILES_MENU);
			} else {
				_goParentDir(sdCurrentDir);
				_sdBuildMenu();
				cursor = 0;
				drawMenu(sdFileMenu, sdFileCount + 2);
			}
			return;
		}

		if (cursor == 1) {
			createFileCurrentDir = sdCurrentDir;
			changeProcess(PID::FILE_CREATE);
			return;
		}

		String selectedPath = sdFileFullPaths[cursor - 2];

		if (sdFileMenu[cursor].name.startsWith("/")) {
			sdCurrentDir = selectedPath;
			_sdBuildMenu();
			cursor = 0;
			drawMenu(sdFileMenu, sdFileCount + 2);
		} else {
			selectedFilePath = selectedPath;
			changeProcess(sdFileMenu[cursor].command);
		}
		return;
	}

	if (isWebDataRequested()) {
		webData = generateWebData("menu", generateMenuString(sdFileMenu, totalItems));
	}
}
