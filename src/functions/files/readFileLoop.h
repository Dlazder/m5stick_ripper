// PID::FILE_VIEW

static int _rfScrollOffset = 0;
static int _rfTotalLines = 0;
static String* _rfLines = nullptr;
static bool _rfIsImage = false;
static float _rfZoom = 0.0f; // 0 = auto-fit; >=1.0 = zoom multiplier
static int32_t _rfPanX = 0; // pan offset in rendered pixels
static int32_t _rfPanY = 0;

void _rfFree() {
	if (_rfLines != nullptr) { delete[] _rfLines; _rfLines = nullptr; }
	_rfTotalLines = 0;
	_rfIsImage = false;
	_rfZoom = 0.0f;
	_rfPanX = 0;
	_rfPanY = 0;
}

void _rfLoad() {
	_rfFree();
	_rfScrollOffset = 0;

	bool useLittleFS = (selectedFileSourcePid != PID::FILE_PICKER_SD);
	File f = Storage::open(selectedFilePath.c_str(), "r", useLittleFS);

	if (!f) { centeredPrint("open error", MEDIUM_TEXT); return; }

	int count = 0;
	while (f.available()) {
		f.readStringUntil('\n');
		count++;
	}
	_rfTotalLines = count;

	canvas.setTextSize(TINY_TEXT);
	int maxW = canvas.width() - 5 - 5 - 5; // left pad + scrollbar + margin
	int charW = canvas.textWidth("W");
	int maxChars = (charW > 0) ? (maxW / charW) : 32;

	_rfLines = new String[count];
	f.seek(0);
	for (int i = 0; i < count && f.available(); i++) {
		_rfLines[i] = f.readStringUntil('\n');
		_rfLines[i].trim();
		if (int(_rfLines[i].length()) > maxChars)
			_rfLines[i] = _rfLines[i].substring(0, maxChars);
	}
	f.close();
}

void _rfDraw() {
	canvas.clear();
	canvas.setTextColor(FGCOLOR, BGCOLOR);
	canvas.setTextSize(TINY_TEXT);
	canvas.setCursor(0, 0);

	int paddingX = 5;
	int lineHeight = canvas.fontHeight();
	int visibleLines = (canvas.height() - paddingX) / lineHeight;

	for (int i = 0; i < visibleLines; i++) {
		int idx = _rfScrollOffset + i;
		if (idx >= _rfTotalLines) break;
		canvas.setCursor(paddingX, i * lineHeight);
		canvas.print(_rfLines[idx].c_str());
	}

	// scrollbar: slider height = visible/total, position = offset/scrollable
	if (_rfTotalLines > visibleLines) {
		int sbW = 5;
		int sbX = canvas.width() - sbW;
		int sbH = canvas.height();
		int sliderH = max(6, sbH * visibleLines / _rfTotalLines);
		int scrollable = _rfTotalLines - visibleLines;
		int sliderY = (_rfScrollOffset * (sbH - sliderH)) / scrollable;
		canvas.fillRect(sbX, 0, sbW, sbH, BGCOLOR);
		canvas.fillRect(sbX, sliderY, sbW, sliderH, FGCOLOR);
	}
	canvas.pushSprite(0, getStatusBarOffset());
}

static bool _rfIsImageExt(const String& path) {
	int dot = path.lastIndexOf('.');
	if (dot < 0) return false;
	String ext = path.substring(dot);
	ext.toLowerCase();
	return ext == ".jpg" || ext == ".jpeg" || ext == ".png"
		|| ext == ".bmp" || ext == ".qoi";
}

