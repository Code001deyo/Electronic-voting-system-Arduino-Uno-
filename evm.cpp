#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// Pin Definitions
#define BUTTON_UP 6
#define BUTTON_DOWN 7
#define BUTTON_ENTER 8
#define BUTTON_BACK 9
#define BUTTON_CANCEL 10
#define BUZZER_PIN 12

// Fingerprint Sensor Setup
SoftwareSerial mySerial(2, 3); // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Candidates
const char* candidates[] = {
  "Lydiah Wairimu",
  "Lawrence Nderu",
  "Isaiah Kindiki", 
 };

const int NUM_CANDIDATES = 3;
int candidateVotes[3] = {0, 0, 0};

// State Management
enum SystemState {
  MAIN_MENU,
  ENROLL_MODE,
  VOTE_MODE,
  RESULTS_MODE
};
SystemState currentState = MAIN_MENU;

// Menu Selection Tracking
int currentMenuSelection = 1; // 1 for Enroll, 2 for Vote, 3 for Results

// Voting Tracking
bool hasVoted[10] = {false}; // Track voting status for each fingerprint ID

// Unique Fingerprint Confidence Threshold
#define CONFIDENCE_THRESHOLD 50

void setup() {
  // Initialize Serial for debugging
  Serial.begin(9600);
  
  // Fingerprint Sensor Initialization
  Serial.println("Initializing Fingerprint Sensor");
  finger.begin(57600);
  
  // Check sensor status
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor found!");
  } else {
    Serial.println("Fingerprint sensor not found :(");
  }
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize Buttons
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_ENTER, INPUT_PULLUP);
  pinMode(BUTTON_BACK, INPUT_PULLUP);
  pinMode(BUTTON_CANCEL, INPUT_PULLUP);
  
  // Initialize Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Clear all stored fingerprints
  clearAllFingerprints();
  
  // Display Startup Screen
  displayStartupScreen();
}

// New function to clear all stored fingerprints
void clearAllFingerprints() {
  Serial.println("Clearing all stored fingerprints...");
  for (int i = 0; i < 10; i++) {
    if (finger.deleteModel(i) == FINGERPRINT_OK) {
      Serial.print("Deleted fingerprint ID: ");
      Serial.println(i);
    }
  }
  Serial.println("Fingerprint clearing complete.");
}

void loop() {
  switch(currentState) {
    case MAIN_MENU:
      handleMainMenu();
      break;
    case ENROLL_MODE:
      handleEnrollMode();
      break;
    case VOTE_MODE:
      handleVoteMode();
      break;
    case RESULTS_MODE:
      handleResultsMode();
      break;
  }
  
  delay(200); // Debounce delay
}

void displayStartupScreen() {
  lcd.clear();   
  lcd.print("Voting Machine");   
  lcd.setCursor(0, 1);   
  lcd.print("by Finger Print");   
  delay(2000);   
  lcd.clear();   
  lcd.print("Group Assignment");   
  lcd.setCursor(0, 1); 
  lcd.print("BSD 3205");  
  delay(2000);   
  lcd.clear();  
  lcd.print("Group Members");  
  lcd.setCursor(0, 1);  
  lcd.print("Harrison Iduyo"); 
  delay(2000);  
  lcd.clear(); 
  lcd.print("Lydiah kiboi");  
  lcd.setCursor(0, 1);  
  lcd.print("Marlon Amunga");  
  delay(2000);
  lcd.clear();   
  lcd.print("Sam Muasya");  
  lcd.setCursor(0, 1);   
  lcd.print("Marion Oketcha"); 
  delay(2000);  
  lcd.clear();  
  lcd.print("Finlay Joash");  
  lcd.setCursor(0, 1);  
  lcd.print("Prototype");  
  delay(2000); 
  displayMainMenu();
}

void displayMainMenu() {
  lcd.clear();
  updateMainMenuDisplay();
}

void handleMainMenu() {
  if (digitalRead(BUTTON_UP) == LOW) {
    tone(BUZZER_PIN, 1000, 50);
    // Cycle menu selection
    currentMenuSelection = (currentMenuSelection > 1) ? currentMenuSelection - 1 : 3;
    updateMainMenuDisplay();
    delay(300);
  }
  
  if (digitalRead(BUTTON_DOWN) == LOW) {
    tone(BUZZER_PIN, 1000, 50);
    // Cycle menu selection
    currentMenuSelection = (currentMenuSelection < 3) ? currentMenuSelection + 1 : 1;
    updateMainMenuDisplay();
    delay(300);
  }
  
  if (digitalRead(BUTTON_ENTER) == LOW) {
    tone(BUZZER_PIN, 1000, 50);
    delay(300); // Debounce
    
    // Determine which option was selected
    switch(currentMenuSelection) {
      case 1: // Enroll
        currentState = ENROLL_MODE;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enroll Mode");
        lcd.setCursor(0, 1);
        lcd.print("Place Finger");
        break;
      
      case 2: // Vote
        currentState = VOTE_MODE;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Vote Mode");
        lcd.setCursor(0, 1);
        lcd.print("Place Finger");
        break;
      
      case 3: // Results
        currentState = RESULTS_MODE;
        break;
    }
  }
}

