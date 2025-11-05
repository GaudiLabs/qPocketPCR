#include "Arduino.h"
#include "TLC59108.h"

TLC59108::TLC59108(TwoWire *i2c = &Wire, byte addr = 0x40) {
  _i2c = i2c;
  _addr = addr;
  
}

TLC59108::TLC59108(byte addr = 0x40) {
  _i2c = &Wire;
  _addr = addr;
}

uint8_t TLC59108::init(const uint8_t hwResetPin) {
  if(hwResetPin)
  {
    pinMode(hwResetPin, OUTPUT);
    digitalWrite(hwResetPin, LOW);
    delay(1);
    digitalWrite(hwResetPin, HIGH);
    delay(1);
  }

   return setRegister(REGISTER::MODE1::ADDR, REGISTER::MODE1::ALLCALL);
}

uint8_t TLC59108::setLedOutputMode(const uint8_t outputMode) {
   if(outputMode & 0xfc)
     return ERROR::ERR;

   byte regValue = (outputMode << 6) | (outputMode << 4) | (outputMode << 2) | outputMode;

   uint8_t retVal = setRegister(REGISTER::LEDOUT0::ADDR, regValue);
   retVal &= setRegister(REGISTER::LEDOUT1::ADDR, regValue);
   return retVal;
}

uint8_t TLC59108::readRegister(const uint8_t reg) const {
   _i2c->beginTransmission(_addr);
   _i2c->write(reg);
   if(!_i2c->endTransmission())
     return -1;

   _i2c->requestFrom(_addr, (uint8_t) 1);
   if(_i2c->available())
     return _i2c->read();
   else
     return -1;
}

uint8_t TLC59108::readRegisters(uint8_t *dest, const uint8_t startReg, const uint8_t num) const {
	Serial.println("in readRegisters");
	_i2c->beginTransmission(_addr);
	_i2c->write(startReg | AUTO_INCREMENT::ALL);
	if(_i2c->endTransmission())
		return 0;

	uint8_t bytesRead = 0;
	_i2c->requestFrom(_addr, num);
	while(_i2c->available() && (bytesRead < num)) {
		(*dest) = (uint8_t) _i2c->read();
		dest++;
		bytesRead++;
	}

	return bytesRead;
}

bool TLC59108::readAllBrightness(uint8_t dutyCycles[]) const {
	return (readRegisters(dutyCycles, REGISTER::PWM0::ADDR, NUM_CHANNELS) == NUM_CHANNELS);
}

uint8_t TLC59108::setRegister(const uint8_t reg, const uint8_t value) {
  _i2c->beginTransmission(_addr);
  _i2c->write(reg);
  _i2c->write(value);
  return _i2c->endTransmission();
}

uint8_t TLC59108::setRegisters(const uint8_t startReg, const uint8_t values[], const uint8_t numValues) {
	_i2c->beginTransmission(_addr);
	_i2c->write(startReg | AUTO_INCREMENT::ALL);
	for(uint8_t i = 0; i < numValues; i++)
		_i2c->write(values[i]);
	return _i2c->endTransmission();
}

uint8_t TLC59108::setBrightness(const uint8_t pwmChannel, const uint8_t dutyCycle) {
   if(pwmChannel > 7)
     return ERROR::ERR;

   return setRegister(pwmChannel + 2, dutyCycle);
}

uint8_t TLC59108::setAllBrightness(const uint8_t dutyCycle) {
   _i2c->beginTransmission(_addr);
   _i2c->write(REGISTER::PWM0::ADDR | AUTO_INCREMENT::IND);
   for(uint8_t i=0; i<NUM_CHANNELS; i++)
     _i2c->write(dutyCycle);
   return _i2c->endTransmission();
}

uint8_t TLC59108::setAllBrightnessArray(const uint8_t dutyCycles[])  {
	return setRegisters(REGISTER::PWM0::ADDR, dutyCycles, NUM_CHANNELS);
}
