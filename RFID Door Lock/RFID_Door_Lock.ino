#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define SS_PIN 10
#define RST_PIN 9
#define SERVO_PIN 3

#define unlockDuration 5000 // Duration (in milliseconds) for which the latch remains unlocked
#define rotationAngle 50    // The position upto which Servo rotates (in degrees)

Servo latchServo;
MFRC522 rfidReader(SS_PIN, RST_PIN);

// Define an array of authorized UIDs
// IMPORTANT: Replace the placeholder UIDs below with the actual UIDs of your authorized RFID tags.
// You can find your tag's UID by running the original code and scanning your tag.
// Add more UIDs as needed, separated by commas.
String authorizedUIDs[] = {
  "1a2b3c4d", // Example UID 1: Replace with your first authorized tag's UID
  "e5f6g7h8", // Example UID 2: Replace with your second authorized tag's UID
  "9i0j1k2l"  // Example UID 3: Replace with your third authorized tag's UID
};

// Calculate the number of authorized UIDs in the array
int numAuthorizedUIDs = sizeof(authorizedUIDs) / sizeof(authorizedUIDs[0]);

void setup() {
  Serial.begin(9600);     // Start serial communication
  SPI.begin();            // Start SPI communication
  rfidReader.PCD_Init();  // Initialize RFID reader

  latchServo.attach(SERVO_PIN);

  // To unlock the door from inside by pressing the reset button on Arduino UNO
  // This section will rotate the servo, keep it open for a duration, then close it.
  latchServo.write(rotationAngle);
  delay(unlockDuration);
  latchServo.write(0); // Return to initial servo position (locked)

  Serial.println("Present your card to the reader...");
}

void loop() {
  // Check for a new card
  if (!rfidReader.PICC_IsNewCardPresent()) {
    return; // No new card, exit loop iteration
  }

  // Check if the card is readable
  if (!rfidReader.PICC_ReadCardSerial()) {
    return; // Card not readable, exit loop iteration
  }

  // Display the UID of the card
  Serial.print("UID tag: ");
  String cardUID = "";
  for (byte i = 0; i < rfidReader.uid.size; i++) {
    // Convert each byte of the UID to its hexadecimal string representation
    // If the hex value is less than 0x10 (i.e., a single digit), prepend a '0' for consistent formatting
    if (rfidReader.uid.uidByte[i] < 0x10) {
      cardUID += "0";
    }
    cardUID += String(rfidReader.uid.uidByte[i], HEX);
  }
  // Convert the entire UID string to lowercase for case-insensitive comparison
  cardUID.toLowerCase();
  Serial.println(cardUID);

  // Flag to track if access is granted
  boolean accessGranted = false;

  // Iterate through the list of authorized UIDs
  for (int i = 0; i < numAuthorizedUIDs; i++) {
    // Compare the scanned card's UID with each authorized UID in the array
    // Convert authorized UIDs to lowercase for consistent comparison
    if (cardUID == authorizedUIDs[i].toLowerCase()) {
      accessGranted = true; // Match found, set flag to true
      break; // Exit the loop as soon as a match is found
    }
  }

  // Act based on access status
  if (accessGranted) {
    Serial.println("Access granted");
    latchServo.write(rotationAngle); // Unlock the door
    delay(unlockDuration);           // Keep door unlocked for specified duration
    latchServo.write(0);             // Lock the door
  } else {
    Serial.println("Access denied");
    delay(1000); // Short delay for "Access denied" message visibility
  }

  // Halt PICC (Proximity Integrated Circuit Card) after authentication
  // This is important to allow the reader to detect new cards.
  rfidReader.PICC_HaltA();
  // Stop encryption on PCD (Proximity Coupling Device)
  rfidReader.PCD_StopCrypto1();
}