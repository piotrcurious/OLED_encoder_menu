// Include the libraries for the OLED display and the rotary encoder
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Encoder.h>

// Define the pins for the OLED display
#define OLED_MOSI  9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13

// Define the pins for the rotary encoder and the button
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 4
#define ENCODER_BUTTON_PIN 3

// Define the pin for the digital output
#define OUTPUT_PIN 5

// Create the OLED display object
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// Create the rotary encoder object
Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

// Define the number of function screens
#define NUM_SCREENS 3

// Define the variables for the function screens
int currentScreen = 0; // The current screen index
bool editMode = false; // The edit mode flag
long encoderPosition = 0; // The encoder position
long lastEncoderPosition = 0; // The last encoder position
long encoderMin = 0; // The minimum encoder value
long encoderMax = 0; // The maximum encoder value
int encoderStep = 0; // The encoder step size

// Define the variables for the voltage display screen
float voltage = 0; // The voltage value
int decimalRange = 2; // The decimal range selection
int decimalMin = 0; // The minimum decimal range
int decimalMax = 3; // The maximum decimal range
int decimalStep = 1; // The decimal step size

// Define the variables for the rolling graph screen
#define GRAPH_WIDTH 128 // The graph width
#define GRAPH_HEIGHT 32 // The graph height
#define GRAPH_X 0 // The graph x position
#define GRAPH_Y 32 // The graph y position
int graphData[GRAPH_WIDTH]; // The graph data array
int graphIndex = 0; // The graph index
int graphUpdateSpeed = 100; // The graph update speed in milliseconds
int graphUpdateMin = 10; // The minimum graph update speed
int graphUpdateMax = 1000; // The maximum graph update speed
int graphUpdateStep = 10; // The graph update speed step size
int graphScale = 5; // The graph scale factor
int graphScaleMin = 1; // The minimum graph scale factor
int graphScaleMax = 10; // The maximum graph scale factor
int graphScaleStep = 1; // The graph scale factor step size

// Define the variables for the voltage display with threshold screen
float threshold = 2.5; // The voltage threshold
float hysteresis = 0.1; // The voltage hysteresis
float thresholdMin = 0; // The minimum voltage threshold
float thresholdMax = 5; // The maximum voltage threshold
float thresholdStep = 0.1; // The voltage threshold step size
float hysteresisMin = 0; // The minimum voltage hysteresis
float hysteresisMax = 1; // The maximum voltage hysteresis
float hysteresisStep = 0.01; // The voltage hysteresis step size
bool outputState = false; // The output state

// Define the debounce parameters for the button
#define DEBOUNCE_DELAY 50 // The debounce delay in milliseconds
unsigned long lastDebounceTime = 0; // The last debounce time
int lastButtonState = HIGH; // The last button state

// Define the edit mode parameters
#define NUM_PARAMETERS 2 // The number of editable parameters per screen
#define BACK_ARROW -1 // The back arrow value
int currentParameter = 0; // The current parameter index

void setup() {
  // Initialize the OLED display
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Bing Arduino Code");
  display.display();
  delay(1000);

  // Initialize the encoder button pin
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);

  // Initialize the output pin
  pinMode(OUTPUT_PIN, OUTPUT);
  digitalWrite(OUTPUT_PIN, outputState);

  // Initialize the graph data array
  for (int i = 0; i < GRAPH_WIDTH; i++) {
    graphData[i] = 0;
  }
}

