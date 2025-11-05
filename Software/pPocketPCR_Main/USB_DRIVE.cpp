#include "USBMSC.h"
#include "USB.h"
#include "FS.h"
#include "SPIFFS.h"
#include "USB_DRIVE.h"

#if ARDUINO_USB_CDC_ON_BOOT
#define HWSerial Serial0
#define USBSerial Serial
#else
#define HWSerial Serial
USBCDC USBSerial;
#endif


  boolean newConfigAvailable=true;
  boolean shoudSave=false;
  boolean initialize_Disk=false;

unsigned long saveTime;


USBMSC MSC;

 uint8_t msc_disk[DISK_SECTOR_COUNT][DISK_SECTOR_SIZE];
 
 uint8_t msc_init[4][DISK_SECTOR_SIZE] =
{
  //------------- Block0: Boot Sector -------------//
  {
    // Header (62 bytes)
    0xEB, 0x3C, 0x90, //jump_instruction
    'M' , 'S' , 'D' , 'O' , 'S' , '5' , '.' , '0' , //oem_name
    FAT_U16(DISK_SECTOR_SIZE), //bytes_per_sector
    FAT_U8(1),    //sectors_per_cluster
    FAT_U16(1),   //reserved_sectors_count
    FAT_U8(1),    //file_alloc_tables_num
    FAT_U16(16),  //max_root_dir_entries
    FAT_U16(DISK_SECTOR_COUNT), //fat12_sector_num
    0xF8,         //media_descriptor
    FAT_U16(DISC_SECTORS_PER_TABLE),   //sectors_per_alloc_table;//FAT12 and FAT16
    FAT_U16(1),   //sectors_per_track;//A value of 0 may indicate LBA-only access
    FAT_U16(1),   //num_heads
    FAT_U32(0),   //hidden_sectors_count
    FAT_U32(0),   //total_sectors_32
    0x00,         //physical_drive_number;0x00 for (first) removable media, 0x80 for (first) fixed disk
    0x00,         //reserved
    0x29,         //extended_boot_signature;//should be 0x29
    FAT_U32(0x1234), //serial_number: 0x1234 => 1234
    'T' , 'i' , 'n' , 'y' , 'U' , 'S' , 'B' , ' ' , 'M' , 'S' , 'C' , //volume_label padded with spaces (0x20)
    'F' , 'A' , 'T' , '1' , '2' , ' ' , ' ' , ' ' ,  //file_system_type padded with spaces (0x20)

    // Zero up to 2 last bytes of FAT magic code (448 bytes)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

     //boot signature (2 bytes)
    0x55, 0xAA
  },

  //------------- Block1: FAT12 Table -------------//
  {
FAT_TBL2B(0xFF8, 0xFFF), 
FAT_TBL2B(0x3,0x4),
FAT_TBL2B(0x5,0x6),
FAT_TBL2B(0x7,0x8),
FAT_TBL2B(0x9,0xA),
FAT_TBL2B(0xB,0xFFF),
FAT_TBL2B(0xD,0xE),
FAT_TBL2B(0xF,0x10),
FAT_TBL2B(0x11,0x12),
FAT_TBL2B(0x13,0x14),
FAT_TBL2B(0x15,0x16),
FAT_TBL2B(0x17,0x18),
FAT_TBL2B(0x19,0x1A),
FAT_TBL2B(0x1B,0x1C),
FAT_TBL2B(0x1D,0x1E),
FAT_TBL2B(0x1F,0x20),
FAT_TBL2B(0x21,0x22),
FAT_TBL2B(0x23,0x24),
FAT_TBL2B(0x25, 0x26),
FAT_TBL2B(0x27, 0x28),
FAT_TBL2B(0x29, 0x2A),
FAT_TBL2B(0x2B, 0xFFF),


// first 2 entries must be 0xFF8 0xFFF, third entry is cluster end of readme file
  },

  //------------- Block2: Root Directory -------------//
  {
    // first entry is volume label
    'q', 'P' , 'o' , 'c' , 'k' , 'e' , 't' , 'P' , 
    'C' , 'R' , ' ' , 
    0x08, //FILE_ATTR_VOLUME_LABEL
    0x00, 
    FAT_MS2B(0,0), 
    FAT_HMS2B(0,0,0),
    FAT_YMD2B(0,0,0), 
    FAT_YMD2B(0,0,0), 
    FAT_U16(0), 
    FAT_HMS2B(13,42,30),  //last_modified_hms
    FAT_YMD2B(2018,11,5), //last_modified_ymd
    FAT_U16(0), 
    FAT_U32(0),
    
 
    // second entry is readme file
    'P' , 'R' , 'O' , 'T' , 'O' , 'C' , 'O' , 'L' ,//file_name[8]; padded with spaces (0x20)
    'T' , 'X' , 'T' ,     //file_extension[3]; padded with spaces (0x20)
    0x20,                 //file attributes: FILE_ATTR_ARCHIVE
    0x00,                 //ignore
    FAT_MS2B(1,980),      //creation_time_10_ms (max 199x10 = 1s 990ms)
    FAT_HMS2B(13,42,36),  //create_time_hms [5:6:5] => h:m:(s/2)
    FAT_YMD2B(2018,11,5), //create_time_ymd [7:4:5] => (y+1980):m:d
    FAT_YMD2B(2020,11,5), //last_access_ymd
    FAT_U16(0),           //extended_attributes
    FAT_HMS2B(13,44,16),  //last_modified_hms
    FAT_YMD2B(2019,11,5), //last_modified_ymd
    FAT_U16(2),           //start of file in cluster
    FAT_U32(sizeof(PROTOCOL_TEMPLATE)-1), //file size


       // second entry is readme file
    'D' , 'A' , 'T' , 'A' , 'Q' , 'P' , 'C' , 'R' ,//file_name[8]; padded with spaces (0x20)
    'T' , 'X' , 'T' ,     //file_extension[3]; padded with spaces (0x20)
    0x20,                 //file attributes: FILE_ATTR_ARCHIVE
    0x00,                 //ignore
    FAT_MS2B(1,980),      //creation_time_10_ms (max 199x10 = 1s 990ms)
    FAT_HMS2B(13,42,36),  //create_time_hms [5:6:5] => h:m:(s/2)
    FAT_YMD2B(2018,11,5), //create_time_ymd [7:4:5] => (y+1980):m:d
    FAT_YMD2B(2020,11,5), //last_access_ymd
    FAT_U16(0),           //extended_attributes
    FAT_HMS2B(13,44,16),  //last_modified_hms
    FAT_YMD2B(2019,11,5), //last_modified_ymd
    FAT_U16(12),           //start of file in cluster
    FAT_U32(0), //file size
    
      

  },

  //------------- Block3: Readme Content -------------//
  PROTOCOL_TEMPLATE
};


