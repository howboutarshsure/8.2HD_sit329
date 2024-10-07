// Definitions for pins and global variables
const int buttonPin = 2;        // Pin for the R16503 pushbutton
const int ledPins[] = {3, 4, 5, 6, 7, 8}; // Pins for the LEDs
const int numLEDs = 6;          // Total number of LEDs in the system
int score = 0;                  // Variable to keep track of the player's score
unsigned long startTime;        // Variable to store the game's start time
unsigned long ledInterval;      // Time interval for LED movement
bool canScore = false;          // Boolean flag for score control

void setup() {
  Serial.begin(9600);
  while(!Serial); // Wait for Serial to initialize

  // Configure the LED pins as outputs
  for (int i = 0; i < numLEDs; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  // Setup button as an input with an internal pull-up resistor
  pinMode(buttonPin, INPUT_PULLUP); // Button is HIGH when not pressed

  // Prompt the user to select a game mode
  Serial.println("Setup complete. Select Game Mode...");
  mainMenu(); // Enter the main menu
}

void loop() {
  // Main logic is handled in other functions, no action needed in loop
}

// Function to display the main menu and choose game mode
void mainMenu() {
  Serial.println("Select Game Mode:");
  Serial.println("0: Catch the Running Lights");
  Serial.println("1: Identify the LED");
  
  while (true) {
    if (Serial.available() > 0) {
      char choice = Serial.read(); // Read user input
      if (choice == '0') {
        configureGame(); // Configure settings before starting
        playGameMode0(); // Start Game Mode 0
      } else if (choice == '1') {
        configureGame(); // Configure settings before starting
        playGameMode1(); // Start Game Mode 1
      }
    }
  }
}

// Function to configure game settings like speed
void configureGame() {
  Serial.println("Select speed (0: Slow, 1: Medium, 2: Fast):");
  while (Serial.available() == 0) {} // Wait for input
  
  char speedChoice = Serial.read(); // Read speed choice
  switch (speedChoice) {
    case '0': ledInterval = 1000; break; // Slow speed
    case '1': ledInterval = 500; break;  // Medium speed
    case '2': ledInterval = 250; break;  // Fast speed
    default: ledInterval = 500; break;   // Default to Medium speed
  }
  
  Serial.println("Game starting...");
  score = 0; // Reset score
  startTime = millis(); // Capture game start time
}

// Game Mode 0: "Catch the Running Lights"
void playGameMode0() {
  static int currentLED = 0;
  static bool direction = true; // To track movement direction
  static unsigned long lastChangeTime = 0;
  static unsigned long lastButtonPressTime = 0; // Debouncing for button presses

  // Run game until the player reaches 20 points
  while (score < 20) {
    // Control LED movement
    if (millis() - lastChangeTime >= ledInterval) {
      digitalWrite(ledPins[currentLED], LOW); // Turn off current LED

      // Update LED position
      currentLED = direction ? currentLED + 1 : currentLED - 1;

      // Reverse direction at the ends
      if (currentLED >= numLEDs - 1 || currentLED <= 0) {
        direction = !direction;
        canScore = true; // Allow scoring only at the ends
      } else {
        canScore = false;
      }

      digitalWrite(ledPins[currentLED], HIGH); // Turn on new LED
      lastChangeTime = millis(); // Reset the timer for movement
    }

    // Check if the player pressed the button
    if (digitalRead(buttonPin) == LOW) {
      if (millis() - lastButtonPressTime > 200) { // Debounce button press
        lastButtonPressTime = millis(); // Update last press time

        // Check if button press is correct
        if (currentLED == 0 || currentLED == numLEDs - 1) {
          score += 2; // Correct press
        } else {
          score -= 1; // Incorrect press
        }

        // Output current score
        Serial.print("Score: ");
        Serial.println(score);
      }
    }
  }

  endGame(); // End game once score reaches 20
}

// Game Mode 1: "Identify the LED"
void playGameMode1() {
  static unsigned long lastButtonPressTime = 0; // For debounce
  static bool buttonPressed = false; // Track button state
  int targetLED; // Target LED number
  unsigned long targetDuration; // How long the LED stays on
  unsigned long ledOnTime; // Time the LED was turned on

  // Play until score reaches 20
  while (score < 20) {
    targetLED = random(0, numLEDs); // Randomly choose a LED
    targetDuration = random(2000, 4000); // Random duration
    digitalWrite(ledPins[targetLED], HIGH); // Turn on the LED
    ledOnTime = millis(); // Record LED activation time

    Serial.print("Identify LED: ");
    Serial.println(targetLED + 1); // Display target LED number
    delay(targetDuration); // Keep the LED on for this duration
    digitalWrite(ledPins[targetLED], LOW); // Turn off the LED

    // Give the player time to respond
    unsigned long buttonPressDeadline = millis() + 1000; // 1 second to press
    int requiredPresses = targetLED + 1; // How many times to press

    // Wait for the player to press the button the required times
    for (int i = 0; i < requiredPresses; i++) {
      bool pressed = false; // Track if the button was pressed

      while (millis() < buttonPressDeadline) {
        int buttonState = digitalRead(buttonPin);

        if (buttonState == LOW && !buttonPressed) {
          if (millis() - lastButtonPressTime > 200) { // Debounce
            lastButtonPressTime = millis(); 
            score += 2; // Correct press, update score
            Serial.print("Score after press: ");
            Serial.println(score);
            pressed = true; // Mark button press
            buttonPressed = true; 
            break;
          }
        } else if (buttonState == HIGH) {
          buttonPressed = false; 
        }
      }

      // Wait until button is released
      if (pressed) {
        while (digitalRead(buttonPin) == LOW) {} 
      }
    }

    // Wait for additional presses
    unsigned long waitUntil = millis() + 1000; 
    while (millis() < waitUntil) {
      if (digitalRead(buttonPin) == LOW) {
        if (millis() - lastButtonPressTime > 200) {
          lastButtonPressTime = millis();
          score -= 1; // Penalize incorrect press
          Serial.print("Score after additional press: ");
          Serial.println(score);
        }
      }
    }
  }

  endGame(); // End game when score reaches 20
}

// Function to end the game and display time
void endGame() {
  unsigned long endTime = millis();
  Serial.print("Game Over! Time taken: ");
  Serial.print((endTime - startTime) / 1000);
  Serial.println(" seconds.");

  // Ask the player if they want to play again or quit
  Serial.println("Press 'Y' to play again or 'N' to quit.");
  
  // Wait for player response
  while (true) {
    if (Serial.available() > 0) {
      char choice = Serial.read();
      if (choice == 'Y' || choice == 'y') {
        mainMenu(); // Restart the game
      } else if (choice == 'N' || choice == 'n') {
        Serial.println("Thank you for playing! Exiting...");
        while (true); // End the program
      }
    }
  }
}
