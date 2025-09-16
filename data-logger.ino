#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Wire.h>
#include <DHT.h>
#include <EEPROM.h>

// ===== Triggers =====
#define TEMP_MIN 15
#define TEMP_MAX 35
#define HUM_MIN 30
#define HUM_MAX 70
#define LUM_MIN 0
#define LUM_MAX 70

int eepromAddr = 0; // posição atual de gravação na EEPROM
// ================== Definições ==================
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define BUTTON_NEXT 3    // Botão de passar tela
#define BUTTON_PREV 5    // Botão de voltar tela
#define BUTTON_CONFIRM 4 // Botão de confirmar
#define BUZZER 6         // BUZZER
#define LED_VERMELHO 7
#define LED_VERDE 8
#define LDRPIN A2    // LDR agora está no A2
#define JOY_X A0     // eixo X do joystick
#define JOY_Y A1     // eixo Y (não usado)
#define JOY_BUTTON 3 // botão do joystick
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 RTC;
// int ldrPin = A0;  // LDR no pino analógico
//  Controle de telas principais
int currentScreen = 0; // 0 = Hora, 1 = Logo, 2 = Sensores, 3 = Configurações
int totalScreens = 4;
// Controle do menu de configurações
bool inConfigMenu = false;
int configOption = 0; // 0 = Unidade, 1 = Alternância, 2 = Saída
int totalConfigOptions = 3;
// Configurações do usuário
bool useFahrenheit = false; // false = Celsius, true = Fahrenheit
bool autoMode = false;      // false = manual, true = automático
unsigned long lastAutoSwitch = 0;
unsigned long autoInterval = 5000; // 5 segundos
unsigned long lastBuzzerTime = 0;
unsigned long buzzerInterval = 500; // tempo em ms para alternar buzzer
// ================== Controle de botões ==================
bool lastNextState = HIGH;
bool lastPrevState = HIGH;
bool lastConfirmState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
// ================== Variáveis da animação do Logo ==================
String brand = "WINEDGE";
String strDataLog = "Computing";
int x = 9;
int p = 5;
// ================== Caracteres customizados ==================
byte w0x0[] = {B00000, B11111, B11111, B11111, B11111, B11111, B11111, B11111};
byte w0x1[] = {B00000, B11000, B11110, B11110, B11110, B11111, B11111, B11111};
byte w1x0[] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B00000};
byte w1x1[] = {B11111, B11111, B11111, B11110, B11110, B11110, B11000, B00000};
byte w0x2[] = {B00000, B00000, B00000, B00000, B00000, B10000, B11111, B11111};
byte w0x3[] = {B00000, B00000, B00000, B00000, B00000, B00000, B11110, B11110};
byte w1x2[] = {B11111, B11111, B10000, B00000, B00000, B00000, B00000, B00000};
byte w1x3[] = {B11110, B11110, B00000, B00000, B00000, B00000, B00000, B00000};
// ================== Variáveis de sensores ==================
float currentTemp = 0.0;
float currentHum = 0.0;
int currentLDR = 0;
unsigned long lastSensorRead = 0;
unsigned long sensorInterval = 2000; // atualiza a cada 2s
// ================== Setup ==================
void setup()
{
    pinMode(JOY_BUTTON, INPUT_PULLUP);
    pinMode(LED_VERMELHO, OUTPUT);
    pinMode(LED_VERDE, OUTPUT);

    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    dht.begin();

    if (!RTC.begin())
    {
        Serial.println("RTC nao encontrado!");
        while (1)
            ;
    }

    // RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));  // ajusta automaticamente na primeira vez

    // cria os caracteres do logo
    lcd.createChar(0, w0x0);
    lcd.createChar(1, w0x1);
    lcd.createChar(2, w1x0);
    lcd.createChar(3, w1x1);
    lcd.createChar(4, w0x2);
    lcd.createChar(5, w0x3);
    lcd.createChar(6, w1x2);
    lcd.createChar(7, w1x3);

    // Registrar a unidade ativa ao iniciar
    EEPROM.write(0, useFahrenheit ? 'F' : 'C'); // endereço 0 apenas para registro da unidade
    Serial.print("Unidade inicial gravada na EEPROM: ");
    Serial.println(useFahrenheit ? "Fahrenheit" : "Celsius");
}

// ================== Loop ==================
void loop()
{
    handleButtons();
    readSensors(); // <- sempre atualiza sensores

    if (!inConfigMenu)
    {
        if (autoMode)
        { // alternância automática
            if (millis() - lastAutoSwitch > autoInterval)
            {
                currentScreen = (currentScreen + 1) % totalScreens;
                lastAutoSwitch = millis();
            }
        }
        // Telas principais
        if (currentScreen == 0)
            showTime();
        else if (currentScreen == 1)
            showLogo();
        else if (currentScreen == 2)
            showSensors(); // aqui só mostra os valores já lidos
        else if (currentScreen == 3)
            showConfigAccess();
    }
    else
    {
        // Menu interno de configurações
        showConfigMenu();
    }
    delay(100);
}

