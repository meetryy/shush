#include "gps.h"
#include "main.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

int hexCharToInt(char a){
    if ((a >= 48) && (a <= 57))
        return (a - 48);
    else if ((a >= 65) && (a <= 70))
        return (a - 65 + 10);
    return 0;
}

struct {
    struct {
        uint8_t     H;
        uint8_t     M;
        uint8_t     S;
        uint16_t    MS;
        char        str[12];
    } time;



    struct {
        uint8_t     D;
        uint8_t     M;
        uint8_t     Y;
        char        str[7];
    } date;


    char    lonStr[16];
    char    latStr[16];
    float   lon;
    float   lat;
    char    qth[5];

    int     satInUse;
    int     satInView;
    int     fix;
    char    status[2];

} gpsData;

char* strtokNew(char* str, char delim);

char msgData[32][16] = {0};
void parseFirstNTokens(char* str, int number){
    memset(msgData, '\0', 32*16);
    char* pch = strtokNew (str, ',');
    int msgN = 0;
    while ((pch != NULL) && ((msgN < number) || (msgN == 0))) {
        strcpy(msgData[msgN], pch);
        pch = strtokNew (NULL, ',');
        msgN++;
    }
}

float ratof(char *arr)
{
  float val = 0;
  int afterdot=0;
  float scale=1;
  int neg = 0;

  if (*arr == '-') {
    arr++;
    neg = 1;
  }
  while (*arr) {
    if (afterdot) {
      scale = scale/10;
      val = val + (*arr-'0')*scale;
    } else {
      if (*arr == '.')
    afterdot++;
      else
    val = val * 10.0 + (*arr - '0');
    }
    arr++;
  }
  if(neg) return -val;
  else    return  val;
}

float GpsToDecimalDegrees(const char* nmeaPos, char quadrant){
  float v= 0;
  if(strlen(nmeaPos)>5)
  {
    char integerPart[3+1];
    int digitCount= (nmeaPos[4]=='.' ? 2 : 3);
    memcpy(integerPart, nmeaPos, digitCount);
    integerPart[digitCount]= 0;
    nmeaPos+= digitCount;
    v = atoi(integerPart) + ratof(nmeaPos)/60.0f;
    if(quadrant=='W' || quadrant=='S')
      v= -v;
  }
  return v;
}


enum nmeaErrors{ERR_NMEA_OK = 0, ERR_NMEA_WRONG_START_CHAR = -1, ERR_NMEA_WRONG_END_CHAR = -2, ERR_NMEA_NO_CHECKSUM_DELIM = -3, ERR_NMEA_CHECKSUM_ERR = -4, NMEA_USELESS_TALKER = -5};

enum RMCitems{RMC_TIME = 1, RMC_STATUS, RMC_LAT, RMC_N, RMC_LON, RMC_E, RMC_SOG, RMC_COG, RMC_DATE, RMC_NUMBER};
enum GGAitems{GGA_SATS = 7, GGA_NUMBER = 8};

