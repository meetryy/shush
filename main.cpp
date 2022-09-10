#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


enum {SET_CALL, SET_LOC, SET_PWR};
enum {  NO_ERR = 0, 
        NO_ERROR = 1, 
        ERR_GENERAL = -1, 
        ERR_POWER_WRONG = -2, 
        ERR_NO_NULL_TERM = -3,  
        ERR_WRONG_LENGTH = -4,  
        ERR_WRONG_FORMAT = -5,  
        ERR_WRONG_LOC = -6,  
        ERR_CALC_CALL = -7,  
        ERR_CALC_LOC = -8,  
        ERR_DATA_INACCESSIBLE = -9,
        ERR_FORBIDDEN_CHAR = -10, 
        };

const uint8_t sync[] = {
  1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0,
  1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
  0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1,
  0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1,
  1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1,
  0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1,
  1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0
};

const uint8_t pwrLevels[] = {
    0,  3,  7,  10, 13, 17, 20, 23, 27, 30, 33, 37, 40, 43, 47, 50, 53, 57, 60
};

struct {
    char callsign[7] = "UB0BAA";
    char loc[5] = "KP50";
    int pwr = 3;

    char    newCallsign[7];
    char    newLoc[5];
    int     newPwr;

} wsprMsg;

struct {
    int     N;
    int     M1;
    int     M;
    char    data[162];
    bool    ready;
    bool    updated;
    bool    accessible = 1;
} wsprData;

int callCharToCharCode(char c){
    int toReturn = ERR_FORBIDDEN_CHAR;
    if      ((c >= 48) && (c <= 57)) toReturn = c - 48;     // 0...9 -> 48...57 -> 0...9
    else if ((c >= 65) && (c <= 90)) toReturn = c - 55;     // A...Z -> 65...90 -> 10...35
    else if (c == ' ') toReturn = 36;                       // space == 36; 
    return toReturn;
}

int callToInt(char* call){
    int callN = ERR_GENERAL;
    char callCharCode[7];
    size_t len = strlen(call);
    for (int i = 0; i < len; i++){
        int thisCharCode = callCharToCharCode(call[i]);
        if (thisCharCode >= NO_ERR)
            callCharCode[i] = thisCharCode;
        else return callCharCode[i]; // error: prohibited char
    }
    callN = (((((callCharCode[0] * 36 + callCharCode[1]) * 10 + callCharCode[2]) * 27 + callCharCode[3] - 10) * 27 + callCharCode[4] - 10) * 27 + callCharCode[5] - 10);
    return callN;
}

int locCharToInt(char c){
    int toReturn = ERR_FORBIDDEN_CHAR;
    if ((c >= 65) && (c <= 90)) toReturn = c - 65;     // A...Z -> 65...90 -> 0...17
    return toReturn;
}

int locNumToInt(char c){
    int toReturn = ERR_FORBIDDEN_CHAR;
    if      ((c >= 48) && (c <= 57)) toReturn = c - 48;     // 0...9 -> 48...57 -> 0...9
    return toReturn;
}

int locToInt(char* loc){
    char locCharCode[5];

    locCharCode[0] = locCharToInt(loc[0]);
    locCharCode[1] = locCharToInt(loc[1]);
    locCharCode[2] = locNumToInt(loc[2]);
    locCharCode[3] = locNumToInt(loc[3]);

    if (locCharCode[0] < NO_ERR)
        return locCharCode[0];
    if (locCharCode[1] < NO_ERR)
        return locCharCode[1];
    if (locCharCode[2] < NO_ERR)
        return locCharCode[2];
    if (locCharCode[3] < NO_ERR)
        return locCharCode[3];

    int locM = (179 - 10 * locCharCode[0] - locCharCode[2]) * 180 + 10 * locCharCode[1] + locCharCode[3];
    return locM;
}

