#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>

#define PIN_TRIG 13
#define PIN_ECHO 12
#define JOYSTICK_PIN_X A1
#define JOYSTICK_PIN_Y A0
#define JOYSTICK_SW 6

const uint8_t BUTTON_PIN = REFS1;

const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const uint8_t ROWS = 4;
const uint8_t COLS = 4;

byte rowPins[ROWS] = {11, 10, 9, 8};
byte colPins[COLS] = {7, 6, 5, 4};

int anodPins[] = {A1, A2, A3, A4};
int segmentsPins[] = {5, 6, 7, 8, 9, 10, 11, 12};

char keys[ROWS][COLS] = {
        {'1','2','3','A'},
        {'4','5','6','B'},
        {'7','8','9','C'},
        {'*','0','#','D'}
};

int seg[10][8] = {
        {1, 1, 1, 1, 1, 1, 0, 0}, //Цифра 0
        {0, 1, 1, 0, 0, 0, 0, 0}, //Цифра 1
        {1, 1, 0, 1, 1, 0, 1, 0}, //Цифра 2
        {1, 1, 1, 1, 0, 0, 1, 0}, //Цифра 3
        {0, 1, 1, 0, 0, 1, 1, 0}, //Цифра 4
        {1, 0, 1, 1, 0, 1, 1, 0}, //Цифра 5
        {1, 0, 1, 1, 1, 1, 1, 0}, //Цифра 6
        {1, 1, 1, 0, 0, 0, 0, 0}, //Цифра 7
        {1, 1, 1, 1, 1, 1, 1, 0}, //Цифра 8
        {1, 1, 1, 1, 0, 1, 1, 0}  //Цифра 9
};

int err[4][8] = {
        {0, 0, 0, 0, 0, 0, 0, 0}, //Пусто
        {1, 0, 0, 1, 1, 1, 1, 0}, //E
        {0, 0, 0, 0, 1, 0, 1, 0}, //r
        {0, 0, 0, 0, 1, 0, 1, 0}  //r
};

int minus[8] = {0, 0, 0, 0, 1, 0, 0, 0};
int empty[8] = {0, 0, 0, 0, 0, 0, 0, 0};

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

Servo servo;

void servoRotate(Servo servo, int from, int to, unsigned angleDelay) {
    if (from == to) return;

    servo.write(from);

    for (int i = from; i <= to; ++i) {
        servo.write(i);
        delay(angleDelay); // Wait for 50 millisecond(s)
    }
}

double measureDistanceCm() {
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(5);
    digitalWrite(PIN_TRIG, HIGH);

    // Выставив высокий уровень сигнала, ждем около 10 микросекунд. В этот момент датчик будет посылать сигналы с частотой 40 КГц.
    delayMicroseconds(15);
    digitalWrite(PIN_TRIG, LOW);

    //  Время задержки акустического сигнала на эхолокаторе.
    unsigned long duration = pulseIn(PIN_ECHO, HIGH, 4000);

    // Теперь осталось преобразовать время в расстояние
    double cm = (duration / 2) / 29.1;

    // Задержка между измерениями для корректной работы скеча
    delay(250);

    return cm;
}

int lastAngle = 0;

void servoJoystick() {
    int X = analogRead(JOYSTICK_PIN_X);              // считываем значение оси Х

    int angle = map(X, 0, 1023, 0, 180);
    servoRotate(servo, lastAngle, angle, 1);
    lastAngle = angle;
}

void enableDigit(int x) {
    for (int i = 0; i < 4; ++i) {
        uint8_t newValue = i == x ? LOW : HIGH;
        if (digitalRead(anodPins[i]) != newValue) {
            digitalWrite(anodPins[i], newValue);
        }
    }
//    digitalWrite(anodPins[x], LOW);
    delay(1);
}

// определение символа (цифры)
//void pickNumber(int x) {
//    switch (x) {
//        default: zero();   break;
//        case 1: one();     break;
//        case 2: two();     break;
//        case 3: three();  break;
//        case 4: four();    break;
//        case 5: five();     break;
//        case 6: six();       break;
//        case 7: seven(); break;
//        case 8: eight();  break;
//        case 9: nine();   break;
//    }
//}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);

    pinMode(JOYSTICK_PIN_X, INPUT);
    pinMode(JOYSTICK_PIN_Y, INPUT);

    pinMode(JOYSTICK_SW, INPUT);
    digitalWrite(JOYSTICK_SW, HIGH);
    servo.attach(0);

    lcd.begin(16, 2);                 //  Инициируем работу с LCD дисплеем, указывая количество (столбцов, строк)
    lcd.setCursor(0, 0);              //  Устанавливаем курсор в позицию (0 столбец, 0 строка)
    lcd.print("LDC1602");             //  Выводим текст "LDC1602", начиная с установленной позиции курсора
    lcd.setCursor(0, 1);

    for (int i = 0; i < 4; i++) {
        pinMode(anodPins[i], OUTPUT);
        digitalWrite(anodPins[i], HIGH);
    }
    for (int i = 0; i < 8; i++) {
        pinMode(segmentsPins[i], OUTPUT);
    }

    pinMode(2, OUTPUT);
}

