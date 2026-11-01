
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Taster an GPIO 4 und GND
const int BUTTON_PIN = 4;

// Eigene UUIDs fuer den Judo-Buzzer
#define SERVICE_UUID        "8b7e1000-1234-4567-890a-bcdef0123456"
#define CHARACTERISTIC_UUID "8b7e1001-1234-4567-890a-bcdef0123456"

BLECharacteristic* buttonCharacteristic;

// Tasterentprellung
int lastRawState = HIGH;
int stableButtonState = HIGH;
unsigned long lastChange = 0;
const unsigned long debounceMs = 40;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* server) override {
    Serial.println("Smartphone verbunden");
  }

  void onDisconnect(BLEServer* server) override {
    Serial.println("Smartphone getrennt");
    BLEDevice::startAdvertising();
    Serial.println("Warte auf Smartphone...");
  }
};

void sendButtonState(const char* message) {
  buttonCharacteristic->setValue(message);
  buttonCharacteristic->notify();

  Serial.print("BLE-Nachricht: ");
  Serial.println(message);
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Starte Judo-Buzzer Green BLE...");

  BLEDevice::init("Greenbuzzer");

  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService* service = server->createService(SERVICE_UUID);

  buttonCharacteristic = service->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  buttonCharacteristic->addDescriptor(new BLE2902());
  buttonCharacteristic->setValue("READY");

  service->start();

  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(false);
  BLEDevice::startAdvertising();

  Serial.println("BLE bereit.");
  Serial.println("Bluetooth-Name: Greenbuzzer");
  Serial.println("Warte auf Smartphone...");
}

void loop() {
  int rawState = digitalRead(BUTTON_PIN);

  if (rawState != lastRawState) {
    lastChange = millis();
    lastRawState = rawState;
  }

  if (millis() - lastChange >= debounceMs &&
      rawState != stableButtonState) {

    stableButtonState = rawState;

    if (stableButtonState == LOW) {
      sendButtonState("PRESSED");
    } else {
      sendButtonState("RELEASED");
    }
  }

  delay(5);
}
