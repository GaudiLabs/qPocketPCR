
// 32 x 32
const uint8_t buttonSettings[] PROGMEM = {

  0x00, 0x07, 0xe0, 0x00, 
  0x00, 0x0e, 0xb0, 0x00, 
  0x00, 0x0c, 0x30, 0x00, 
  0x03, 0x8c, 0x31, 0xc0, 
  0x0f, 0xcc, 0x33, 0xe0, 
  0x0c, 0x7c, 0x1e, 0x38, 
  0x18, 0x30, 0x0c, 0x18, 
  0x18, 0x00, 0x00, 0x18, 
  0x18, 0x00, 0x00, 0x38, 
  0x0c, 0x07, 0xe0, 0x30, 
  0x06, 0x1f, 0xf0, 0x60, 
  0x06, 0x18, 0x1c, 0x60, 
  0x7c, 0x30, 0x0c, 0x3e, 
  0xf8, 0x60, 0x06, 0x2f, 
  0x80, 0x60, 0x06, 0x01, 
  0xc0, 0x60, 0x06, 0x01, 
  0x80, 0x60, 0x06, 0x03, 
  0xc0, 0x60, 0x06, 0x01, 
  0xfc, 0x20, 0x0c, 0x1f, 
  0x7c, 0x30, 0x0c, 0x3e, 
  0x06, 0x1c, 0x38, 0x60, 
  0x06, 0x0f, 0xf0, 0x60, 
  0x0c, 0x03, 0xc0, 0x30, 
  0x1c, 0x00, 0x00, 0x18, 
  0x18, 0x00, 0x00, 0x18, 
  0x18, 0x30, 0x0c, 0x18, 
  0x0c, 0xfc, 0x3e, 0x30, 
  0x07, 0xcc, 0x33, 0xf0, 
  0x03, 0x8c, 0x31, 0xc0, 
  0x00, 0x0c, 0x30, 0x00, 
  0x00, 0x0c, 0x30, 0x00, 
  0x00, 0x07, 0xe0, 0x00, 
};

// 40 x 40
const uint8_t buttonArrow[] PROGMEM = {
  0x0f, 0xff, 0xff, 0xff, 0xf0, 
  0x3f, 0xff, 0xff, 0xff, 0xfc, 
  0x7f, 0xff, 0xff, 0xff, 0xfe, 
  0x7f, 0xff, 0xff, 0xff, 0xfe, 
  0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xf9, 0xff, 0xff, 0xff, 
  0xff, 0xe0, 0x7f, 0xff, 0xff, 
  0xff, 0xf0, 0x3f, 0xff, 0xff, 
  0xff, 0xe0, 0x0f, 0xff, 0xff, 
  0xff, 0xe0, 0x07, 0xff, 0xff, 
  0xff, 0xe0, 0x01, 0xff, 0xff, 
  0xff, 0xe0, 0x00, 0xff, 0xff, 
  0xff, 0xe0, 0x00, 0x3f, 0xff, 
  0xff, 0xe0, 0x00, 0x1f, 0xff, 
  0xff, 0xe0, 0x00, 0x07, 0xff, 
  0xff, 0xe0, 0x00, 0x03, 0xff, 
  0xff, 0xe0, 0x00, 0x01, 0xff, 
  0xff, 0xe0, 0x00, 0x00, 0xff, 
  0xff, 0xe0, 0x00, 0x00, 0xff, 
  0xff, 0xe0, 0x00, 0x01, 0xff, 
  0xff, 0xe0, 0x00, 0x01, 0xff, 
  0xff, 0xe0, 0x00, 0x07, 0xff, 
  0xff, 0xe0, 0x00, 0x1f, 0xff, 
  0xff, 0xe0, 0x00, 0x3f, 0xff, 
  0xff, 0xe0, 0x00, 0xff, 0xff, 
  0xff, 0xe0, 0x01, 0xff, 0xff, 
  0xff, 0xe0, 0x07, 0xff, 0xff, 
  0xff, 0xe0, 0x0f, 0xff, 0xff, 
  0xff, 0xe0, 0x3f, 0xff, 0xff, 
  0xff, 0xf0, 0x7f, 0xff, 0xff, 
  0xff, 0xf1, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 
  0x7f, 0xff, 0xff, 0xff, 0xfe, 
  0x7f, 0xff, 0xff, 0xff, 0xfe, 
  0x3f, 0xff, 0xff, 0xff, 0xfc, 
  0x0f, 0xff, 0xff, 0xff, 0xf0, 
};

//30x17
const uint8_t buttonLAMP[] PROGMEM = {
  0xff, 0x80, 0x07, 0xfc, 
  0xff, 0x80, 0x07, 0xfc, 
  0xff, 0x9f, 0xe7, 0xfc, 
  0xff, 0x3f, 0xf3, 0xfc, 
  0xff, 0x3f, 0xf3, 0xfc, 
  0xff, 0x3f, 0xf3, 0xfc, 
  0xff, 0x3f, 0xf3, 0xfc, 
  0xfe, 0x3f, 0xf1, 0xfc, 
  0xfe, 0x7f, 0xf9, 0xfc, 
  0xfe, 0x7f, 0xf9, 0xfc, 
  0xfe, 0x7f, 0xf9, 0xfc, 
  0xfe, 0x7f, 0xf9, 0xfc, 
  0xfc, 0xff, 0xfc, 0xfc, 
  0xfc, 0xff, 0xfc, 0xfc, 
  0xfc, 0xff, 0xfc, 0xfc, 
  0x00, 0xff, 0xfc, 0x00, 
  0x00, 0xff, 0xfc, 0x00, 
};

//30x17
const uint8_t buttonPCR[] PROGMEM = {
  0xfe, 0x01, 0xff, 0xfc, 
  0xfc, 0x01, 0xff, 0xfc, 
  0xfc, 0xf9, 0xff, 0xfc, 
  0xfc, 0xf8, 0xff, 0xfc, 
  0xf8, 0xf9, 0xff, 0xfc, 
  0xf9, 0xfc, 0xff, 0xfc, 
  0xf9, 0xfc, 0xff, 0xfc, 
  0xf1, 0xfc, 0xff, 0x00, 
  0x03, 0xfc, 0xfe, 0x00, 
  0x03, 0xfc, 0x7e, 0x7c, 
  0xff, 0xfc, 0xfe, 0x7c, 
  0xff, 0xfe, 0x7e, 0x7c, 
  0xff, 0xfe, 0x7c, 0x7c, 
  0xff, 0xfe, 0x7c, 0xfc, 
  0xff, 0xfe, 0x7c, 0xfc, 
  0xff, 0xfe, 0x00, 0xfc, 
  0xff, 0xff, 0x00, 0xfc, 
};
