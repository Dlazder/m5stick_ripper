// PID::FILE_DELETE

void deleteFileLoop() {
	if (isSetup()) {
		String name = selectedFilePath.substring(selectedFilePath.lastIndexOf('/') + 1);
		String lines[] = { "delete file?", name};
		centeredPrintRows(lines, 2, SMALL_TEXT, true);
		drawHintCustom("enter: delete", "A: delete");
	}

	if (isBtnAWasPressed() || isKbEnterPressed()) {
		bool useLittleFS = (selectedFileSourcePid != PID::FILE_PICKER_SD);
		bool ok = Storage::remove(selectedFilePath.c_str(), useLittleFS);

		const char* result = ok ? L->TXT_SUCCESS : L->TXT_ERROR;
		centeredPrint(result, MEDIUM_TEXT);
		delay(800);
		selectedFilePath = "";
		changeProcess(selectedFileSourcePid);
		return;
	}

	checkExit();
}
