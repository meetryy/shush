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

const unsigned char sync[] = {
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

unsigned char reverseBits(unsigned char b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

unsigned char reverseAddress(unsigned char &reverseAddressIndex) {
  unsigned char result = reverseBits(reverseAddressIndex++);
  while (result > 161) {
    result = reverseBits(reverseAddressIndex++);
  }
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
    unsigned char reverseAddressIndex = 0;

    wsprData.N = newN;
    wsprData.M1 = newM1;
    wsprData.M = newM;

    for (i = 0; i < 162; i++) 
        wsprData.data[i] = sync[i];
    
    for (i = 27; i >= 0; i--) {
        reg <<= 1;

        if (wsprData.N & ((uint32_t)1 << i)) reg |= 1;
        wsprData.data[reverseAddress(reverseAddressIndex)] += 2 * calculateParity(reg & 0xf2d05351L);
        wsprData.data[reverseAddress(reverseAddressIndex)] += 2 * calculateParity(reg & 0xe4613c47L);
    }

    for (i = 21; i >= 0; i--) {
        reg <<= 1;
        if (wsprData.M & ((uint32_t)1 << i)) 
            reg |= 1;
        wsprData.data[reverseAddress(reverseAddressIndex)] += 2 * calculateParity(reg & 0xf2d05351L);
        wsprData.data[reverseAddress(reverseAddressIndex)] += 2 * calculateParity(reg & 0xe4613c47L);
    }

    for (i = 30; i >= 0; i--) {
        reg <<= 1;
        wsprData.data[reverseAddress(reverseAddressIndex)] += 2 * calculateParity(reg & 0xf2d05351L);
        wsprData.data[reverseAddress(reverseAddressIndex)] += 2 * calculateParity(reg & 0xe4613c47L);
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

int main(int, char**) {
    // call to int 
    // The 37 allowed characters are allocated values from 0 to 36 such that ‘0’ – ‘9’ give 0 – 9, ‘A’ to ‘Z’ give 10 to 35 and [space] is given the value 36.
    // TODO: check callsign format
    int res = wsprMsgSet(SET_CALL, "r1bsa\0");
    if (res < NO_ERR) errOut(res);

    res = wsprMsgSet(SET_LOC, "kp50\0");
    if (res < NO_ERR) errOut(res);

    res = wsprMsgSet(SET_PWR, "6\0");
    if (res < NO_ERR) errOut(res);

    res = wsprCalcData();
    if (res < NO_ERR) errOut(res);

    return 0;
}