void updateMainMenuDisplay() {
  switch(currentMenuSelection) {
    case 1:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Select one:");
      lcd.setCursor(0, 1);
      lcd.print("1. Enroll ....");
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Select one:");
      lcd.setCursor(0, 1);
      lcd.print("2. Vote ....");
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Select one:");
      lcd.setCursor(0, 1);
      lcd.print("3. Results ....");
      break;
  }
}

void handleEnrollMode() {
  // Check if there's space for enrollment
  int enrollID = findAvailableID();
  
  if (enrollID == -1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Memory Full!");
    lcd.setCursor(0, 1);
    lcd.print("Cannot Enroll");
    delay(2000);
    currentState = MAIN_MENU;
    displayMainMenu();
    return;
  }
  
  // Debug: Print the available enrollment ID
  Serial.print("Available Enrollment ID: ");
  Serial.println(enrollID);
  
  // First Fingerprint Capture
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("First Capture");
  lcd.setCursor(0, 1);
  lcd.print("Place Finger");
  
  // Wait for initial fingerprint capture
  uint8_t p = FINGERPRINT_NOFINGER;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    
    // Allow cancellation
    if (digitalRead(BUTTON_CANCEL) == LOW) {
      tone(BUZZER_PIN, 1000, 50);
      currentState = MAIN_MENU;
      displayMainMenu();
      return;
    }
  }
  
  // Convert first image to template
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("First Capture");
    lcd.setCursor(0, 1);
    lcd.print("Failed!");
    delay(2000);
    currentState = MAIN_MENU;
    displayMainMenu();
    return;
  }
  
  // Debugging: Print the status of existing fingerprint check
  int existingID = finger.fingerFastSearch();
  Serial.print("Existing Fingerprint Search Result: ");
  Serial.println(existingID);
  
  // More robust check for existing fingerprints
  if (existingID != -1) {
    // Additional verification
    if (finger.loadModel(existingID) == FINGERPRINT_OK) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Fingerprint");
      lcd.setCursor(0, 1);
      lcd.print("Already Exists!");
      tone(BUZZER_PIN, 500, 500);
      delay(2000);
      currentState = MAIN_MENU;
      displayMainMenu();
      return;
    }
  }
  
  // Remove finger prompt
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Remove Finger");
  delay(2000);
  
  // Wait for finger removal
  while (finger.getImage() != FINGERPRINT_NOFINGER) {
    delay(100);
  }
  
  // Second Fingerprint Capture
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Second Capture");
  lcd.setCursor(0, 1);
  lcd.print("Place Same Finger");
  
  // Wait for second fingerprint capture
  p = FINGERPRINT_NOFINGER;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    
    // Allow cancellation
    if (digitalRead(BUTTON_CANCEL) == LOW) {
      tone(BUZZER_PIN, 1000, 50);
      currentState = MAIN_MENU;
      displayMainMenu();
      return;
    }
  }
  
  // Convert second image to template
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Second Capture");
    lcd.setCursor(0, 1);
    lcd.print("Failed!");
    delay(2000);
    currentState = MAIN_MENU;
    displayMainMenu();
    return;
  }
  
  // Create model
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    // Store the model
    p = finger.storeModel(enrollID);
    if (p == FINGERPRINT_OK) {
      // Success messages
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enrollment");
      lcd.setCursor(0, 1);
      lcd.print("Successful!");
      tone(BUZZER_PIN, 2000, 500);
      delay(2000);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Fingerprint Added");
      lcd.setCursor(0, 1);
      lcd.print("ID: " + String(enrollID));
      tone(BUZZER_PIN, 1500, 300);
      delay(2000);
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Store Model");
      lcd.setCursor(0, 1);
      lcd.print("Failed!");
      tone(BUZZER_PIN, 500, 500);
      delay(2000);
    }
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fingerprint");
    lcd.setCursor(0, 1);
    lcd.print("Not Matched!");
    tone(BUZZER_PIN, 500, 500);
    delay(2000);
  }
  
  currentState = MAIN_MENU;
  displayMainMenu();
}