float getActiveTemp(float tempC)
{
    if (useFahrenheit)
        return tempC * 9.0 / 5.0 + 32;
    else
        return tempC;
}

// ================== Leitura de Sensores ==================
void readSensors()
{
    if (millis() - lastSensorRead > sensorInterval)
    {
        lastSensorRead = millis();

        currentHum = dht.readHumidity();
        float tempC = dht.readTemperature(); // sempre em Celsius
        currentTemp = getActiveTemp(tempC);  // valor já convertido conforme a unidade
        currentLDR = analogRead(LDRPIN);
        if (currentLDR > 1023)
            currentLDR = 1023;

        // Normaliza LDR em % (0 a 100)
        int ldrPercent = map(currentLDR, 0, 1023, 100, 0);

        // --- Debug no Serial Monitor ---
        Serial.print("Temp: ");
        Serial.print(currentTemp, 1);
        Serial.print(useFahrenheit ? " F" : " C");
        Serial.print(" | Umid: ");
        Serial.print(currentHum, 0);
        Serial.print(" % | Luz: ");
        Serial.print(ldrPercent);
        Serial.println(" %");

        // Checa triggers usando o valor original em Celsius para não interferir nos limites
        checkTriggers(tempC, currentHum, ldrPercent);
    }
}

// ================== Funções de Telas ==================
void showTime()
{
    DateTime now = RTC.now();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DATA: ");
    lcd.print(now.day() < 10 ? "0" : "");
    lcd.print(now.day());
    lcd.print("/");
    lcd.print(now.month() < 10 ? "0" : "");
    lcd.print(now.month());
    lcd.print("/");
    lcd.print(now.year());
    lcd.setCursor(0, 1);
    lcd.print("HORA: ");
    lcd.print(now.hour() < 10 ? "0" : "");
    lcd.print(now.hour());
    lcd.print(":");
    lcd.print(now.minute() < 10 ? "0" : "");
    lcd.print(now.minute());
    lcd.print(":");
    lcd.print(now.second() < 10 ? "0" : "");
    lcd.print(now.second());
}
void showLogo()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(byte(0));
    lcd.setCursor(1, 0);
    lcd.write(byte(1));
    lcd.setCursor(0, 1);
    lcd.write(byte(2));
    lcd.setCursor(1, 1);
    lcd.write(byte(3));
    lcd.setCursor(2, 0);
    lcd.write(byte(4));
    lcd.setCursor(3, 0);
    lcd.write(byte(5));
    lcd.setCursor(2, 1);
    lcd.write(byte(6));
    lcd.setCursor(3, 1);
    lcd.write(byte(7));
    lcd.setCursor(5, 0);
    lcd.print(brand);
    lcd.setCursor(p, 1);
    lcd.print(strDataLog.substring(x));
    x--;
    if (x < 0)
    {
        while (p <= strDataLog.length() * 2)
        {
            lcd.setCursor(p, 1);
            lcd.print(" " + strDataLog);
            delay(200);
            p++;
        }
        x = 9;
        p = 5;
    }
}
void showSensors()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(currentTemp, 1); // já convertido
    lcd.print(useFahrenheit ? "F " : "C ");
    lcd.print("U:");
    lcd.print(currentHum, 0);
    lcd.print("%");

    int ldrPercent = map(currentLDR, 0, 1023, 100, 0);
    lcd.setCursor(0, 1);
    lcd.print("Luz:");
    lcd.print(ldrPercent);
    lcd.print("%");
}

void showConfigAccess()
{
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Configuracoes");
    lcd.setCursor(0, 1);
    lcd.print("Press. Confirma");
}
void showConfigMenu()
{
    lcd.clear();
    int optionOnTop = configOption;                               // opção atual
    int optionOnBottom = (configOption + 1) % totalConfigOptions; // próxima
    // --- Primeira linha ---
    lcd.setCursor(0, 0);
    lcd.print("> ");
    printConfigOption(optionOnTop);
    // --- Segunda linha ---
    lcd.setCursor(0, 1);
    lcd.print("  ");
    printConfigOption(optionOnBottom);
}
void printConfigOption(int option)
{
    if (option == 0)
    { // Unidade
        lcd.print("Unidade:");
        lcd.print(useFahrenheit ? "F" : "C");
    }
    else if (option == 1)
    { // Alternância
        lcd.print("Altern:");
        lcd.print(autoMode ? "Auto" : "Manual");
    }
    else if (option == 2)
    { // Saída
        lcd.print("Saida");
    }
}

