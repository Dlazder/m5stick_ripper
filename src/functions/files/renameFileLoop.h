// PID::FILE_RENAME

void renameFileLoop() {
	if (isSetup()) {
		kbReset();
		drawKeyboardUi();
	}

	keyboardLoop(
		[]() {
			changeProcess(PID::SELECTED_FILE_MENU);
		},
		[](const char* buf) {
			if (kbLen == 0) {
				changeProcess(PID::SELECTED_FILE_MENU);
				return;
			}

			String dir = selectedFilePath.substring(0, selectedFilePath.lastIndexOf('/') + 1);
			String newPath = dir + String(buf);
			bool ok = false;

			ok = Storage::rename(selectedFilePath.c_str(), newPath.c_str(), selectedFileSourcePid != PID::FILE_PICKER_SD);

			if (ok) selectedFilePath = newPath;

			const char* result = ok ? L->TXT_SUCCESS : L->TXT_ERROR;
			centeredPrint(result, MEDIUM_TEXT);
			delay(800);
			changeProcess(PID::SELECTED_FILE_MENU);
		},
		nullptr
	);
}