void loop() {
  // Read the voltage from the analog input A0
  voltage = analogRead(A0) * (5.0 / 1023.0);

  // Read the encoder position
  encoderPosition = encoder.read();

  // Read the button state
  int buttonState = digitalRead(ENCODER_BUTTON_PIN);

  // Check if the button state has changed
  if (buttonState != lastButtonState) {
    // Reset the debounce timer
    lastDebounceTime = millis();
  }

  // Check if the debounce time has elapsed
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    // Check if the button state has changed
    if (buttonState != lastButtonState) {
      // Update the button state
      lastButtonState = buttonState;

      // Check if the button is pressed
      if (buttonState == LOW) {
        // Check if the edit mode is on
        if (editMode) {
          // Check if the current parameter is the back arrow
          if (currentParameter == BACK_ARROW) {
            // Turn off the edit mode
            editMode = false;

            // Reset the encoder position
            encoder.write(0);
            encoderPosition = 0;
            lastEncoderPosition = 0;
          }
          else {
            // Toggle the parameter selection
            currentParameter = (currentParameter + 1) % NUM_PARAMETERS;
          }
        }
        else {
          // Turn on the edit mode
          editMode = true;

          // Set the current parameter to the back arrow
          currentParameter = BACK_ARROW;

          // Reset the encoder position
          encoder.write(0);
          encoderPosition = 0;
          lastEncoderPosition = 0;
        }
      }
    }
  }

  // Check if the encoder position has changed
  if (encoderPosition != lastEncoderPosition) {
    // Update the last encoder position
    lastEncoderPosition = encoderPosition;

    // Check if the edit mode is on
    if (editMode) {
      // Update the current screen parameters based on the encoder position and the current parameter
      updateScreenParameters();
    }
    else {
      // Update the current screen index based on the encoder position
      updateScreenIndex();
    }
  }

  // Update the output state based on the voltage and the threshold
  updateOutputState();

  // Display the current screen
  displayScreen();

  // Update the graph data
  updateGraphData();
}

// Update the current screen parameters based on the encoder position and the current parameter
void updateScreenParameters() {
  // Check which screen is active
  switch (currentScreen) {
    case 0: // Voltage display screen
      // Check which parameter is selected
      switch (currentParameter) {
        case 0: // Decimal range
          // Map the encoder position to the decimal range
          decimalRange = map(encoderPosition, encoderMin, encoderMax, decimalMin, decimalMax);

          // Constrain the decimal range to the valid range
          decimalRange = constrain(decimalRange, decimalMin, decimalMax);

          // Set the encoder step size to the decimal step size
          encoderStep = decimalStep;
          break;
        case 1: // Back arrow
          // Do nothing
          break;
      }
      break;
    case 1: // Rolling graph screen
      // Check which parameter is selected
      switch (currentParameter) {
        case 0: // Graph update speed
          // Map the encoder position to the graph update speed
          graphUpdateSpeed = map(encoderPosition, encoderMin, encoderMax, graphUpdateMin, graphUpdateMax);

          // Constrain the graph update speed to the valid range
          graphUpdateSpeed = constrain(graphUpdateSpeed, graphUpdateMin, graphUpdateMax);

          // Set the encoder step size to the graph update speed step size
          encoderStep = graphUpdateStep;
          break;
        case 1: // Graph scale factor
          // Map the encoder position to the graph scale factor
          graphScale = map(encoderPosition, encoderMin, encoderMax, graphScaleMin, graphScaleMax);

          // Constrain the graph scale factor to the valid range
          graphScale = constrain(graphScale, graphScaleMin, graphScaleMax);

          // Set the encoder step size to the graph scale factor step size
          encoderStep = graphScaleStep;
          break;
      }
      break;
    case 2: // Voltage display with threshold screen
      // Check which parameter is selected
      switch (currentParameter) {
        case 0: // Voltage threshold
          // Map the encoder position to the voltage threshold
          threshold = map(encoderPosition, encoderMin, encoderMax, thresholdMin, thresholdMax);

          // Constrain the voltage threshold to the valid range
          threshold = constrain(threshold, thresholdMin, thresholdMax);

          // Set the encoder step size to the voltage threshold step size
          encoderStep = thresholdStep;
          break;
        case 1: // Voltage hysteresis
          // Map the encoder position to the voltage hysteresis
          hysteresis = map(encoderPosition, encoderMin, encoderMax, hysteresisMin, hysteresisMax);

          // Constrain the voltage hysteresis to the valid range
          hysteresis = constrain(hysteresis, hysteresisMin, hysteresisMax);

          // Set the encoder step size to the voltage hysteresis step size
          encoderStep = hysteresisStep;
          break;
      }
      break;
  }
}

// Update the current screen index based on the encoder position
void updateScreenIndex() {
  // Check if the encoder position is positive or negative
  if (encoderPosition > 0) {
    // Increment the current screen index
    currentScreen++;

    // Wrap the current screen index to the valid range
    currentScreen = currentScreen % NUM_SCREENS;
  }
  else if (encoderPosition < 0) {
    // Decrement the current screen index
    currentScreen--;

    // Wrap the current screen index to the valid range
    currentScreen = (currentScreen + NUM_SCREENS) % NUM_SCREENS;
  }

  // Reset the encoder position
  encoder.write(0);
  encoderPosition = 0;
  lastEncoderPosition = 0;
}