void displayMessage(int dig[4][8]) {
    for (int i = 0; i < 4; i++) { // Каждый разряд по очереди
//        enableDigit(i);

        digitalWrite(anodPins[i], LOW);

//        delay(200);

        for (int k = 0; k < 8; k++) {// Каждый сегмент по очереди - исходя из заданной карты
//            Serial.print(dig[i][k]);
//            Serial.print(" ");
            digitalWrite(segmentsPins[k], (dig[i][k] ? HIGH : LOW));
        }
//        Serial.println();
//        delay(4);
        digitalWrite(anodPins[i], HIGH);
//        digitalWrite(anodPins[i], HIGH);
//        delay(1);
//        digitalWrite(anodPins[i], LOW);
    }
}

void generateMessage(float value, int arr[4][8]) {
    if ((value >= 10000) ||
        (value <= -1000)) {

        for (int i = 0; i < 4; i++) { // Каждый разряд по очереди
            for (int k = 0; k < 8; k++) {// Каждый сегмент по очереди - исходя из заданной карты
                arr[i][k] = err[i][k];
            }
        }
        return; // Выходим
    }

    int digits = 4; // У нас 4 разряда
    if (value < 0) digits = 3; // Из-за минуса один символ убирается*/

    // Делим число на 2 группы - отдельно целую часть и дробную.
    int intPart = (int)abs(value);
    int intLength = ((String)intPart).length(); // Смотрим, сколько разрядов занимает целая часть

    char valueChars[8]; // По нормальному float в String преобразовать нельзя, поэтому...
    dtostrf(value, 5, 4, valueChars); //... копируем данные float в массив chars[]

    // На дробную часть у нас остается разрядов: digits-intLength
    String valueStr = valueChars;                                                 // Теперь с текстовым форматом можно работать
    //  Serial.print("valueStr: "); Serial.println(valueStr);

    int fracIndex = valueStr.indexOf(".");                                        // Получаем индекс начала дробной части
    String fracPartStr = valueStr.substring(fracIndex + 1, valueStr.length());    // Выделяем цифры дробной части
    int fracDigits = digits - intLength;
    fracPartStr = fracPartStr.substring(0, fracDigits);                           // Вычленяем цифры, которые будут выводиться на дисплей
    //  Serial.print("fracDigits: "); Serial.println(fracDigits);
    //  Serial.print("fracPartStr: "); Serial.println(fracPartStr);

    int fracInt = fracPartStr.toInt();                                            // Переменная для проверки, нужно ли что-то выводить, если нули, то нет
    //  Serial.print("fracInt: "); Serial.println(fracInt);

    // Собираем строку для отображения
    String output = (value < 0) ? "-" : "";
    output += (String)intPart;

    String outputFrac = ((digits - intLength <= 0) || (fracInt == 0)) ? "" : ((String)"." + fracPartStr);

    output += (String)outputFrac;

    // Дополняем символы спереди, если цифр слишком мало, например для "-1" делаем "  -1"
    String spaces = "     ";
    digits = 4;
    if (~output.indexOf(".")) digits += 1;
    if (output.length() < digits) output = spaces.substring(0, digits - output.length()) + output;

    // Формирум данные для показа:
    int dig = -1;
    for (int i = 0; i < output.length(); i++) {
        String _char = output.substring(i, i + 1);

        if (_char != ".") dig += 1; // Точка не занимает сегмент - увеличиваем на 1

        int actualdigit = 11; // По умолчанию пустой символ
        if ((_char == "-")) {
            actualdigit = 10;
        }
        else if (_char == " " || _char == ".") {
        }
        else {
            actualdigit = _char.toInt();
        }

        if (_char == ".") {
            arr[dig][7] = 1; // Если нужно - ставим точку
        }
        else  {
            for (int n = 0; n <= 7; n++) {
                arr[dig][n] = seg[actualdigit][n];
            }
        }
    }
}

void generateMessage(int value, int arr[4][8]) {
    if ((value >= 10000) ||
        (value <= -1000)) {

        for (int i = 0; i < 4; i++) { // Каждый разряд по очереди
            for (int k = 0; k < 8; k++) {// Каждый сегмент по очереди - исходя из заданной карты
                arr[i][k] = err[i][k];
            }
        }
        return; // Выходим
    }

    int firstDigitIndex = 0;
    if (value < 0) {
        value = -value;
        firstDigitIndex = 1;
        for (int i = 0; i < 8; ++i) {
            arr[0][i] = minus[i];
        }
    }

    int used = 0;

    while (value > 0 || used == 0) {
        ++used;
        int currentDigit = value % 10;
        for (int i = 0; i < 8; ++i) {
            arr[4 - used][i] = seg[currentDigit][i];
        }

        value /= 10;
    }

    for (int i = firstDigitIndex; i < 4 - used; ++i) {
        for (int k = 0; k < 8; k++) {// Каждый сегмент по очереди - исходя из заданной карты
            arr[i][k] = empty[k];
        }
    }
}

int t = 0;
int digit = 0;

static int dig[4][8];

void loop() {

    digitalWrite(2, HIGH);
    digitalWrite(2, LOW);

//    int secs = millis() / 1000;
//
//    generateMessage(secs, dig);
//    if (Serial.availableForWrite()) {
//        Serial.println(secs);
//    }
//
//    while (secs == millis() / 1000) {
//        displayMessage(dig);
//    }
//    secs = millis() / 1000;
//
//
//    generateMessage(secs, dig);
//    displayMessage(dig);

//    delay(1000);
}