int nmeaProcessString(char* nmeaArray, int msgLength){
    // check if begin is $
    if (nmeaArray[0] != '$')
        return ERR_NMEA_WRONG_START_CHAR;
    int msgLen = msgLength;//sizeof(nmeaArray);
    char nmeaString[msgLen];
    strncpy(nmeaString, nmeaArray, msgLen); // "...\r\n\0"
    // check if end is \r\n

    if (!((nmeaArray[msgLen-3] == '\r') && (nmeaArray[msgLen-2] == '\n')))
        return ERR_NMEA_WRONG_END_CHAR;

    if (nmeaArray[msgLen-6] != '*')
        return ERR_NMEA_NO_CHECKSUM_DELIM;
    // calculate checkdum and compare
    // extract sentence type
    char thisChar = '\0';
    uint8_t checksum = 0;

    for (int i = 1; i < msgLen - 6; i++){
        thisChar = nmeaArray[i];
        checksum = checksum ^ thisChar;
    }

    uint8_t rxChecksum = hexCharToInt(nmeaArray[msgLen - 5]) * 16 + hexCharToInt(nmeaArray[msgLen - 4]);
    if (rxChecksum != checksum)
        return ERR_NMEA_CHECKSUM_ERR;

    char talkerID[4];
    strncpy(talkerID, nmeaArray + 3, 3);
    talkerID[3] = '\0';

    if (!strcmp("RMC", talkerID)) {//|| !strcmp("$GNRMC\0", talkerID) || !strcmp("$GNGGA\0", talkerID)){
        parseFirstNTokens(nmeaString, RMC_NUMBER);

        strcpy(gpsData.time.str, msgData[RMC_TIME]);
        gpsData.time.H = (gpsData.time.str[0] - 48) * 10 + (gpsData.time.str[1] - 48);
        gpsData.time.M = (gpsData.time.str[2] - 48) * 10 + (gpsData.time.str[3] - 48);
        gpsData.time.S = (gpsData.time.str[4] - 48) * 10 + (gpsData.time.str[5] - 48);

        char timeMsStr[6];
        strncpy(timeMsStr, gpsData.time.str+7, strlen(gpsData.time.str)-7);
        gpsData.time.MS = atoi(timeMsStr);

        strcpy(gpsData.date.str, msgData[RMC_DATE]);
        gpsData.date.D = (gpsData.date.str[0] - 48) * 10 + (gpsData.date.str[1] - 48);
        gpsData.date.M = (gpsData.date.str[2] - 48) * 10 + (gpsData.date.str[3] - 48);
        gpsData.date.Y = (gpsData.date.str[4] - 48) * 10 + (gpsData.date.str[5] - 48);

        strcpy(gpsData.lonStr, msgData[RMC_LON]);
        strcpy(gpsData.latStr, msgData[RMC_LAT]);
        gpsData.lat = GpsToDecimalDegrees(gpsData.latStr, msgData[RMC_N][0]);
        gpsData.lon = GpsToDecimalDegrees(gpsData.lonStr, msgData[RMC_E][0]);

        float lat = gpsData.lat + 90.0f;                                                  // Locator lat/lon shift.
        float lon = gpsData.lon + 180.0f;

        gpsData.qth[0] = ((int)lon / 20) + 65;              // 1st digit: 20deg longitude slot.
        gpsData.qth[1] = ((int)lat / 10) + 65;             // 2nd digit: 10deg latitude slot.
        gpsData.qth[2] = ((int)lon % 20) / 2 + 48;          // 3rd digit: 2deg longitude slot.
        gpsData.qth[3] = ((int)lat % 10) / 1 + 48;          // 4th digit: 1deg latitude slot.
        //gpsData.qth[4] = ((int)lon % 2) * (int)(60.0f / 5.0f) + 97; // 5th digit: 5min longitude slot.
        //gpsData.qth[5] = ((int)lat % 1) * (int)(60.0f / 2.5f) + 97;  // 6th digit: 2.5min latitude slot.
        gpsData.qth[4] = '\0';

        strcpy(gpsData.status, msgData[RMC_STATUS]);
        gpsData.fix = (gpsData.status[0] == 'A');
    }

    else if (!strcmp("GGA", talkerID)) {
        parseFirstNTokens(nmeaString, GGA_NUMBER);
        gpsData.satInUse = atoi(msgData[GGA_SATS]);
    }

    else return NMEA_USELESS_TALKER;

    return ERR_NMEA_OK;
}


#define STRTOK_MAX  (64)
char* strtokNew(char* str, char delim){
    static int lastPos;
    static int lastLen;
    static char* lastStr;
    static char bufStr[STRTOK_MAX];

    if (str != NULL) {
        lastPos = 0;
        lastLen = strlen(str);
        lastStr = str;
    }

    int j = 0;
    for (int i = lastPos; i < lastLen; i++){

        if (lastStr[i] != delim)
            bufStr[j] = lastStr[i];
        else {
            bufStr[j] = '\0';
            lastPos = i + 1;
            return bufStr;
        }
        if (j<STRTOK_MAX) j++;
    }
    return NULL;
}


