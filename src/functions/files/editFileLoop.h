// PID::FILE_EDIT

#if HAS_PHYSICAL_KB

// Editor state
static String* _efLines = nullptr;
static int _efTotalLines = 0;
static int _efCursorLine = 0;
static int _efCursorCol = 0;
static int _efScrollOffset = 0;
static bool _efModified = false;

static const int EF_MAX_LINE_LEN = 128;
static const int EF_MAX_LINES = 256;

void _efFree() {
    if (_efLines != nullptr) { delete[] _efLines; _efLines = nullptr; }
    _efTotalLines = 0;
}

bool _efLoad() {
    _efFree();
    _efCursorLine = 0;
    _efCursorCol = 0;
    _efScrollOffset = 0;
    _efModified = false;

    bool useLittleFS = (selectedFileSourcePid != PID::FILE_PICKER_SD);
    File f = Storage::open(selectedFilePath.c_str(), "r", useLittleFS);

    _efLines = new String[EF_MAX_LINES];
    _efTotalLines = 0;

    if (f) {
        while (f.available() && _efTotalLines < EF_MAX_LINES) {
            String line = f.readStringUntil('\n');
            line.replace("\r", "");
            _efLines[_efTotalLines++] = line;
        }
        f.close();
    }

    if (_efTotalLines == 0) {
        _efLines[0] = "";
        _efTotalLines = 1;
    }
    return true;
}

bool _efSave() {
    bool ok = false;
    bool useLittleFS = (selectedFileSourcePid != PID::FILE_PICKER_SD);
    File f = Storage::open(selectedFilePath.c_str(), "w", useLittleFS);
    if (f) {
        for (int i = 0; i < _efTotalLines; i++) {
            f.print(_efLines[i]);
            if (i < _efTotalLines - 1) f.print('\n');
        }
        f.close();
        ok = true;
    }
    return ok;
}

// Wrap a logical line into display rows given the column width.
// Returns number of display rows produced; fills rows[] (max maxRows).
static int _efWrapLine(const String& line, int colWidth, String* rows, int maxRows) {
    if (line.length() == 0) {
        if (maxRows > 0) rows[0] = "";
        return 1;
    }
    int produced = 0;
    int start = 0;
    while (start < (int)line.length() && produced < maxRows) {
        rows[produced++] = line.substring(start, start + colWidth);
        start += colWidth;
    }
    return produced;
}

void _efDraw() {
    canvas.clear();
    canvas.setTextSize(TINY_TEXT);
    canvas.setTextColor(FGCOLOR, BGCOLOR);

    int lineH   = canvas.fontHeight();
    int canvasH = canvas.height();
    int canvasW = canvas.width();

    // Line number column: 1 digit wide for <10 lines, 2 digits otherwise
    int numColW = canvas.textWidth(_efTotalLines < 10 ? "9" : "99") + 4;
    int textPadding = 2; // gap between separator line and text
    // Separator line between line numbers and text area
    canvas.drawFastVLine(numColW - 1, 0, canvasH, TFT_DARKGREY);
    int textAreaW = canvasW - numColW - textPadding;

    // How many characters fit in one display row
    int charW = canvas.textWidth("W");
    int colWidth = (charW > 0) ? max(1, textAreaW / charW) : 20;

    int visibleRows = canvasH / lineH;

    // Count how many display rows each logical line takes (long lines wrap),
    // and find which display row the cursor is on
    int cursorDisplayRow = 0;
    int totalDisplayRows = 0;
    for (int i = 0; i < _efTotalLines; i++) {
        int len = _efLines[i].length();
        int wraps = (len == 0) ? 1 : (len + colWidth - 1) / colWidth;
        if (i == _efCursorLine)
            cursorDisplayRow = totalDisplayRows + _efCursorCol / colWidth;
        totalDisplayRows += wraps;
    }

    // Keep the cursor row visible by adjusting the scroll offset
    if (cursorDisplayRow < _efScrollOffset)
        _efScrollOffset = cursorDisplayRow;
    if (cursorDisplayRow >= _efScrollOffset + visibleRows)
        _efScrollOffset = cursorDisplayRow - visibleRows + 1;
    if (_efScrollOffset < 0) _efScrollOffset = 0;

    // Draw visible display rows
    int renderedRows = 0;
    int absRow = 0;
    String wrapBuf[8];
    for (int i = 0; i < _efTotalLines && renderedRows < visibleRows; i++) {
        int lineWraps = _efWrapLine(_efLines[i], colWidth, wrapBuf, 8);
        for (int sub = 0; sub < lineWraps; sub++, absRow++) {
            if (absRow < _efScrollOffset) continue;
            if (renderedRows >= visibleRows) break;

            int y = renderedRows * lineH;

            // Line number (only on the first display row of a logical line)
            canvas.setTextColor(TFT_DARKGREY, BGCOLOR);
            if (sub == 0) {
                String num = String(i + 1);
                int numW = canvas.textWidth(num.c_str());
                canvas.setCursor(numColW - numW - 2, y);
                canvas.print(num);
            }

            // Line text
            canvas.setTextColor(FGCOLOR, BGCOLOR);
            canvas.setCursor(numColW + textPadding, y);
            canvas.print(wrapBuf[sub].c_str());

            // Cursor
            if (i == _efCursorLine) {
                int cursorSubRow = _efCursorCol / colWidth;
                int cursorSubCol = _efCursorCol % colWidth;
                if (sub == cursorSubRow) {
                    int cx = numColW + textPadding + cursorSubCol * charW;
                    canvas.fillRect(cx, y, 2, lineH, FGCOLOR);
                }
            }

            renderedRows++;
        }
    }

    // Scrollbar
    if (totalDisplayRows > visibleRows) {
        int sbW = 3;
        int sbX = canvasW - sbW;
        int sliderH = max(4, canvasH * visibleRows / totalDisplayRows);
        int sliderY = (_efScrollOffset * (canvasH - sliderH)) / max(1, totalDisplayRows - visibleRows);
        canvas.fillRect(sbX, 0, sbW, canvasH, BGCOLOR);
        canvas.fillRect(sbX, sliderY, sbW, sliderH, FGCOLOR);
    }

    canvas.pushSprite(0, getStatusBarOffset());
}