void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.print (file.name());
            time_t t= file.getLastWrite();
            struct tm * tmstruct = localtime(&t);
            Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.print(file.size());
            time_t t= file.getLastWrite();
            struct tm * tmstruct = localtime(&t);
            Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
        }
        file = root.openNextFile();
    }
}


void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}


void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}


uint16_t extract12BitNumber(int index) {
  // Calculate the starting index in the byte array
  int byteIndex = (index * 3) / 2; // Dividing by 2 because each 12-bit number occupies 1.5 bytes
  
  // Determine whether to take the most significant or least significant bits
  bool takeMSB = index % 2 == 0; // If index is even, take most significant bits; otherwise, take least significant bits
  
  // Combine bytes to form 12-bit number
  uint16_t twelveBitNumber;
   if (takeMSB) {
    twelveBitNumber = (msc_disk[1][byteIndex]) | ((msc_disk[1][byteIndex + 1]& 0x0F) << 8);
  } else {
    twelveBitNumber = (msc_disk[1][byteIndex]&0xf0)>>4 | msc_disk[1][byteIndex+1]<<4;
  }
  
  return twelveBitNumber;
}


String getConfig()
{

  int config_index=0;
  char nameString[8];
  uint8_t* namePointer;
  
   for (int i = 0; i <= (DISK_SIZE*2); i++) {

    if (msc_disk[2][32*i] == 0x00) {
            // Empty entry, stop counting
            break;
        } else if ((msc_disk[2][32*i] != 0xE5)&&(msc_disk[2][32*i+11]==0x20)) {
            // Valid entry, get index
          
          namePointer=&msc_disk[2][32*i];
          strncpy(nameString,(const char*)namePointer, 8);
          String extraString(nameString, 8);
          
         // Serial.print(extraString);
         // Serial.print(" type: ");
         // Serial.println(msc_disk[2][32*i+11]);
          
          if (extraString=="PROTOCOL"){config_index=i;}
        }
    }

    String configString="";
    

if (config_index>0){

  int config_length=msc_disk[2][32*config_index+28]+(msc_disk[2][32*config_index+29]<<8);
  int config_cluster=msc_disk[2][32*config_index+26]+(msc_disk[2][32*config_index+27]<<8);

  int clusterLength=0;

    for (int cluster = 0; cluster<(config_length/DISK_SECTOR_SIZE+1) ; cluster++) {

     uint8_t* config_pointer=&msc_disk[config_cluster+1][0];

    clusterLength=config_length-DISK_SECTOR_SIZE*cluster;
    if (clusterLength>DISK_SECTOR_SIZE) clusterLength=DISK_SECTOR_SIZE;
    char newString[clusterLength+1];
    
    strncpy(newString,(const char*)config_pointer, clusterLength);
    String extraString(newString, clusterLength);
    configString=configString+extraString;
    
   // Serial.print("clusterLength: ");
    //Serial.println(clusterLength);
     
    config_cluster=extract12BitNumber(config_cluster);
    }
}
return configString;
}

