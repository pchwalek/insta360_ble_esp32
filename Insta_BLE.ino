/*
    Author: Patrick Chwalek (09/22/2023)
    Description: Example code used to control Insta360 X3 and RS 1-inch cameras
*/

#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>

#define SERVICE_UUID "ce80"
#define CHARACTERISTIC_UUID1 "ce81"
#define CHARACTERISTIC_UUID2 "ce82"
#define CHARACTERISTIC_UUID3 "ce83"

#define SECONDARY_SERVICE_UUID "0000D0FF-3C17-D293-8E48-14FE2E4DA212"
#define SECONDARY_CHARACTERISTIC_UUID1 "ffd1"
#define SECONDARY_CHARACTERISTIC_UUID2 "ffd2"
#define SECONDARY_CHARACTERISTIC_UUID3 "ffd3"
#define SECONDARY_CHARACTERISTIC_UUID4 "ffd4"
#define SECONDARY_CHARACTERISTIC_UUID5 "ffd5"
#define SECONDARY_CHARACTERISTIC_UUID8 "ffd8"
#define SECONDARY_CHARACTERISTIC_UUID9 "fff1"
#define SECONDARY_CHARACTERISTIC_UUID10 "fff2"
#define SECONDARY_CHARACTERISTIC_UUID11 "ffe0"

#define CAPTURE_DELAY 120000

uint32_t data_32;
uint16_t data_16;
uint8_t data_8[30];

BLEServer *pServer = NULL;
BLE2902 *pDescriptor2902;

bool deviceConnected = false;
bool oldDeviceConnected = false;

BLECharacteristic *pCharacteristicRx;

unsigned long myTime;

uint8_t manuf_data[30];

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
        deviceConnected = true;
        myTime = millis();
        BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer *pServer) { deviceConnected = false; }
};

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE work!");

    BLEDevice::init("Insta360 GPS Remote");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pCharacteristic1 = pService->createCharacteristic(
        CHARACTERISTIC_UUID1, BLECharacteristic::PROPERTY_WRITE);

    pCharacteristicRx = pService->createCharacteristic(
        CHARACTERISTIC_UUID2, BLECharacteristic::PROPERTY_NOTIFY);
    pDescriptor2902 = new BLE2902();
    pDescriptor2902->setNotifications(true);
    pDescriptor2902->setIndications(true);
    pCharacteristicRx->addDescriptor(pDescriptor2902);
    data_8[0] = 0;
    pCharacteristicRx->setValue(data_8, 1);

    BLECharacteristic *pCharacteristic3 = pService->createCharacteristic(
        CHARACTERISTIC_UUID3, BLECharacteristic::PROPERTY_READ);
    data_16 = 0x0201;
    pCharacteristic3->setValue(data_16);

    BLEService *pService2 = pServer->createService(SECONDARY_SERVICE_UUID);
    BLECharacteristic *secondary_pCharacteristic1 =
        pService2->createCharacteristic(SECONDARY_CHARACTERISTIC_UUID1,
                                        BLECharacteristic::PROPERTY_WRITE);

    BLECharacteristic *secondary_pCharacteristic2 =
        pService2->createCharacteristic(SECONDARY_CHARACTERISTIC_UUID2,
                                        BLECharacteristic::PROPERTY_READ);

    BLECharacteristic *secondary_pCharacteristic3 =
        pService2->createCharacteristic(SECONDARY_CHARACTERISTIC_UUID3,
                                        BLECharacteristic::PROPERTY_READ);
    data_32 = 0x301e9001;
    secondary_pCharacteristic3->setValue(data_32);

    BLECharacteristic *secondary_pCharacteristic4 =
        pService2->createCharacteristic(SECONDARY_CHARACTERISTIC_UUID4,
                                        BLECharacteristic::PROPERTY_READ);

    data_32 = 0x18002001;
    secondary_pCharacteristic4->setValue(data_32);

    BLECharacteristic *secondary_pCharacteristic5 =
        pService2->createCharacteristic(SECONDARY_CHARACTERISTIC_UUID5,
                                        BLECharacteristic::PROPERTY_READ);

    BLECharacteristic *secondary_pCharacteristic8 =
        pService2->createCharacteristic(SECONDARY_CHARACTERISTIC_UUID8,
                                        BLECharacteristic::PROPERTY_WRITE);

    BLECharacteristic *secondary_pCharacteristic9 =
        pService2->createCharacteristic(SECONDARY_CHARACTERISTIC_UUID9,
                                        BLECharacteristic::PROPERTY_READ);

    BLECharacteristic *secondary_pCharacteristic10 =
        pService2->createCharacteristic(SECONDARY_CHARACTERISTIC_UUID10,
                                        BLECharacteristic::PROPERTY_WRITE);

    BLECharacteristic *secondary_pCharacteristic11 =
        pService2->createCharacteristic(SECONDARY_CHARACTERISTIC_UUID11,
                                        BLECharacteristic::PROPERTY_READ);

    pService->start();
    pService2->start();
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still
    // is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->addServiceUUID(SECONDARY_SERVICE_UUID);

    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(
        0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);

    BLEDevice::startAdvertising();
    Serial.println(
        "Characteristic defined! Now you can read it in your phone!");

    /* set the manufacturing data for wakeon packet */
    manuf_data[0] = 0x4c;
    manuf_data[1] = 0x00;
    manuf_data[2] = 0x02;
    manuf_data[3] = 0x15;
    manuf_data[4] = 0x09;
    manuf_data[5] = 0x4f;
    manuf_data[6] = 0x52;
    manuf_data[7] = 0x42;
    manuf_data[8] = 0x49;
    manuf_data[9] = 0x54;
    manuf_data[10] = 0x09;
    manuf_data[11] = 0xff;
    manuf_data[12] = 0x0f;
    manuf_data[13] = 0x00;
    /* note: see powerOnPrevConnectedCameras() for bytes 14-19 */
    manuf_data[20] = 0x00;
    manuf_data[21] = 0x00;
    manuf_data[22] = 0x00;
    manuf_data[23] = 0x00;
    manuf_data[24] = 0xe4;
    manuf_data[25] = 0x01;
}

