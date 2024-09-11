#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10 // ss du mfrc522
#define RST_PIN 9  // rst du mfrc522
#define BUTTON_PIN 2  // pin du boutton
#define CREDIT_BLOCK 28 // blocs pour les crédits

const int L1 = 3;   // LED rouge
const int L2 = 4;   // LED verte
const int BUZZ = 7; // Buzzer

MFRC522 mfrc522(SS_PIN, RST_PIN);

int mode = 0; // 0 = Vérification, 1 = Enregistrement, 2 = Test
byte storedUID[7];
byte storedUIDSize = 0;
bool isUIDStored = false;
unsigned long recordingStartTime = 0;


MFRC522::MIFARE_Key keyA = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; // Clefs par défauts changer en fonction de votre config
MFRC522::MIFARE_Key keyB = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };


void setup() {
    Serial.begin(9600);
    while (!Serial);

    SPI.begin();
    mfrc522.PCD_Init();
    delay(4);
    mfrc522.PCD_DumpVersionToSerial();

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(L1, OUTPUT);
    pinMode(L2, OUTPUT);
    pinMode(BUZZ, OUTPUT);

    Serial.println("Approchez la carte RFID");
    Serial.println("Appuyez sur le bouton pour changer de mode.");
}

void loop() {
    static bool lastButtonState = HIGH;
    bool currentButtonState = digitalRead(BUTTON_PIN);

    if (lastButtonState == HIGH && currentButtonState == LOW) {
        mode = (mode + 1) % 3; // Cycle entre 0, 1 et 2
        if (mode == 1) {
            recordingStartTime = millis();
        }
        Serial.println(mode == 0 ? "Mode Vérification" : (mode == 1 ? "Mode Enregistrement" : "Mode Test"));
        delay(500);
    }
    lastButtonState = currentButtonState;

    // Gestion des LEDs
    if (mode == 1) {
        digitalWrite(L1, HIGH);
        digitalWrite(L2, millis() % 500 < 250 ? HIGH : LOW);
        if (millis() - recordingStartTime >= 15000) {
            mode = 0;
            Serial.println("Retour automatique en mode Vérification après 15 secondes.");
            digitalWrite(L1, LOW);
            digitalWrite(L2, LOW);
        }
    } else if (mode == 2) {
        digitalWrite(L1, millis() % 1000 < 500 ? HIGH : LOW);
        digitalWrite(L2, millis() % 1000 < 500 ? LOW : HIGH);
    } else {
        digitalWrite(L1, LOW);
        digitalWrite(L2, LOW);
    }

    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        Serial.print("UID de la carte : ");
        for (byte i = 0; i < mfrc522.uid.size; i++) {
            Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
            Serial.print(mfrc522.uid.uidByte[i], HEX);
        }
        Serial.println();

        if (mode == 1) {
            registerCard();
        } else if (mode == 0 && isUIDStored) {
            verifyCard();
        } else if (mode == 2) {
            testCard();
        }

        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
    }
}

bool readBlock(byte blockAddr, byte *data) {
    MFRC522::StatusCode status;
    byte trailerBlock = blockAddr / 4 * 4 + 3;
    byte bufferLen = 18;

    // Tentative avec la clé A
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &keyA, &(mfrc522.uid));
    if (status == MFRC522::STATUS_OK) {
        status = mfrc522.MIFARE_Read(blockAddr, data, &bufferLen);
        if (status == MFRC522::STATUS_OK) {
            return true;
        }
    }

    Serial.print("Erreur de lecture avec la clé A : ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;  // Ne tente pas la clé B si la clé A échoue
}

bool writeBlock(byte blockAddr, byte *data) {
    MFRC522::StatusCode status;
    byte trailerBlock = blockAddr / 4 * 4 + 3;

    // Tentative avec la clé A seulement
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &keyA, &(mfrc522.uid));
    if (status == MFRC522::STATUS_OK) {
        status = mfrc522.MIFARE_Write(blockAddr, data, 16);
        if (status == MFRC522::STATUS_OK) {
            return true;
        }
    }

    Serial.print("Erreur d'écriture avec la clé A : ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;  // Ne tente pas la clé B si la clé A échoue
}

