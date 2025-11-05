#include <Adafruit_FT6206.h>

#define COLOR_DEPTH 1  // Colour depth (1, 8 or 16 bits per pixel)

enum SystemState {
CASE_Main,
CASE_RunMeasurement,
CASE_EditSettings,
CASE_RunQPCR,
CASE_AbortRunQPCR,
CASE_RunPotocolDisplay,
CASE_RunSettingsDisplay,
CASE_InitMeasurement,
CASE_InitCalibration,
CASE_RunCalibration,
CASE_InitQPCR,
CASE_RunSubMenuDisplay,
CASE_RunComplete,
CASE_InitBaseline,
CASE_RunBaseline,
CASE_RunInitUSB,
CASE_InitRun
};



extern SystemState caseUX;

bool PointInRect(TS_Point point, int x, int y, int h, int v);

void drawMainDisplay();

void draw_qLAMP_display();

void drawInitUSBDisplay();

void drawCalibrationDisplay();

void drawBaselineDisplay();

void draw_WIFI_display();

void drawProtocolDisplay();

void setPrototcolDisplayFrame(int frame);

void runProtocolDisplay();

void drawSubMenuDisplay();

void runSubMenuDisplay();

void drawSettingsDisplay();

void runSettingsDisplay();

void drawRunCompleteDisplay();

void drawMeasurementDisplay();

void drawInitRunDisplay(float voltage);

void draw_settings_display();

void drawBackButton();

void status_line(String printtext, bool status_flag, String extra_text="");