bool captureFingerprint() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place Finger");
  
  while (finger.getImage() != FINGERPRINT_OK) {
    if (digitalRead(BUTTON_CANCEL) == LOW) {
      currentState = MAIN_MENU;
      displayMainMenu();
      return false;
    }
  }
  
  // Convert image to template
  if (finger.image2Tz() != FINGERPRINT_OK) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Image Conversion");
    lcd.setCursor(0, 1);
    lcd.print("Failed!");
    delay(2000);
    return false;
  }
  
  return true;
}

void handleVoteMode() {
  // Authenticate voter
  int fingerprintID = authenticateVoter();
  
  if (fingerprintID != -1) {
    // Check if this fingerprint has already voted
    if (hasVoted[fingerprintID]) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Already Voted!");
      lcd.setCursor(0, 1);
      lcd.print("One Vote Only");
      tone(BUZZER_PIN, 500, 500);
      delay(2000);
      currentState = MAIN_MENU;
      displayMainMenu();
      return;
    }
    
    // Voter is authenticated and hasn't voted before
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Select Candidate:");
    
    int selectedCandidate = 0;
    lcd.setCursor(0, 1);
    lcd.print(candidates[selectedCandidate]);
    
    while (true) {
      if (digitalRead(BUTTON_UP) == LOW) {
        tone(BUZZER_PIN, 1000, 50);
        selectedCandidate = (selectedCandidate - 1 + NUM_CANDIDATES) % NUM_CANDIDATES;
        lcd.setCursor(0, 1);
        lcd.print("                "); // Clear previous candidate
        lcd.setCursor(0, 1);
        lcd.print(candidates[selectedCandidate]);
        delay(300);
      }
      
      if (digitalRead(BUTTON_DOWN) == LOW) {
        tone(BUZZER_PIN, 1000, 50);
        selectedCandidate = (selectedCandidate + 1) % NUM_CANDIDATES;
        lcd.setCursor(0, 1);
        lcd.print("                "); // Clear previous candidate
        lcd.setCursor(0, 1);
        lcd.print(candidates[selectedCandidate]);
        delay(300);
      }
      
      if (digitalRead(BUTTON_ENTER) == LOW) {
        tone(BUZZER_PIN, 1000, 50);
        candidateVotes[selectedCandidate]++;
        
        // Mark this fingerprint as having voted
        hasVoted[fingerprintID] = true;
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Vote Recorded!");
        delay(1500);
        break;
      }
      
      if (digitalRead(BUTTON_BACK) == LOW || digitalRead(BUTTON_CANCEL) == LOW) {
        tone(BUZZER_PIN, 1000, 50);
        break;
      }
    }
  } else {
    // Not an enrolled voter
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Not Registered");
    lcd.setCursor(0, 1);
    lcd.print("Voter");
    delay(1500);
  }
  
  currentState = MAIN_MENU;
  displayMainMenu();
}

int authenticateVoter() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place Finger");
  
  while (true) {
    if (finger.getImage() == FINGERPRINT_OK) {
      if (finger.image2Tz() == FINGERPRINT_OK) {
        int fingerprintID = finger.fingerFastSearch();
        if (fingerprintID != -1) {
          return fingerprintID;
        }
      }
    }
    
    if (digitalRead(BUTTON_BACK) == LOW || digitalRead(BUTTON_CANCEL) == LOW) {
      tone(BUZZER_PIN, 1000, 50);
      return -1;
    }
  }
}

void handleResultsMode() {
  // Scroll through results
  int currentResult = 0;
  
  while (true) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(candidates[currentResult]);
    lcd.setCursor(0, 1);
    lcd.print("Votes: ");
    lcd.print(candidateVotes[currentResult]);
    
    if (digitalRead(BUTTON_UP) == LOW) {
      tone(BUZZER_PIN, 1000, 50);
      currentResult = (currentResult - 1 + NUM_CANDIDATES) % NUM_CANDIDATES;
      delay(300);
    }
    
    if (digitalRead(BUTTON_DOWN) == LOW) {
      tone(BUZZER_PIN, 1000, 50);
      currentResult = (currentResult + 1) % NUM_CANDIDATES;
      delay(300);
    }
    
    if (digitalRead(BUTTON_ENTER) == LOW) {
      tone(BUZZER_PIN, 1000, 50);
      delay(300);
    }
    
    if (digitalRead(BUTTON_BACK) == LOW || digitalRead(BUTTON_CANCEL) == LOW) {
      tone(BUZZER_PIN, 1000, 50);
      currentState = MAIN_MENU;
      displayMainMenu();
      return;
    }
  }
}

int findAvailableID() {
  for (int i = 0; i < 10; i++) {
    // Debug: Check the status of each model slot
    uint8_t status = finger.loadModel(i);
    Serial.print("Checking ID ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(status);
    
    if (status != FINGERPRINT_OK) {
      return i;
    }
  }
  return -1; // No available ID
}
