
#include "Preferences.h"
#define MAX_STEPS 16

typedef struct StepType
{
  String name;
  float temperature;
  float duration;
  boolean capture;
};

typedef struct ProtocolType
{

  StepType steps[MAX_STEPS];
  String name = "undefined";
  String date = "undefined";
  int stepCount = 0;
  int cycleCount = 0;
  int repeatStart = 0;
  int repeatEnd = 0;
};

extern ProtocolType pcrProtocol;

void loadProtocol();
