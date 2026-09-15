// PID::FILE_PICKER

int lfsFileCount = 0;
MENU* lfsFileMenu = nullptr;
String* lfsFileFullPaths = nullptr;
String lfsCurrentDir = "/";
static int _lfsStoredReturnPid = 0;
static int _lfsStoredCancelPid = 0;

void _lfsBuildMenu() {
	if (lfsFileMenu != nullptr) { delete[] lfsFileMenu; lfsFileMenu = nullptr; }
	if (lfsFileFullPaths != nullptr) { delete[] lfsFileFullPaths; lfsFileFullPaths = nullptr; }
	lfsFileCount = 0;

	if (!Storage::mountLittleFS()) { centeredPrint("LittleFS error", MEDIUM_TEXT); return; }

	String* names = nullptr;
	bool* isDir = nullptr;
	lfsFileCount = Storage::list(lfsCurrentDir, true, names, isDir);
	if (lfsFileCount < 0) return;

	lfsFileMenu = new MENU[lfsFileCount + 2];
	lfsFileFullPaths = new String[lfsFileCount];

	int backPid = (lfsCurrentDir == "/")
		? (_lfsStoredCancelPid != 0 ? _lfsStoredCancelPid : PID::FILES_MENU)
		: 0;
	int filePid = (_lfsStoredReturnPid != 0) ? _lfsStoredReturnPid : PID::SELECTED_FILE_MENU;

	lfsFileMenu[0] = { backPid, L->MENU_BACK, Icons::back };
	lfsFileMenu[1] = { PID::FILE_CREATE, "create", Icons::create };

	for (int i = 0; i < lfsFileCount; i++) {
		lfsFileFullPaths[i] = _fpMakePath(lfsCurrentDir, names[i]);
		int cmd = isDir[i] ? PID::FILE_PICKER : filePid;
		lfsFileMenu[i + 2] = { cmd, isDir[i] ? "/" + names[i] : names[i], isDir[i] ? Icons::folder : Icons::file };
	}

	delete[] names;
	delete[] isDir;
}

void filePickerLFSLoop() {
	if (isSetup()) {
		_lfsStoredReturnPid = lfsReturnPid;
		_lfsStoredCancelPid = lfsCancelPid;
		lfsReturnPid = 0;
		lfsCancelPid = 0;
		cursor = 0;
		lfsCurrentDir = "/";
		_lfsBuildMenu();
		if (lfsFileMenu == nullptr) return;
		drawMenu(lfsFileMenu, lfsFileCount + 2);
	}

	if (lfsFileMenu == nullptr) {
		checkExit(PID::FILES_MENU);
		return;
	}

	int totalItems = lfsFileCount + 2;

	if (isBtnBWasPressed() || isKbDownPressed() || isWebControlDownWasPressed()) {
		cursor++;
		drawMenu(lfsFileMenu, totalItems);
	}
	if (isBtnPWRWasPressed() || isKbUpPressed() || isWebControlUpWasPressed()) {
		cursor--;
		drawMenu(lfsFileMenu, totalItems);
	}

	if (isBtnAWasPressed() || isKbEnterPressed()) {
		if (cursor == 0) {
			if (lfsCurrentDir == "/") {
				changeProcess(_lfsStoredCancelPid != 0 ? _lfsStoredCancelPid : PID::FILES_MENU);
			} else {
				_goParentDir(lfsCurrentDir);
				_lfsBuildMenu();
				cursor = 0;
				drawMenu(lfsFileMenu, lfsFileCount + 2);
			}
			return;
		}

		if (cursor == 1) {
			createFileCurrentDir = lfsCurrentDir;
			changeProcess(PID::FILE_CREATE);
			return;
		}

		String selectedPath = lfsFileFullPaths[cursor - 2];

		if (lfsFileMenu[cursor].name.startsWith("/")) {
			lfsCurrentDir = selectedPath;
			_lfsBuildMenu();
			cursor = 0;
			drawMenu(lfsFileMenu, lfsFileCount + 2);
		} else {
			selectedFilePath = selectedPath;
			changeProcess(lfsFileMenu[cursor].command);
		}
		return;
	}

	if (isWebDataRequested()) {
		webData = generateWebData("menu", generateMenuString(lfsFileMenu, totalItems));
	}
}
