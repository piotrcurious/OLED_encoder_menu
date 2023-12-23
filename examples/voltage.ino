// Include the libraries for the encoder and the display
#include <Encoder.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define the pins for the encoder and the display
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
#define ENCODER_BUTTON_PIN 4
#define OLED_DC 5
#define OLED_CS 6
#define OLED_RESET 7

// Create the encoder and the display objects
Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

// Define the number of visualization functions
#define NUM_FUNCTIONS 3

// Define the names of the visualization functions
const char* function_names[NUM_FUNCTIONS] = {"Voltage Display", "Voltage Graph", "Voltage Control"};

// Define the current function index and the edit mode flag
int function_index = 0;
bool edit_mode = false;

// Define the previous encoder position and button state
long prev_encoder_pos = 0;
int prev_button_state = HIGH;

// Define the debounce time for the button
#define DEBOUNCE_TIME 50

// Define the last time the button state changed
unsigned long last_button_change = 0;

// Define the encoder sensitivity
#define ENCODER_SENSITIVITY 4

// Define the analog input pin for reading the voltage
#define VOLTAGE_PIN A0

// Define the digital output pin for controlling the output
#define OUTPUT_PIN 8

// Define the array to store the voltage values for the rolling graph display
#define GRAPH_SIZE 128
int voltage_values[GRAPH_SIZE];

// Define the variables to store the decimal range, the graph update speed, the graph scale, the voltage threshold, and the hysteresis
int decimal_range = 2; // The number of decimal places to display the voltage
int graph_speed = 100; // The delay in milliseconds between each graph update
int graph_scale = 5; // The scale factor to multiply the voltage values for the graph display
int voltage_threshold = 512; // The threshold value to compare the voltage with
int hysteresis = 10; // The hysteresis value to prevent output oscillation

// Define the constants to store the minimum and maximum values for these variables
#define DECIMAL_RANGE_MIN 0
#define DECIMAL_RANGE_MAX 3
#define GRAPH_SPEED_MIN 10
#define GRAPH_SPEED_MAX 1000
#define GRAPH_SCALE_MIN 1
#define GRAPH_SCALE_MAX 10
#define VOLTAGE_THRESHOLD_MIN 0
#define VOLTAGE_THRESHOLD_MAX 1023
#define HYSTERESIS_MIN 0
#define HYSTERESIS_MAX 100

// Define the last time the graph was updated
unsigned long last_graph_update = 0;

// Initialize the encoder, the display, and the output
void setup() {
  // Set the encoder button pin as input with pullup
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);

  // Set the output pin as output
  pinMode(OUTPUT_PIN, OUTPUT);

  // Initialize the display with the correct size
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Encoder UI Demo");
  display.display();
}

// Update the encoder, the display, and the output
void loop() {
  // Read the current encoder position and button state
  long encoder_pos = encoder.read();
  int button_state = digitalRead(ENCODER_BUTTON_PIN);

  // Check if the encoder position has changed
  if (encoder_pos != prev_encoder_pos) {
    // Calculate the difference in encoder position
    long encoder_diff = encoder_pos - prev_encoder_pos;

    // Divide the encoder difference by the encoder sensitivity
    encoder_diff /= ENCODER_SENSITIVITY;

    // Update the function index or the edit mode value based on the encoder direction
    if (edit_mode) {
      // Edit the current function value
      edit_function_value(function_index, encoder_diff);
    } else {
      // Change the current function index
      function_index += encoder_diff;

      // Wrap the function index within the range
      function_index = (function_index + NUM_FUNCTIONS) % NUM_FUNCTIONS;
    }

    // Update the previous encoder position
    prev_encoder_pos = encoder_pos;

    // Update the display
    update_display();
  }

  // Check if the button state has changed and the debounce time has passed
  if (button_state != prev_button_state && millis() - last_button_change > DEBOUNCE_TIME) {
    // Toggle the edit mode flag
    edit_mode = !edit_mode;

    // Update the previous button state and the last button change time
    prev_button_state = button_state;
    last_button_change = millis();

    // Update the display
    update_display();
  }

  // Check if the function index is 1 (voltage graph) and the graph update speed has passed
  if (function_index == 1 && millis() - last_graph_update > graph_speed) {
    // Update the graph display
    update_graph();

    // Update the last graph update time
    last_graph_update = millis();
  }

  // Check if the function index is 2 (voltage control) and the edit mode is off
  if (function_index == 2 && !edit_mode) {
    // Check the output state
    check_output();
  }
}

