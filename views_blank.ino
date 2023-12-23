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
const char* function_names[NUM_FUNCTIONS] = {"Function 1", "Function 2", "Function 3"};

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

// Initialize the encoder and the display
void setup() {
  // Set the encoder button pin as input with pullup
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);

  // Initialize the display with the correct size
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Encoder UI Demo");
  display.display();
}

// Update the encoder and the display
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
  // TODO: Implement this function according to your visualization functions
  // For example, you can use global variables, arrays, or EEPROM to store the function values
  // For simplicity, this example returns a fixed value for each function index
  switch (index) {
    case 0:
      return 10;
    case 1:
      return 20;
    case 2:
      return 30;
    default:
      return 0;
  }
}

// Edit the current value of the given function index by the given amount
void edit_function_value(int index, int amount) {
  // TODO: Implement this function according to your visualization functions
  // For example, you can use global variables, arrays, or EEPROM to store the function values
  // For simplicity, this example does nothing
}