void init_msc_disk()
{
 for (int block = 0; block<4 ; block++) {

        for (int block = 0; block<4 ; block++) {
           for (int i = 0; i<DISK_SECTOR_SIZE ; i++) 
           msc_disk[block][i]=msc_init[block][i];
        }
    }
 }
 

void addFileToFAT(fs::FS &fs, String path){
    
   uint8_t* c=&msc_disk[13][0];

   File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.printf("Reading file: %s\n", path);

    unsigned long i=0;
      while(file.available()){
       c[i]=file.read();
       i++;
    }

    file.close();

    msc_disk[2][32*2+28]=FAT_U8(i);
    msc_disk[2][32*2+29]=FAT_U8(i>> 8);
    msc_disk[2][32*2+30]=FAT_U8(i>> 16);
    msc_disk[2][32*2+31]=FAT_U8(i>> 32);
}

void addProtoToFAT(String str){
    
    uint8_t* c=&msc_disk[3][0]; 

    int i = 0;
    unsigned long str_length = str.length();

    for (int i = 0; i < str_length && i < 512; i++) {
        c[i] = str[i];
    }

    msc_disk[2][32+28]=FAT_U8((str_length)); 
    msc_disk[2][32+29]=FAT_U8((str_length)>> 8);
    msc_disk[2][32+30]=FAT_U8((str_length)>> 16);
    msc_disk[2][32+31]=FAT_U8((str_length)>> 24); //not 32?
}

void addStringToFAT(String str){
    
    uint8_t* c=&msc_disk[13][0]; //13

   unsigned long offset=msc_disk[2][32*2+28]+ (msc_disk[2][32*2+29]<<8)+(msc_disk[2][32*2+30]<<16)+(msc_disk[2][32*2+31]<<32);

    int i = 0;
    unsigned long str_length = str.length();

    for (int i = 0; i < str_length && i < 512; i++) {
        c[offset + i] = str[i];
    }

    msc_disk[2][32*2+28]=FAT_U8((str_length+offset)); //32*2
    msc_disk[2][32*2+29]=FAT_U8((str_length+offset)>> 8);
    msc_disk[2][32*2+30]=FAT_U8((str_length+offset)>> 16);
    msc_disk[2][32*2+31]=FAT_U8((str_length+offset)>> 24); //not 32?
}



void saveBinToSPIFFS(uint8_t binArray[],size_t binSize,const char* filename) {
    File file = SPIFFS.open(filename, FILE_WRITE);
    if (!file) {
        Serial.print("Failed to open file for writing: ");
        Serial.println(filename);

        return;
    }

    // Write the array to the file

if( file.write((uint8_t *)binArray, binSize)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }

    
    file.close();
}

bool loadBinFromSPIFFS(uint8_t binArray[], size_t binSize, const char* filename) {
    File file = SPIFFS.open(filename, FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return false; // Return false indicating failure
    }

    // Read the array from the file
    size_t bytesRead = file.readBytes((char *)binArray, binSize);
    
    file.close();

    // Check if read operation was successful
    if (bytesRead != binSize) {
        Serial.println("Error: Incomplete read");
        return true; // Return true indicating failure
    }

    return false; // Return flase indicating success
}




