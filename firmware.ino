#include "M5StickCPlus2.h"

#define DISP StickCP2.Display

uint16_t BGCOLOR=0x0001; // placeholder
uint16_t FGCOLOR=0xFFF1; // placeholder

struct MENU {
  char name[20];
  int command;
};
int cursor = 0;
int currentProc = 0;

void setup() {
  auto cfg = M5.config();
  StickCP2.begin(cfg);
  DISP.setRotation(1);
  DISP.setTextSize(1);
  DISP.setTextFont(&fonts::Orbitron_Light_24);
  mainMenu_drawMenu();
}

MENU mainMenu[] = {
  {"clock", 1},
  {"Battery info", 2},
  {"settings", 3},
};
int mainMenuSize = sizeof(mainMenu) / sizeof(MENU);

void mainMenu_drawMenu() {
  for (int i = 0; i < 3; i++) {
    DISP.print(cursor == i ? ">" : " ");
    DISP.println(mainMenu[i].name);
  }
}

void mainMenuLoop() {
  StickCP2.update();
  if (StickCP2.BtnB.wasPressed()) {
    DISP.clear();
    DISP.setCursor(0, 0);
    cursor++;
    if (cursor == mainMenuSize) cursor = cursor % mainMenuSize;
    mainMenu_drawMenu();
    StickCP2.Speaker.tone(8000, 20);
  }
  if (StickCP2.BtnA.wasPressed()) {
    DISP.clear();
    DISP.setCursor(0, 0);
    currentProc = mainMenu[cursor].command;
  }
}


int oldSeconds;
void clockLoop() {
  auto dt = StickCP2.Rtc.getDateTime();
  if (dt.time.seconds != oldSeconds) {
    // DISP.clear();
    DISP.setCursor(10, 40, 7);
    M5.Lcd.printf("%02d:%02d:%02d\n", dt.time.hours, dt.time.minutes, dt.time.seconds);
  }
  oldSeconds = dt.time.seconds;
}

int oldBattery;
void battery_drawMenu(int battery) {
  DISP.clear();
  DISP.setCursor(0, 0);
  DISP.print(battery);
}
void batteryLoop() {
  delay(500);
  int battery = StickCP2.Power.getBatteryLevel();
  if (battery != oldBattery) {
    battery_drawMenu(battery);
  }
  oldBattery = battery;
}

void settingsLoop() {
  DISP.setCursor(0, 0);
  DISP.print("Settings");

}

void loop() {
  switch (currentProc) {
    case 0:
      mainMenuLoop();
      break;
    case 1:
      clockLoop();
      break;
    case 2:
      batteryLoop();
      break;
    case 3:
      settingsLoop();
      break;
  }
}