// Insert a character at cursor position
void _efInsertChar(char c) {
    if (_efCursorLine >= _efTotalLines) return;
    String& line = _efLines[_efCursorLine];
    if ((int)line.length() >= EF_MAX_LINE_LEN) return;
    line = line.substring(0, _efCursorCol) + c + line.substring(_efCursorCol);
    _efCursorCol++;
    _efModified = true;
}

// Delete character before cursor (backspace)
void _efBackspace() {
    if (_efCursorCol > 0) {
        String& line = _efLines[_efCursorLine];
        line = line.substring(0, _efCursorCol - 1) + line.substring(_efCursorCol);
        _efCursorCol--;
        _efModified = true;
    } else if (_efCursorLine > 0) {
        // Merge with previous line
        int prevLen = _efLines[_efCursorLine - 1].length();
        _efLines[_efCursorLine - 1] += _efLines[_efCursorLine];
        for (int i = _efCursorLine; i < _efTotalLines - 1; i++)
            _efLines[i] = _efLines[i + 1];
        _efTotalLines--;
        _efCursorLine--;
        _efCursorCol = prevLen;
        _efModified = true;
    }
}

// Insert newline at cursor
void _efNewline() {
    if (_efTotalLines >= EF_MAX_LINES) return;
    String& cur = _efLines[_efCursorLine];
    String rest = cur.substring(_efCursorCol);
    cur = cur.substring(0, _efCursorCol);
    for (int i = _efTotalLines; i > _efCursorLine + 1; i--)
        _efLines[i] = _efLines[i - 1];
    _efTotalLines++;
    _efCursorLine++;
    _efLines[_efCursorLine] = rest;
    _efCursorCol = 0;
    _efModified = true;
}

void editFileLoop() {
    if (isSetup()) {
        kbTextMode = true;
        _efLoad();
        _efDraw();
        return;
    }

    bool changed = false;

    // Save: Ctrl+S equivalent — fn+s on Cardputer sends kbEnterPressed
    // We use ESC to exit (with save prompt) via kbEscPressed
    // and Enter for newline.

    if (kbEscPressed) {
        kbEscPressed = false;
        kbTextMode = false;
        if (_efModified) {
            bool ok = _efSave();
            centeredPrint(ok ? L->TXT_SUCCESS : L->TXT_ERROR, MEDIUM_TEXT);
            delay(700);
        }
        _efFree();
        changeProcess(PID::SELECTED_FILE_MENU);
        return;
    }

    if (kbEnterPressed) {
        kbEnterPressed = false;
        _efNewline();
        changed = true;
    }

    if (kbDelPressed) {
        kbDelPressed = false;
        _efBackspace();
        changed = true;
    }

    if (isKbCursorUpPressed()) {
        if (_efCursorLine > 0) {
            _efCursorLine--;
            _efCursorCol = min(_efCursorCol, (int)_efLines[_efCursorLine].length());
        }
        changed = true;
    }

    if (isKbCursorDownPressed()) {
        if (_efCursorLine < _efTotalLines - 1) {
            _efCursorLine++;
            _efCursorCol = min(_efCursorCol, (int)_efLines[_efCursorLine].length());
        }
        changed = true;
    }

    if (isKbCursorLeftPressed()) {
        if (_efCursorCol > 0) {
            _efCursorCol--;
        } else if (_efCursorLine > 0) {
            _efCursorLine--;
            _efCursorCol = _efLines[_efCursorLine].length();
        }
        changed = true;
    }

    if (isKbCursorRightPressed()) {
        if (_efCursorCol < (int)_efLines[_efCursorLine].length()) {
            _efCursorCol++;
        } else if (_efCursorLine < _efTotalLines - 1) {
            _efCursorLine++;
            _efCursorCol = 0;
        }
        changed = true;
    }

    for (char c : kbWord) {
        _efInsertChar(c);
        changed = true;
    }
    kbWord = "";

    if (changed) _efDraw();
}

#else // !HAS_PHYSICAL_KB

void editFileLoop() {
    if (isSetup()) {
        centeredPrint("no keyboard", MEDIUM_TEXT);
    }
    checkExit();
}

#endif