void saveMscToSPIFFS(uint8_t array[DISK_SECTOR_COUNT][DISK_SECTOR_SIZE]) {
    File file = SPIFFS.open("/my_array.bin", FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    // Write the array to the file
    file.write((uint8_t *)array, DISK_SECTOR_COUNT * DISK_SECTOR_SIZE);
    
    file.close();
}

void readMscFromSPIFFS(uint8_t array[DISK_SECTOR_COUNT][DISK_SECTOR_SIZE]) {
    File file = SPIFFS.open("/my_array.bin", FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return;
    }

    // Read the array from the file
    file.readBytes((char *)array, DISK_SECTOR_COUNT * DISK_SECTOR_SIZE);
    
    file.close();
}


static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize){
  HWSerial.printf("MSC WRITE: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);
  memcpy(msc_disk[lba] + offset, buffer, bufsize);
  Serial.print("write");
  //  Serial.println(bufsize);
           shoudSave=true;      

  return bufsize;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize){

  HWSerial.printf("MSC READ: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);
  memcpy(buffer, msc_disk[lba] + offset, bufsize);
    Serial.println("read");

  return bufsize;
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject){
  HWSerial.printf("MSC START/STOP: power: %u, start: %u, eject: %u\n", power_condition, start, load_eject);
      Serial.println("start stop");
         shoudSave=true;
         saveTime=millis();
         
            //  saveMscToSPIFFS(msc_disk);
  return true;
}

static void usbEventCallback(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
  if(event_base == ARDUINO_USB_EVENTS){
    arduino_usb_event_data_t * data = (arduino_usb_event_data_t*)event_data;
    switch (event_id){
      case ARDUINO_USB_STARTED_EVENT:
        HWSerial.println("USB PLUGGED");
        break;
      case ARDUINO_USB_STOPPED_EVENT:
        HWSerial.println("USB UNPLUGGED");
        break;
      case ARDUINO_USB_SUSPEND_EVENT:
        HWSerial.printf("USB SUSPENDED: remote_wakeup_en: %u\n", data->suspend.remote_wakeup_en);
        break;
      case ARDUINO_USB_RESUME_EVENT:
        HWSerial.println("USB RESUMED");
        break;
      
      default:
        break;
    }
  }
}

void Start_USB_Drive()
{
  HWSerial.begin(250000);
  HWSerial.setDebugOutput(true);


if (initialize_Disk)
  {init_msc_disk();
  saveMscToSPIFFS(msc_disk);}
  
  else
  {
                readMscFromSPIFFS(msc_disk);
               //  String protoString=getConfig();
               //  init_msc_disk();
                //addProtoToFAT(protoString);
                
                //saveMscToSPIFFS(msc_disk);

  }
  addStringToFAT("Start Run to get Data");


  USB.onEvent(usbEventCallback);
  MSC.vendorID("ESP32");//max 8 chars
  MSC.productID("USB_MSC");//max 16 chars
  MSC.productRevision("1.0");//max 4 chars
  MSC.onStartStop(onStartStop);
  MSC.onRead(onRead);
  MSC.onWrite(onWrite);
  MSC.mediaPresent(true);
  MSC.begin(DISK_SECTOR_COUNT, DISK_SECTOR_SIZE);
  USBSerial.begin();
  USB.begin();

}


void Service_USB()
{
  if (!cameraOn&&shoudSave&&((millis()-saveTime)>1000)){saveMscToSPIFFS(msc_disk);shoudSave=false;      Serial.println("save");
   newConfigAvailable=true;
   saveTime=millis();
}

  }


void InitializeUSBFiles()
{

  readMscFromSPIFFS(msc_disk);
  String protoString=getConfig();
  init_msc_disk();
  addProtoToFAT(protoString);


}
void InitializeUSB()
{
SPIFFS.begin(true);

bool formatted = SPIFFS.format();

MSC.mediaPresent(false);
delay(500);
initialize_Disk=true;
Start_USB_Drive();
initialize_Disk=false;
  }


void SPIFF_Format()
{
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("\n\n----Listing files before format----");
  bool formatted = SPIFFS.format();
  if(formatted){
    Serial.println("\n\nSuccess formatting");
  }else{
    Serial.println("\n\nError formatting");
  }
  Serial.println("\n\n----Listing files after format----");
}
  