static bool _rfDrawImage() {
	canvas.clear();

	// Must pass fs::FS& (not sd::SDFS& or LittleFSFS&) so the M5GFX template
	// instantiates DataWrapperT<fs::FS>, which has a working specialization.
	fs::FS& filesys = Storage::getFS(selectedFileSourcePid != PID::FILE_PICKER_SD);

	String ext = selectedFilePath.substring(selectedFilePath.lastIndexOf('.'));
	ext.toLowerCase();

	// Zoom: when _rfZoom > 0, scale the effective fit-area so M5GFX auto-fit
	// produces a larger zoom. Pan: offX/offY skip rendered pixels from edges.
	int32_t maxW = canvas.width();
	int32_t maxH = canvas.height();
	float scaleX = 0.0f; // 0 = auto-fit in M5GFX

	if (_rfZoom > 0.001f) {
		maxW = roundf(canvas.width() * _rfZoom);
		maxH = roundf(canvas.height() * _rfZoom);
		scaleX = 0.0f;
	}

	bool ok = false;

	if (ext == ".jpg" || ext == ".jpeg") {
		ok = canvas.drawJpgFile(filesys, selectedFilePath.c_str(), 0, 0, maxW, maxH, _rfPanX, _rfPanY, scaleX);
	} else if (ext == ".png") {
		ok = canvas.drawPngFile(filesys, selectedFilePath.c_str(), 0, 0, maxW, maxH, _rfPanX, _rfPanY, scaleX);
	} else if (ext == ".bmp") {
		ok = canvas.drawBmpFile(filesys, selectedFilePath.c_str(), 0, 0, maxW, maxH, _rfPanX, _rfPanY, scaleX);
	} else if (ext == ".qoi") {
		ok = canvas.drawQoiFile(filesys, selectedFilePath.c_str(), 0, 0, maxW, maxH, _rfPanX, _rfPanY, scaleX);
	}

	if (ok) canvas.pushSprite(0, getStatusBarOffset());
	return ok;
}

void readFileLoop() {
	if (isSetup()) {
		if (_rfIsImageExt(selectedFilePath)) {
			_rfIsImage = true;
			_rfZoom = 0.0f;
			_rfPanX = 0;
			_rfPanY = 0;
			if (!_rfDrawImage()) {
				centeredPrint("Can't display image", MEDIUM_TEXT);
			}
		} else {
			_rfLoad();
			if (_rfLines == nullptr) centeredPrint("File empty", MEDIUM_TEXT);
			else _rfDraw();
		}
	}

	// Image
	if (_rfIsImage) {
		// Zoom
		if (isKbPlusPressed() || isBtnAWasPressed()) {
			_rfZoom = (_rfZoom < 1.0f) ? 1.0f : _rfZoom * 1.25f;
			if (_rfZoom > 8.0f) _rfZoom = 8.0f;
			_rfDrawImage();
		}
		if (isKbMinusPressed() || isBtnPWRWasPressed()) {
			if (_rfZoom > 0.001f) {
				_rfZoom /= 1.25f;
				if (_rfZoom < 1.0f) _rfZoom = 0.0f;
				_rfDrawImage();
			}
		}

		// Pan
		if (_rfZoom > 0.001f) {
			int step = max(10, int(roundf(20.0f * _rfZoom)));
			if (isKbLeftPressed() && _rfPanX > 0) { _rfPanX -= step; _rfDrawImage(); }
			if (isKbRightPressed()) { _rfPanX += step; _rfDrawImage(); }
			if (isKbUpPressed() && _rfPanY > 0) { _rfPanY -= step; _rfDrawImage(); }
			if (isKbDownPressed()) { _rfPanY += step; _rfDrawImage(); }
		} else {
			_rfPanX = 0;
			_rfPanY = 0;
		}

		checkExit();
		return;
	}

	// Text
	if (_rfLines == nullptr) {
		checkExit();
		return;
	}

	int visibleLines = canvas.height() / canvas.fontHeight();

	if (isBtnAWasPressed() || isKbDownPressed()) {
		if (_rfScrollOffset + visibleLines < _rfTotalLines) {
			_rfScrollOffset++;
			_rfDraw();
		}
	} else if (isBtnPWRWasPressed() || isKbUpPressed()) {
		if (_rfScrollOffset > 0) {
			_rfScrollOffset--;
			_rfDraw();
		}
	}

	if (checkExit()) {
		_rfFree();
	}
}
