// Storage service — unified filesystem access over LittleFS and SD.
//
// Mirrors Flipper Zero's storage service: callers talk to one API and pass a
// backend flag instead of branching on LittleFS/SD themselves.
//
// Backend resolution (the `useLittleFS` flag):
// open(path, mode)				> auto: SD if a card is present, else LittleFS
// open(path, mode, true) > force LittleFS
// open(path, mode, false) > force SD (invalid File if no card)
//
// Read paths that act on a user-picked file (file picker) must pass the
// backend explicitly; only new captures/saves should rely on the auto default.

#pragma once
#include <Arduino.h>
#include <FS.h>

namespace Storage {

// Mount / unmount 

/// Mounts LittleFS (idempotent). Returns true when available.
bool mountLittleFS();

/// Mounts SD over SPI (idempotent). Returns true when a card is present.
bool mountSD();

/// Unmounts SD, releasing the SPI bus and its pins.
void unmountSD();

bool isLittleFS();
bool isSD();

/// Default backend for new captures: SD if a card is present, else LittleFS.
/// May mount on first call — this is the lazy-mount entry point.
bool autoDetect();

/// Returns the resolved `fs::FS` reference for callers that need the raw
/// backend object (e.g. M5GFX image decoders). Mounts if necessary.
fs::FS& getFS(bool useLittleFS = autoDetect());

// File operations 

/// Opens a file on the resolved backend. See header note for flag semantics.
File open(const char* path, const char* mode, bool useLittleFS = autoDetect());

bool exists(const char* path, bool useLittleFS = autoDetect());
bool remove(const char* path, bool useLittleFS = autoDetect());
bool rename(const char* from, const char* to, bool useLittleFS = autoDetect());

/// Scans a directory, returning sorted entry names + directory flags.
/// Directories come first, then files; each group sorted alphabetically.
/// @return number of entries, or -1 on error (caller frees both arrays).
int list(const String& dirPath, bool useLittleFS, String*& outNames, bool*& outIsDir);

/// Reads a whole file into `out`. Backend is explicit (must not auto-detect).
bool readString(const String& path, String& out, bool useLittleFS);

/// Reads a text file line-by-line into a caller-freed String[].
/// Blank lines are skipped. Backend is explicit (see readString).
bool readLines(const String& path, String*& outLines, int& outCount, bool useLittleFS);

/// Generates a unique path by appending "_N" before the extension.
String uniquePath(const String& basePath, const String& ext, bool useLittleFS = autoDetect());

/// Opens a new uniquely-named file for writing on the resolved backend.
File openUnique(const String& basePath, const String& ext, bool useLittleFS = autoDetect());

} // namespace Storage