// Update the output state based on the voltage and the threshold
void updateOutputState() {
  // Check if the voltage is above the threshold plus the hysteresis
  if (voltage > threshold + hysteresis) {
    // Set the output state to high
    outputState = true;
  }
  // Check if the voltage is below the threshold minus the hysteresis
  else if (voltage < threshold - hysteresis) {
    // Set the output state to low
    outputState = false;
  }

  // Write the output state to the output pin
  digitalWrite(OUTPUT_PIN, outputState);
}

// Display the current screen
void displayScreen() {
  // Clear the display
  display.clearDisplay();

  // Check which screen is active
  switch (currentScreen) {
    case 0: // Voltage display screen
      // Display the voltage value with the selected decimal range
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.print("Voltage: ");
      display.print(voltage, decimalRange);
      display.println(" V");

      // Display the edit mode indicator
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,24);
      display.print("Edit mode: ");
      display.println(editMode ? "ON" : "OFF");

      // Check if the edit mode is on
      if (editMode) {
        // Display the editable parameter
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,40);
        display.print("Decimal range: ");
        display.print(decimalRange);

        // Display the back arrow
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0,56);
        display.print("<");

        // Highlight the selected parameter or the back arrow
        display.drawRect(currentParameter == 0 ? 64 : 0, 40, currentParameter == 0 ? 64 : 16, 24, WHITE);
      }
      break;
    case 1: // Rolling graph screen
      // Display the graph data
      display.drawBitmap(GRAPH_X, GRAPH_Y, graphData, GRAPH_WIDTH, GRAPH_HEIGHT, WHITE);

      // Display the edit mode indicator
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.print("Edit mode: ");
      display.println(editMode ? "ON" : "OFF");

      // Check if the edit mode is on
      if (editMode) {
        // Display the editable parameters
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,8);
        display.print("Update speed: ");
        display.print(graphUpdateSpeed);
        display.println(" ms");
        display.setCursor(0,16);
        display.print("Scale factor: ");
        display.print(graphScale);

        // Display the back arrow
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0,24);
        display.print("<");

        // Highlight the selected parameter or the back arrow
        display.drawRect(currentParameter == 0 ? 64 : 0, 8, currentParameter == 0 ? 64 : 16, 16, WHITE);
      }
      break;
    case 2: // Voltage display with threshold screen
      // Display the voltage value
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.print("Voltage: ");
      display.print(voltage, decimalRange);
      display.println(" V");

      // Display the edit mode indicator
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,24);
      display.print("Edit mode: ");
      display.println(editMode ? "ON" : "OFF");

      // Check if the edit mode is on
      if (editMode) {
        // Display the editable parameters
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,40);
        display.print("Threshold: ");
        display.print(threshold, 2);
        display.println(" V");
        display.setCursor(0,48);
        display.print("Hysteresis: ");
        display.print(hysteresis, 2);
        display.println(" V");

        // Display the back arrow
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0,56);
        display.print("<");

        // Highlight the selected parameter or the back arrow
        display.drawRect(currentParameter == 0 ? 64 : 0, 40, currentParameter == 0 ? 64 : 16, 24, WHITE);
      }
      break;
  }

  // Update the display
  display.display();
}

// Update the graph data
void updateGraphData() {
  // Check if the graph update time has elapsed
  if (millis() - lastGraphUpdateTime > graphUpdateSpeed) {
    // Update the last graph update time
    lastGraphUpdateTime = millis();

    // Shift the graph data to the left
    for (int i = 0; i < GRAPH_WIDTH - 1; i++) {
      graphData[i] = graphData[i + 1];
    }

    // Map the voltage value to the graph height and scale it
    graphData[GRAPH_WIDTH - 1] = map(voltage * 1000, 0, 5000, 0, GRAPH_HEIGHT) / graphScale;

    // Constrain the graph data to the valid range
    graphData[GRAPH_WIDTH - 1] = constrain(graphData[GRAPH_WIDTH - 1], 0, GRAPH_HEIGHT);
  }
}