// Update the display with the current function name and value
void update_display() {
  // Clear the display
  display.clearDisplay();

  // Print the current function name
  display.setCursor(0, 0);
  display.print("Function: ");
  display.println(function_names[function_index]);

  // Print the current function value
  display.print("Value: ");
  display.println(get_function_value(function_index));

  // Print the edit mode indicator
  display.print("Edit mode: ");
  display.println(edit_mode ? "ON" : "OFF");

  // Show the display
  display.display();
}

// Get the current value of the given function index
int get_function_value(int index) {
  // Read the voltage from the analog input pin
  int voltage = analogRead(VOLTAGE_PIN);

  // Map the voltage to the decimal range
  int decimal_factor = pow(10, decimal_range);
  voltage = map(voltage, 0, 1023, 0, 5000 * decimal_factor);
  voltage = constrain(voltage, 0, 5000 * decimal_factor);

  // Return the voltage value
  return voltage;
}

// Edit the current value of the given function index by the given amount
void edit_function_value(int index, int amount) {
  // Change the value of the variable according to the function index and the encoder direction
  switch (index) {
    case 0: // Voltage display
      // Change the decimal range
      decimal_range += amount;

      // Constrain the decimal range within the limits
      decimal_range = constrain(decimal_range, DECIMAL_RANGE_MIN, DECIMAL_RANGE_MAX);
      break;
    case 1: // Voltage graph
      // Check if the edit mode is on
      if (edit_mode) {
        // Change the graph update speed
        graph_speed += amount * 10;

        // Constrain the graph update speed within the limits
        graph_speed = constrain(graph_speed, GRAPH_SPEED_MIN, GRAPH_SPEED_MAX);
      } else {
        // Change the graph scale
        graph_scale += amount;

        // Constrain the graph scale within the limits
        graph_scale = constrain(graph_scale, GRAPH_SCALE_MIN, GRAPH_SCALE_MAX);
      }
      break;
    case 2: // Voltage control
      // Check if the edit mode is on
      if (edit_mode) {
        // Change the voltage threshold
        voltage_threshold += amount * 10;

        // Constrain the voltage threshold within the limits
        voltage_threshold = constrain(voltage_threshold, VOLTAGE_THRESHOLD_MIN, VOLTAGE_THRESHOLD_MAX);
      } else {
        // Change the hysteresis
        hysteresis += amount;

        // Constrain the hysteresis within the limits
        hysteresis = constrain(hysteresis, HYSTERESIS_MIN, HYSTERESIS_MAX);
      }
      break;
    default:
      break;
  }
}

// Draw the rolling graph of the voltage on the display
void draw_graph() {
  // Read the voltage from the analog input pin
  int voltage = analogRead(VOLTAGE_PIN);

  // Scale the voltage value by the graph scale factor
  voltage *= graph_scale;

  // Constrain the voltage value within the display height
  voltage = constrain(voltage, 0, display.height() - 1);

  // Shift the voltage values array to the left by one
  for (int i = 0; i < GRAPH_SIZE - 1; i++) {
    voltage_values[i] = voltage_values[i + 1];
  }

  // Store the new voltage value at the end of the array
  voltage_values[GRAPH_SIZE - 1] = voltage;

  // Clear the display
  display.clearDisplay();

  // Draw the voltage values as a line graph
  for (int i = 0; i < GRAPH_SIZE - 1; i++) {
    display.drawLine(i, display.height() - voltage_values[i], i + 1, display.height() - voltage_values[i + 1], WHITE);
  }

  // Show the display
  display.display();
}

// Check the voltage against the threshold and hysteresis and set the digital output pin accordingly
void check_output() {
  // Read the voltage from the analog input pin
  int voltage = analogRead(VOLTAGE_PIN);

  // Read the current output state
  int output_state = digitalRead(OUTPUT_PIN);

  // Compare the voltage with the threshold and hysteresis
  if (voltage > voltage_threshold + hysteresis) {
    // Set the output state to high
    output_state = HIGH;
  } else if (voltage < voltage_threshold - hysteresis) {
    // Set the output state to low
    output_state = LOW;
  }

  // Write the output state to the output pin
  digitalWrite(OUTPUT_PIN, output_state);
}