void loop() {
    // notify changed value
    if (deviceConnected) {
        delay(10); // bluetooth stack will go into congestion, if too many packets
    }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        Serial.println("connecting");
        oldDeviceConnected = deviceConnected;
    }

    /* this loop just goes through all the features */
    if (((millis() - myTime) > CAPTURE_DELAY)) {
        Serial.println("capture");
        myTime = millis();
        shutterButton(pCharacteristicRx);
        delay(30000);

        Serial.println("screen off");
        screenToggle(pCharacteristicRx);
        delay(5000);

        Serial.println("screen on");
        screenToggle(pCharacteristicRx);
        delay(5000);
        
        Serial.println("power off");
        powerOff(pCharacteristicRx);
        delay(30000);

        Serial.println("power on");
        BLEDevice::stopAdvertising();
        powerOnPrevConnectedCameras();
    }
}

void screenToggle(BLECharacteristic *characteristic) {
    data_8[0] = 0xfc;
    data_8[1] = 0xef;
    data_8[2] = 0xfe;
    data_8[3] = 0x86;
    data_8[4] = 0x00;
    data_8[5] = 0x03;
    data_8[6] = 0x01;
    data_8[7] = 0x00;
    data_8[8] = 0x00;
    characteristic->setValue(data_8, 9);
    characteristic->notify();
}

void powerOff(BLECharacteristic *characteristic) {
    data_8[0] = 0xfc;
    data_8[1] = 0xef;
    data_8[2] = 0xfe;
    data_8[3] = 0x86;
    data_8[4] = 0x00;
    data_8[5] = 0x03;
    data_8[6] = 0x01;
    data_8[7] = 0x00;
    data_8[8] = 0x03;
    characteristic->setValue(data_8, 9);
    characteristic->notify();
}

void powerOnPrevConnectedCameras() {
    /* the below might be camera specific and you may need to sniff them yourself for your camera */

    /* used for Insta360 X3 */
    // manuf_data[14] = 0x37;
    // manuf_data[15] = 0x4b;
    // manuf_data[16] = 0x43;
    // manuf_data[17] = 0x4d;
    // manuf_data[18] = 0x54;
    // manuf_data[19] = 0x4b;

    /* used for Insta360 RS 1-inch */
    manuf_data[14] = 0x38;
    manuf_data[15] = 0x51;
    manuf_data[16] = 0x53;
    manuf_data[17] = 0x4a;
    manuf_data[18] = 0x38;
    manuf_data[19] = 0x52;

    // BLEAdvertisementData advertisementData;
    // advertisementData.setName("Insta360 GPS Remote");
    // advertisementData.setFlags(ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_DMT_CONTROLLER_SPT | ESP_BLE_ADV_FLAG_DMT_HOST_SPT);
    // advertisementData.setManufacturerData((char *) manuf_data);

    // BLEAdvertising* advertisement = new BLEAdvertising();
    // advertisement->setAdvertisementData(advertisementData);
	  // advertisement->start();

    // custom function that adds manufacturing data and modifies flags of advertisement
    BLEDevice::startAdvertisingWithManufData(manuf_data);
}

void modeButton(BLECharacteristic *characteristic) {
    data_8[0] = 0xfc;
    data_8[1] = 0xef;
    data_8[2] = 0xfe;
    data_8[3] = 0x86;
    data_8[4] = 0x00;
    data_8[5] = 0x03;
    data_8[6] = 0x01;
    data_8[7] = 0x01;
    data_8[8] = 0x00;
    characteristic->setValue(data_8, 9);
    characteristic->notify();
}

void shutterButton(BLECharacteristic *characteristic) {
    data_8[0] = 0xfc;
    data_8[1] = 0xef;
    data_8[2] = 0xfe;
    data_8[3] = 0x86;
    data_8[4] = 0x00;
    data_8[5] = 0x03;
    data_8[6] = 0x01;
    data_8[7] = 0x02;
    data_8[8] = 0x00;
    characteristic->setValue(data_8, 9);
    characteristic->notify();
}