uint8_t reverseBits(uint8_t b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

uint8_t reverseAddress(uint8_t* reverseAddressIndex) {
//*reverseAddressIndex = *reverseAddressIndex +1;
  uint8_t result = reverseBits((*reverseAddressIndex)++);
  while (result > 161) 
    result = reverseBits((*reverseAddressIndex)++);
  return result;
}

int calculateParity(unsigned long int x){
  int even = 0;
  while (x) {
    even = 1 - even;
    x = x & (x - 1);
  }
  return even;
}

int wsprCalcData(void) {
    if (wsprData.accessible == false)
        return ERR_DATA_INACCESSIBLE;
    int newN = callToInt(wsprMsg.newCallsign);       // 28 bit
    int newM1 = locToInt(wsprMsg.newLoc);

    if (newN < NO_ERR)
        return ERR_CALC_CALL;
    
    if (newM1 < NO_ERR)
        return ERR_CALC_LOC;

    int newM = newM1 * 128 + (wsprMsg.newPwr) + 64;  // 22 bit

    int i;
    uint32_t reg = 0;
    uint8_t reverseAddressIndex;

    wsprData.N = newN;
    wsprData.M1 = newM1;
    wsprData.M = newM;

    for (i = 0; i < 162; i++) 
        wsprData.data[i] = sync[i];
    
    for (i = 27; i >= 0; i--) {
        reg <<= 1;

        if (wsprData.N & ((uint32_t)1 << i)) reg |= 1;
        wsprData.data[reverseAddress(&reverseAddressIndex)] += 2 * calculateParity(reg & 0xf2d05351L);
        wsprData.data[reverseAddress(&reverseAddressIndex)] += 2 * calculateParity(reg & 0xe4613c47L);
    }

    for (i = 21; i >= 0; i--) {
        reg <<= 1;
        if (wsprData.M & ((uint32_t)1 << i)) 
            reg |= 1;
        wsprData.data[reverseAddress(&reverseAddressIndex)] += 2 * calculateParity(reg & 0xf2d05351L);
        wsprData.data[reverseAddress(&reverseAddressIndex)] += 2 * calculateParity(reg & 0xe4613c47L);
    }

    for (i = 30; i >= 0; i--) {
        reg <<= 1;
        wsprData.data[reverseAddress(&reverseAddressIndex)] += 2 * calculateParity(reg & 0xf2d05351L);
        wsprData.data[reverseAddress(&reverseAddressIndex)] += 2 * calculateParity(reg & 0xe4613c47L);
    }
    
    strcpy(wsprMsg.callsign, wsprMsg.newCallsign);
    strcpy(wsprMsg.newLoc, wsprMsg.loc);
    wsprMsg.pwr = wsprMsg.newPwr;
    wsprData.updated = false;
    wsprData.ready = true;
}

int wsprMsgSet(int setting, char* value){
    int toReturn = ERR_GENERAL;
    size_t len = strlen(value);//sizeof(value)/sizeof(value[0]);
    if ((value[len] != '\0') | (len < 1))
        return ERR_NO_NULL_TERM;
    switch(setting){
        case SET_CALL: {
            char newCall[len+2];// AB1CDE\0
            // check length
            if (len == 5){ // make "A1BCD\0" -> " A1BCD\0"   
                newCall[0] = ' ';
                for (int j=1; j<6; j++)
                    newCall[j] = value[j-1];
            }

            else if (len == 6) // copy AB1CDE as is
                strcpy(newCall, value); 
            else {
                toReturn = ERR_WRONG_LENGTH;
                break;
            }

            // convert to uppercase
            for (int j=0; j<7; j++)
                newCall[j] = toupper(newCall[j]);

            // check if 2nd char is not a space
            // check if char 3...6 is not a number
            // check if 3rd char is number
            if (!isdigit(newCall[2]) | newCall[1] == ' ' | !isalpha(newCall[3]) | !isalpha(newCall[4]) | !isalpha(newCall[5]))   {
                toReturn = ERR_WRONG_FORMAT;
                break;
            }

            strcpy(wsprMsg.newCallsign, newCall);
            toReturn = NO_ERR;
            break;
        }

        case SET_LOC:{ 
            char newLoc[len+1]; // AA01\0
            if ((len < 4) | (len > 4)){
                toReturn = ERR_WRONG_LENGTH;
                break;
            }
                
            strcpy(newLoc, value); 
            for (int j=0; j<5; j++)
                newLoc[j] = toupper(newLoc[j]);

            // check if loc is AA00...RR99
            if (!isalpha(newLoc[0]) | !isalpha(newLoc[1]) | !isdigit(newLoc[2]) | !isdigit(newLoc[3])){
                toReturn = ERR_WRONG_FORMAT;
                break;
            }
            
            if ((newLoc[2] > 82) | (newLoc[3] > 82)){
                toReturn = ERR_WRONG_LOC;
                break;
            }
            
            strcpy(wsprMsg.newLoc, newLoc);
            toReturn = NO_ERR;
            break;
        }

        case SET_PWR:{
            int newPwr = atoi(value);
            if ((newPwr < 0) | (newPwr > 60))
                break;
            bool found = false;
            for (int j=0; j<sizeof(pwrLevels)/sizeof(pwrLevels[0]);j++){
                if (pwrLevels[j] == newPwr){
                    found = true;
                    break;
                }
            }
            wsprMsg.newPwr = newPwr;
            toReturn = ERR_POWER_WRONG;
            break;
        }
    }

    if (toReturn > NO_ERR) 
        wsprData.updated = true;
    return toReturn;
}

void consoleOut(char* a){
    printf("ERROR: %s", a);
};

void errOut(int errCode){
    switch (errCode){
        case ERR_GENERAL: {
            consoleOut("General error!\n");
            break;
        };
        case ERR_CALC_CALL: {
            consoleOut("Callsign calculation fail\n");
            break;
        };
        case ERR_CALC_LOC: {
            consoleOut("Locator calculation fail\n");
            break;
        };
        case ERR_DATA_INACCESSIBLE: {
            consoleOut("Data is inaccessible now\n");
            break;
        };
        case ERR_FORBIDDEN_CHAR: {
            consoleOut("Forbidden character during calculation\n");
            break;
        };
        case ERR_NO_NULL_TERM: {
            consoleOut("String is not null-terminated!\n");
            break;
        };
        case ERR_POWER_WRONG: {
            consoleOut("Power setting is wrong, saved anyway\n");
            break;
        };
        case ERR_WRONG_FORMAT: {
            consoleOut("Wrong format\n");
            break;
        };
        case ERR_WRONG_LENGTH: {
            consoleOut("Wrong length\n");
            break;
        };
        case ERR_WRONG_LOC: {
            consoleOut("Wrong locator\n");
            break;
        };
    }
}

int hexCharToInt(char a){
    if ((a >= 48) && (a <= 57))
        return (a - 48);
    else if ((a >= 65) && (a <= 70))
        return (a - 65 + 10);
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

}   gpsData;


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


float ratof(const char *arr)
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
    v= atoi(integerPart) + ratof(nmeaPos)/60.0f;
    if(quadrant=='W' || quadrant=='S')
      v= -v;
  }
  return v;
}
#include <math.h>

