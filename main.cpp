#include "mbed.h"
#include "LCDi2c.h"

#define CODE_LENGTH 4

LCDi2c lcd(LCD20x4);

char str[32];

char correctCode[CODE_LENGTH] = {'2', '3', '5', '6'};
char enterCode[CODE_LENGTH];
int enterDigits = 0;

AnalogIn potentiometer(A0);
DigitalOut alarmLed(LED1);
DigitalOut incorrectCodeLed(LED3);
PwmOut buzzer(D9);
UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

float potentiometerReading = 0.00f;

DigitalOut keypadRow[4] = {PB_3, PB_5, PC_7, PA_15};
DigitalIn keypadCol[4] = {PB_12, PB_13, PB_15, PC_6};

char matrixKeypadIndexToCharArray[] = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'
};

void inputsInit() {
    for(int i = 0; i < 4; i++) {
        keypadCol[i].mode(PullUp);
    }
}

void outputsInit() {
    alarmLed = 0;
    incorrectCodeLed = 0;
    buzzer = 0;
}

char matrixKeypadScan() {
    for(int row = 0; row < 4; row++) {
        for(int i = 0; i < 4; i++) {
            keypadRow[i] = 1;
        }
        keypadRow[row] = 0;

        for(int col = 0; col < 4; col++) {
            if(keypadCol[col] == 0) {
                return matrixKeypadIndexToCharArray[row * 4 + col];
            }
        }
    }
    return '\0';
}

char matrixKeypadUpdate() {
    static char lastKey = '\0';
    static Timer debounceTimer;
    static bool keyRelease = false;
    char currentKey = matrixKeypadScan();

    if(currentKey != '\0' && lastKey == '\0' && !keyRelease) {
        lastKey = currentKey;
        debounceTimer.reset();
        debounceTimer.start();
        keyRelease = true;
        return currentKey;
    }

    if(keyRelease && currentKey == '\0') {
        debounceTimer.stop();
        keyRelease = false;
        lastKey = '\0';
    }

    if(currentKey == '\0') {
        lastKey = '\0';
    }

    return '\0';
}

int main() {
    inputsInit();
    outputsInit();

    uartUsb.write("\nSystem is on.\r\n", 16);

    bool alarmActivated = true;

    while (true) {

        char key = matrixKeypadUpdate();

        str[0] = '\0';
        lcd.locate(0, 0);
        sprintf(str, "Alarm is on, enter");
        lcd.printf("%s", str);
        lcd.locate(0, 1);
        sprintf(str, "code to disable:");
        lcd.printf("%s", str);
        lcd.locate(0, 0);
        str[0] = '\0';

        if (key != '\0') {
            if(key >= '0' && key <= '9') {
                if(enterDigits < CODE_LENGTH) {
                enterCode[enterDigits++] = key;
                //uartUsb.write(&key, 1);
                }

                lcd.locate(0, 2);

                for (int i = 0; i < enterDigits; i++) {
                    if (i < enterDigits) {
                        lcd.printf("*");
                    }
                }

            } else if(key == '#') {
                if(enterDigits == CODE_LENGTH) {
                    //uartUsb.write("\r\nCode entered: ", 16);
                    //uartUsb.write(enterCode, CODE_LENGTH);
                    //uartUsb.write("\r\n", 2);
                    bool correct = true;
                    for(int i = 0; i < CODE_LENGTH; i++) {
                        if(enterCode[i] != correctCode[i]) {
                            correct = false;
                            break;
                        }
                    }

                    if (correct && alarmActivated) {
                        alarmActivated = false;
                        alarmLed = 0;
                        buzzer = 0;
                        lcd.cls();
                        str[0] = '\0';
                        lcd.locate(0, 0);
                        sprintf(str, "Correct code");
                        lcd.printf("%s", str);
                        lcd.locate(0, 1);
                        sprintf(str, "Alarm disabled");
                        lcd.printf("%s", str);
                        str[0] = '\0';
                        break;
                    } else {
                        str[0] = '\0';
                        lcd.locate(0, 3);
                        sprintf(str, "Incorrect code----");
                        lcd.printf("%s", str);
                        lcd.locate(0, 2);
                        lcd.printf(" ");
                        lcd.locate(1, 2);
                        lcd.printf(" ");
                        lcd.locate(2, 2);
                        lcd.printf(" ");
                        lcd.locate(3, 2);
                        lcd.printf(" ");
                        str[0] = '\0';
                        incorrectCodeLed = 1;
                    }
                } else {
                    str[0] = '\0';
                    lcd.locate(0, 3);
                    sprintf(str, "Enter 4 digit code");
                    lcd.printf("%s", str);
                    lcd.locate(0, 2);
                    lcd.printf(" ");
                    lcd.locate(1, 2);
                    lcd.printf(" ");
                    lcd.locate(2, 2);
                    lcd.printf(" ");
                    lcd.locate(3, 2);
                    lcd.printf(" ");
                    str[0] = '\0';
                }
                enterDigits = 0;
            }
        }

        if (alarmActivated) {
        buzzer.period(1.0/100);
        buzzer = 10; 
        ThisThread::sleep_for(200ms);
        buzzer.period(1.0/105);
        buzzer = 10;
        ThisThread::sleep_for(200ms);
        buzzer = 0;
        }
    
        ThisThread::sleep_for(1ms);
    }
}
