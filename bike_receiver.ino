
#include <RH_ASK.h>
#include <SPI.h>
#include <string.h>
#include <LiquidCrystal.h>

// RF receiver: DATA -> D2
RH_ASK rf(2000, 2, 12, 10);

const int RELAY_PIN = 7;

// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(3, 4, 5, 6, 8, 9);

// Active-LOW relay module
const int RELAY_ON = LOW;
const int RELAY_OFF = HIGH;

const unsigned long LOCKOUT_TIME = 30000UL;
const unsigned long SIGNAL_TIMEOUT = 2000UL;

unsigned long lastMessageTime = 0;
unsigned long lockoutStart = 0;

bool allowStart = false;
bool haveMessage = false;
bool lockoutActive = false;
bool relayWasOn = false;

bool helmetDetected = false;
bool alcoholDetected = false;

void showLCD(const char *line1, const char *line2) {
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(line1);

  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void setup() {
  Serial.begin(9600);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);

  lcd.begin(16, 2);
  showLCD("Smart Helmet", "Starting...");

  if (rf.init()) {
    Serial.println("Bike receiver ready");
    showLCD("Bike Receiver", "Ready");
  } else {
    Serial.println("RF receiver initialization failed!");
    showLCD("RF Init Failed", "Check Receiver");
  }
}

void loop() {
  uint8_t buffer[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t bufferLength = sizeof(buffer);

  if (rf.recv(buffer, &bufferLength)) {
    char message[RH_ASK_MAX_MESSAGE_LEN + 1];

    memcpy(message, buffer, bufferLength);
    message[bufferLength] = '\0';

    if (strcmp(message, "ALLOW") == 0 ||
        strcmp(message, "NOHELMET") == 0 ||
        strcmp(message, "ALCOHOL") == 0) {

      lastMessageTime = millis();
      haveMessage = true;

      Serial.print("Received: ");
      Serial.println(message);

      if (strcmp(message, "ALCOHOL") == 0) {
        allowStart = false;
        alcoholDetected = true;

        if (!lockoutActive) {
          lockoutActive = true;
          lockoutStart = millis();

          Serial.println("ALCOHOL DETECTED");
          Serial.println("Motor locked for 30 seconds");
        }
      }
      else if (strcmp(message, "NOHELMET") == 0) {
        allowStart = false;
        helmetDetected = false;
        alcoholDetected = false;
      }
      else if (strcmp(message, "ALLOW") == 0) {
        allowStart = true;
        helmetDetected = true;
        alcoholDetected = false;
      }
    }
  }

  bool signalFresh =
      haveMessage &&
      (millis() - lastMessageTime <= SIGNAL_TIMEOUT);

  bool timerFinished =
      lockoutActive &&
      (millis() - lockoutStart >= LOCKOUT_TIME);

  bool canStart =
      signalFresh &&
      allowStart &&
      (!lockoutActive || timerFinished);

  if (canStart) {
    digitalWrite(RELAY_PIN, RELAY_ON);

    if (lockoutActive && timerFinished) {
      lockoutActive = false;
      Serial.println("30-second lockout completed");
    }
  } else {
    digitalWrite(RELAY_PIN, RELAY_OFF);
  }

  if (canStart != relayWasOn) {
    Serial.println(canStart ? "Motor: ON" : "Motor: OFF");
    relayWasOn = canStart;
  }

  // LCD status display
  char line1[17];
  char line2[17];

  if (lockoutActive && !timerFinished) {
    unsigned long remaining =
        (LOCKOUT_TIME - (millis() - lockoutStart) + 999) / 1000;

    snprintf(line1, sizeof(line1), "ALCOHOL DETECTED");
    snprintf(line2, sizeof(line2), "Wait: %lu sec", remaining);
  }
  else if (!signalFresh) {
    snprintf(line1, sizeof(line1), "RF Signal Lost");
    snprintf(line2, sizeof(line2), "Motor: OFF");
  }
  else if (alcoholDetected) {
    snprintf(line1, sizeof(line1), "ALCOHOL DETECTED");
    snprintf(line2, sizeof(line2), "Motor: LOCKED");
  }
  else if (!helmetDetected) {
    snprintf(line1, sizeof(line1), "Helmet: NO");
    snprintf(line2, sizeof(line2), "Motor: OFF");
  }
  else {
    snprintf(line1, sizeof(line1), "Helmet: YES");

    if (canStart) {
      snprintf(line2, sizeof(line2), "Alcohol: SAFE");
    } else {
      snprintf(line2, sizeof(line2), "Motor: LOCKED");
    }
  }

  showLCD(line1, line2);
  delay(100);
}