enum nmeaErrors{ERR_NMEA_WRONG_START_CHAR, ERR_NMEA_WRONG_END_CHAR, ERR_NMEA_NO_CHECKSUM_DELIM, ERR_NMEA_CHECKSUM_ERR};

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

    if (!strcmp("GGA", talkerID)) {
        parseFirstNTokens(nmeaString, GGA_NUMBER);
        gpsData.satInUse = atoi(msgData[GGA_SATS]);
    }

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



int main(int, char**) {
    // call to int 
    // The 37 allowed characters are allocated values from 0 to 36 such that ‘0’ – ‘9’ give 0 – 9, ‘A’ to ‘Z’ give 10 to 35 and [space] is given the value 36.
    // TODO: check callsign format

    /*
    int res = wsprMsgSet(SET_CALL, "r1ccu\0");
    if (res < NO_ERR) errOut(res);

    res = wsprMsgSet(SET_LOC, "kp50\0");
    if (res < NO_ERR) errOut(res);

    res = wsprMsgSet(SET_PWR, "6\0");
    if (res < NO_ERR) errOut(res);

    res = wsprCalcData();
    if (res < NO_ERR) errOut(res);
*/



    char nmeaArray1[] = "$GNGGA,115113.00,6003.16079,N,03025.78127,E,1,03,3.48,126.8,M,15.8,M,,*4A\r\n";
    //char nmeaArray1[] = "$GNGGA,,,,,,0,00,99.99,,,,,,*56\r\n";
    char nmeaArray2[] = "$GNRMC,222024.00,A,6003.13892,N,03025.80624,E,0.166,,090922,,,A*63\r\n";

    nmeaProcessString(nmeaArray1, sizeof(nmeaArray1));
    nmeaProcessString(nmeaArray2, sizeof(nmeaArray2));


    return 0;
}