// ================== Controle de Botões ==================
void handleButtons()
{
    unsigned long currentMillis = millis();

    // Leitura joystick
    int joyX = analogRead(JOY_X);
    bool confirmState = digitalRead(JOY_BUTTON);

    // Define thresholds para esquerda/direita
    const int LEFT_THRESHOLD = 300;
    const int RIGHT_THRESHOLD = 700;

    // --- Next / Previous via X ---
    if (joyX < LEFT_THRESHOLD && currentMillis - lastDebounceTime > debounceDelay)
    {
        lastDebounceTime = currentMillis;
        if (!inConfigMenu)
        {
            currentScreen = (currentScreen - 1 + totalScreens) % totalScreens;
        }
        else
        {
            configOption = (configOption - 1 + totalConfigOptions) % totalConfigOptions;
        }
    }
    if (joyX > RIGHT_THRESHOLD && currentMillis - lastDebounceTime > debounceDelay)
    {
        lastDebounceTime = currentMillis;
        if (!inConfigMenu)
        {
            currentScreen = (currentScreen + 1) % totalScreens;
        }
        else
        {
            configOption = (configOption + 1) % totalConfigOptions;
        }
    }

    // --- Confirm ---
    if (lastConfirmState == HIGH && confirmState == LOW && currentMillis - lastDebounceTime > debounceDelay)
    {
        lastDebounceTime = currentMillis;
        if (!inConfigMenu)
        {
            if (currentScreen == 3)
            {
                inConfigMenu = true;
                configOption = 0;
            }
        }
        else
        {
            if (configOption == 0)
                toggleUnit();
            else if (configOption == 1)
                toggleMode();
            else if (configOption == 2)
            {
                inConfigMenu = false;
                currentScreen = 0;
            }
        }
    }

    lastConfirmState = confirmState;
}

// ================== Placeholder ==================
void toggleUnit()
{
    useFahrenheit = !useFahrenheit;
    Serial.print("Unidade: ");
    Serial.println(useFahrenheit ? "Fahrenheit" : "Celsius");

    // Grava a unidade na EEPROM ao mudar
    EEPROM.write(0, useFahrenheit ? 'F' : 'C');
}

void toggleMode()
{
    autoMode = !autoMode;
    Serial.print("Modo: ");
    Serial.println(autoMode ? "Automatico" : "Manual");
}

//================TRIGGERS===================

void checkTriggers(float temp, float hum, int lum)
{
    bool triggered = false;
    String msg = "";

    // Temperatura
    if (temp <= TEMP_MIN || temp >= TEMP_MAX)
    {
        logToEEPROM('T', (int)temp);
        msg += "Temp fora da faixa! ";
        triggered = true;
    }

    // Umidade
    if (hum <= HUM_MIN || hum >= HUM_MAX)
    {
        logToEEPROM('U', (int)hum);
        msg += "Umid fora da faixa! ";
        triggered = true;
    }

    // Luminosidade
    if (lum <= LUM_MIN || lum >= LUM_MAX)
    {
        logToEEPROM('L', lum);
        msg += "Luz fora da faixa! ";
        triggered = true;
    }

    unsigned long currentMillis = millis();

    // Controle de LEDs e Buzzer
    if (triggered)
    {
        digitalWrite(LED_VERMELHO, HIGH); // acende vermelho
        digitalWrite(LED_VERDE, LOW);     // apaga verde

        // Buzzer intermitente
        if (currentMillis - lastBuzzerTime >= buzzerInterval)
        {
            lastBuzzerTime = currentMillis;
            if (digitalRead(BUZZER) == LOW)
                tone(BUZZER, 1000); // liga buzzer
            else
                noTone(BUZZER); // desliga buzzer
        }

        Serial.println("Trigger ativada: " + msg);
    }
    else
    {
        digitalWrite(LED_VERDE, HIGH);   // acende verde
        digitalWrite(LED_VERMELHO, LOW); // apaga vermelho
        noTone(BUZZER);                  // garante buzzer desligado
    }
}

void logToEEPROM(char type, float valueC)
{
    DateTime now = RTC.now(); // pega o tempo atual
    int valueToSave = 0;

    if (type == 'T')
        valueToSave = (int)getActiveTemp(valueC);
    else
        valueToSave = (int)valueC;

    if (eepromAddr + 7 < EEPROM.length())
    {
        EEPROM.write(eepromAddr, type);
        EEPROM.write(eepromAddr + 1, valueToSave);
        EEPROM.write(eepromAddr + 2, now.hour());
        EEPROM.write(eepromAddr + 3, now.minute());
        EEPROM.write(eepromAddr + 4, now.day());
        EEPROM.write(eepromAddr + 5, now.month());
        EEPROM.write(eepromAddr + 6, useFahrenheit ? 'F' : 'C');
        eepromAddr += 7;

        Serial.print("Gravado na EEPROM: ");
        Serial.print(type);
        Serial.print(" = ");
        Serial.print(valueToSave);
        Serial.print(" ");
        Serial.print(useFahrenheit ? "F" : "C");
        Serial.print(" em ");
        Serial.print(now.day());
        Serial.print("/");
        Serial.print(now.month());
        Serial.print("/");
        Serial.print(now.year());
        Serial.print(" ");
        Serial.print(now.hour());
        Serial.print(":");
        Serial.println(now.minute());
    }
    else
    {
        Serial.println("EEPROM cheia!");
    }
}
