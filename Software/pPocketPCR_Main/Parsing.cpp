
#include "Preferences.h"
#include "Parsing.h"
#include "USB_DRIVE.h"

ProtocolType pcrProtocol;

float getFloatFromString(String InputString)
{
  // Extract the substring containing the float number
  String floatString = InputString;
  floatString.trim();
  // Convert the extracted substring to a float
  float extractedFloat = floatString.toFloat();

  return extractedFloat;
}

float getSecondFloatFromString(String InputString)
{
  int idx = 0;

  while (idx < InputString.length() && isdigit(InputString.charAt(idx)))
    idx++;
  while (idx < InputString.length() && !isdigit(InputString.charAt(idx)))
    idx++;

  // Extract the substring containing the float number
  String floatString = InputString.substring(idx, InputString.length());
  ;
  floatString.trim();

  // Convert the extracted substring to a float
  float extractedFloat = floatString.toFloat();

  return extractedFloat;
}

String getStringInBrackets(String InputString)
{
  int startPos = InputString.indexOf("(");
  int endPos = InputString.indexOf(")");
  String floatString = "";
  if ((startPos != -1) && (endPos != -1))
  {
    // Extract the substring containing the float number
    floatString = InputString.substring(startPos + 1, endPos);
    floatString.trim();
    // Convert the extracted substring to a float
  }
  return floatString;
}

boolean isFarenheit(String InputString)
{
  return (InputString.indexOf("f") != -1) || (InputString.indexOf("F") != -1);
}

boolean isMinutes(String InputString)
{
  return (InputString.indexOf("m") != -1) || (InputString.indexOf("M") != -1);
}

String getStringFromStream(String InputString, String InputStream)

{
  String floatString = "";
  // Find the position of the float number in the string
  int startPos = InputStream.indexOf(InputString);
  if (startPos != -1)
  {
    floatString = InputStream.substring(startPos + InputString.length());
    int endPos = floatString.indexOf("\n");
    floatString = floatString.substring(0, endPos);
    floatString.trim();
  }
  return floatString;
}

void getSteps(String InputStream)

{
  int startStep = InputStream.indexOf("STEP");
  int endStep = 0;
  int stepCount = 0;

  String floatString;

  while ((startStep != -1) && (stepCount < MAX_STEPS))
  {

    floatString = InputStream.substring(startStep + 4);
    endStep = floatString.indexOf("STEP");

    if (endStep != -1)
    {
      floatString = floatString.substring(0, endStep);
      startStep = startStep + 4 + endStep;
    }
    else
    {
      startStep = endStep;
    }

    pcrProtocol.steps[stepCount].name = getStringFromStream(":", floatString);
    // Serial.println( (int)getFloatFromString(getStringFromStream("STEP",floatString)));

    pcrProtocol.steps[stepCount].temperature = getFloatFromString(getStringFromStream("TEMPERATURE:", floatString));
    if (isFarenheit(getStringFromStream("TEMPERATURE:", floatString)))
      pcrProtocol.steps[stepCount].temperature = (5.0 / 9.0) * (pcrProtocol.steps[stepCount].temperature - 32.0);
    ;
    pcrProtocol.steps[stepCount].duration = getFloatFromString(getStringFromStream("DURATION:", floatString));
    if (pcrProtocol.steps[stepCount].temperature > 99)
      pcrProtocol.steps[stepCount].temperature = 99;
    if (pcrProtocol.steps[stepCount].temperature < 20)
      pcrProtocol.steps[stepCount].temperature = 20;

    if (isMinutes(getStringFromStream("DURATION:", floatString)))
      pcrProtocol.steps[stepCount].duration = pcrProtocol.steps[stepCount].duration * 60;
    if ((getStringFromStream("CAPTURE", floatString)) != "")
      pcrProtocol.steps[stepCount].capture = true;
    else
      pcrProtocol.steps[stepCount].capture = false;

    // Serial.println( pcrProtocol.steps[stepCount].name);
    // Serial.println( pcrProtocol.steps[stepCount].temperature);
    // Serial.println( pcrProtocol.steps[stepCount].duration);

    stepCount++;
  }

  pcrProtocol.stepCount = stepCount;
}

void parseConfig(String InputStream)
{

  pcrProtocol.stepCount = 0;

  if (InputStream.length() > 0)
  {
    pcrProtocol.name = getStringFromStream("NAME:", InputStream);
    pcrProtocol.date = getStringFromStream("DATE:", InputStream);
    pcrProtocol.repeatStart = getFloatFromString(getStringFromStream("REPEAT:", InputStream));
    pcrProtocol.repeatEnd = getSecondFloatFromString(getStringFromStream("REPEAT:", InputStream));
    if (pcrProtocol.repeatEnd <= pcrProtocol.repeatStart)
      pcrProtocol.repeatEnd = pcrProtocol.repeatStart;
    pcrProtocol.cycleCount = getFloatFromString(getStringFromStream("CYCLES:", InputStream));

    getSteps(InputStream);

    /*  Serial.print("Name");
    // Serial.println(pcrProtocol.name);

     Serial.print("Date");
     Serial.println(pcrProtocol.date);

     Serial.print("repeatStart");
     Serial.println(pcrProtocol.repeatStart);

     Serial.print("repeatEnd");
     Serial.println(pcrProtocol.repeatEnd);

     Serial.println(pcrProtocol.steps[1].name);
     Serial.println(pcrProtocol.steps[1].temperature);
     Serial.println(pcrProtocol.steps[1].duration);
   */
  }
}
void loadProtocol()
{

  String myConfig = getConfig();
  // Serial.println(myConfig);
  parseConfig(myConfig);
}
