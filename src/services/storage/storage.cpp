#include "storage.h"
#include "../../hal/sdPins.h"
#include <LittleFS.h>
#include <SD.h>
#include <SPI.h>

namespace Storage {

bool lfsBegun = false;
bool sdBegun = false;

// ── Mount / unmount ──

bool mountLittleFS() {
	if (lfsBegun) return true;
	lfsBegun = LittleFS.begin(true);
	return lfsBegun;
}

bool mountSD() {
	if (sdBegun) return true;
	SPI.begin(SD_CLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
	sdBegun = SD.begin(SD_CS_PIN);
	return sdBegun;
}

void unmountSD() {
	if (!sdBegun) return;
	SD.end();
	sdBegun = false;
}

bool isLittleFS() { return lfsBegun; }
bool isSD() { return sdBegun; }

bool autoDetect() {
	if (mountSD()) return false; // SD available → use SD
	return mountLittleFS();      // else fall back to LittleFS
}

// ── Backend routing ──

static fs::FS& fsFor(bool useLittleFS) {
	if (useLittleFS) { mountLittleFS(); return LittleFS; }
	mountSD(); return SD;
}

fs::FS& getFS(bool useLittleFS) {
	return fsFor(useLittleFS);
}

// ── File operations ──

File open(const char* path, const char* mode, bool useLittleFS) {
	return fsFor(useLittleFS).open(path, mode);
}

bool exists(const char* path, bool useLittleFS) {
	return fsFor(useLittleFS).exists(path);
}

bool remove(const char* path, bool useLittleFS) {
	return fsFor(useLittleFS).remove(path);
}

bool rename(const char* from, const char* to, bool useLittleFS) {
	return fsFor(useLittleFS).rename(from, to);
}

int list(const String& dirPath, bool useLittleFS, String*& outNames, bool*& outIsDir) {
	fs::FS& fs = fsFor(useLittleFS);

	File dir = fs.open(dirPath);
	if (!dir) return -1;

	// Count
	File f = dir.openNextFile();
	int count = 0;
	while (f) { count++; f = dir.openNextFile(); }

	if (count == 0) { outNames = nullptr; outIsDir = nullptr; return 0; }

	outNames = new String[count];
	outIsDir = new bool[count];

	// Collect
	dir = fs.open(dirPath);
	f = dir.openNextFile();
	for (int i = 0; i < count; i++) {
		outNames[i] = String(f.name());
		outIsDir[i] = f.isDirectory();
		f = dir.openNextFile();
	}

	// Sort: directories first (alphabetically), then files (alphabetically)
	for (int i = 0; i < count - 1; i++) {
		for (int j = i + 1; j < count; j++) {
			bool swap = false;
			if (!outIsDir[i] && outIsDir[j]) {
				swap = true;
			} else if (outIsDir[i] == outIsDir[j] && outNames[i].compareTo(outNames[j]) > 0) {
				swap = true;
			}
			if (swap) {
				String tmpName = outNames[i]; outNames[i] = outNames[j]; outNames[j] = tmpName;
				bool tmpDir = outIsDir[i]; outIsDir[i] = outIsDir[j]; outIsDir[j] = tmpDir;
			}
		}
	}

	return count;
}

bool readString(const String& path, String& out, bool useLittleFS) {
	out = "";
	File f = open(path.c_str(), "r", useLittleFS);
	if (!f) return false;
	out = f.readString();
	f.close();
	return out.length() > 0;
}

bool readLines(const String& path, String*& outLines, int& outCount, bool useLittleFS) {
	outLines = nullptr;
	outCount = 0;

	File f = open(path.c_str(), "r", useLittleFS);
	if (!f) return false;

	int count = 0;
	while (f.available()) {
		String line = f.readStringUntil('\n');
		line.trim();
		if (line.length() > 0) count++;
	}
	if (count == 0) { f.close(); return false; }

	outLines = new String[count];
	outCount = count;

	f.seek(0);
	int i = 0;
	while (f.available() && i < count) {
		String line = f.readStringUntil('\n');
		line.trim();
		if (line.length() > 0) outLines[i++] = line;
	}
	f.close();
	return true;
}

String uniquePath(const String& basePath, const String& ext, bool useLittleFS) {
	fs::FS& fs = fsFor(useLittleFS);
	String path;
	int n = 1;
	do {
		path = basePath + "_" + String(n) + ext;
		n++;
	} while (fs.exists(path));
	return path;
}

File openUnique(const String& basePath, const String& ext, bool useLittleFS) {
	String path = uniquePath(basePath, ext, useLittleFS);
	return fsFor(useLittleFS).open(path.c_str(), FILE_WRITE);
}

}