void registerCard() {
    Serial.println("Enregistrement de la carte...");
    storedUIDSize = mfrc522.uid.size;
    for (byte i = 0; i < storedUIDSize; i++) {
        storedUID[i] = mfrc522.uid.uidByte[i];
    }
    isUIDStored = true;

    // Lire le bloc avant d'écrire
    Serial.println("Lecture du bloc avant écriture...");
    byte readData[16];
    if (readBlock(CREDIT_BLOCK, readData)) {
        Serial.print("Contenu avant écriture : ");
        for (int i = 0; i < 16; i++) {
            Serial.print(readData[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    } else {
        Serial.println("Erreur lors de la lecture du bloc avant écriture.");
        return;
    }

    // Attendre l'entrée du crédit pendant 5 secondes
    int initialCredits = 10; // Crédit par défaut
    Serial.println("Entrez le montant de crédits (par défaut : 10) :");
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) {
        if (Serial.available() > 0) {
            String creditStr = Serial.readStringUntil('\n');
            initialCredits = creditStr.toInt();
            if (initialCredits <= 0) {
                initialCredits = 10;
            }
            break;
        }
    }

    Serial.print("Crédits enregistrés : ");
    Serial.println(initialCredits);

    byte creditData[16] = {0};
    creditData[0] = (initialCredits >> 8) & 0xFF;
    creditData[1] = initialCredits & 0xFF;

    // Écrire les données
    if (writeBlock(CREDIT_BLOCK, creditData)) {
        Serial.println("Données écrites avec succès.");
    } else {
        Serial.println("Erreur lors de l'écriture des données.");
        return;
    }

    // Lire le bloc après écriture pour vérification
    if (readBlock(CREDIT_BLOCK, readData)) {
        Serial.print("Contenu après écriture : ");
        for (int i = 0; i < 16; i++) {
            Serial.print(readData[i], HEX);
            Serial.print(" ");
        }
        Serial.println();

        // Vérifier les données écrites
        if (readData[0] == creditData[0] && readData[1] == creditData[1]) {
            Serial.println("Carte enregistrée avec succès !");
            Serial.print("Crédits initiaux : ");
            Serial.println(initialCredits);
        } else {
            Serial.println("Erreur : Données écrites non vérifiées.");
        }
    } else {
        Serial.println("Erreur lors de la lecture des données après écriture.");
    }

    tone(BUZZ, 1500, 500);
    delay(1000);
    mode = 0;
    Serial.println("Retour au mode vérification.");
    digitalWrite(L1, LOW);
    digitalWrite(L2, LOW);
}

void verifyCard() {
    bool accessGranted = true;
    if (mfrc522.uid.size != storedUIDSize) {
        accessGranted = false;
    } else {
        for (byte i = 0; i < storedUIDSize; i++) {
            if (mfrc522.uid.uidByte[i] != storedUID[i]) {
                accessGranted = false;
                break;
            }
        }
    }

    if (accessGranted) {
        byte blockData[16];
        if (readBlock(CREDIT_BLOCK, blockData)) {
            int credits = (blockData[0] << 8) | blockData[1];
            Serial.print("Crédits disponibles : ");
            Serial.println(credits);

            if (credits > 0) {
                credits--;
                byte newData[16] = {0};
                newData[0] = (credits >> 8) & 0xFF;
                newData[1] = credits & 0xFF;

                if (writeBlock(CREDIT_BLOCK, newData)) {
                    Serial.print("Accès autorisé. Crédits restants : ");
                    Serial.println(credits);
                    digitalWrite(L2, HIGH);
                    tone(BUZZ, 1000, 500);
                } else {
                    Serial.println("Erreur lors de la mise à jour des crédits.");
                    digitalWrite(L1, HIGH);
                    tone(BUZZ, 500, 500);
                }
            } else {
                Serial.println("Crédits insuffisants.");
                digitalWrite(L1, HIGH);
                tone(BUZZ, 500, 500);
            }
        } else {
            Serial.println("Erreur lors de la lecture des crédits.");
        }
    } else {
        Serial.println("Accès refusé.");
        digitalWrite(L1, HIGH);
        tone(BUZZ, 500, 500);
    }

    delay(1000);
    digitalWrite(L1, LOW);
    digitalWrite(L2, LOW);
}

void testCard() {
    // Fonctionnalité de test pour le mode 2
    Serial.println("Mode Test - Affichage du contenu du bloc de crédit");

    byte blockData[16];
    if (readBlock(CREDIT_BLOCK, blockData)) {
        Serial.print("Contenu du bloc : ");
        for (int i = 0; i < 16; i++) {
            Serial.print(blockData[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    } else {
        Serial.println("Erreur lors de la lecture du bloc.");
    }
}
