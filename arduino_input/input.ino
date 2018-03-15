#define DIAL_A_INT_PIN  2
#define DIAL_B_INT_PIN  3
#define DIAL_PIN_A    4
#define DIAL_PIN_B    5
#define JOY_X_PIN     A0
#define JOY_Y_PIN     A1
#define BUTTON_1_PIN  6
#define BUTTON_2_PIN  7

// Interface pins with the Raspberry Pi
#define DATA_READY_PIN 8
#define DATA_CLK_PIN 9
#define DATA_PIN 10

// Max length of data packet to be transfered
#define DATA_N_BITS 4

#define DIAL_LEFT -1
#define DIAL_RIGHT 1
#define DIAL_STACK_SIZE 32

volatile int dial_moves[DIAL_STACK_SIZE];
int dial_start_index = 0;
volatile int dial_end_index = 0;
int dial_move; // Amount by which the dial was moved

void setup() {
  Serial.begin(9600);

  pinMode(DIAL_PIN_A, INPUT);
  pinMode(DIAL_PIN_B, INPUT);
  pinMode(JOY_X_PIN, INPUT);
  pinMode(JOY_Y_PIN, INPUT);
  pinMode(BUTTON_1_PIN, INPUT);
  pinMode(BUTTON_2_PIN, INPUT);

  pinMode(DATA_READY_PIN, OUTPUT);
  pinMode(DATA_CLK_PIN, INPUT);
  pinMode(DATA_PIN, OUTPUT);
  
  // Setup interrupts
  pinMode(DIAL_A_INT_PIN, INPUT);
  pinMode(DIAL_B_INT_PIN, INPUT);
  //digitalWrite(DIAL_A_INT_PIN, HIGH);
  //digitalWrite(DIAL_B_INT_PIN, HIGH);
  attachInterrupt(digitalPinToInterrupt(DIAL_A_INT_PIN), dial_int, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DIAL_B_INT_PIN), dial_int, CHANGE);
}

void loop() {

  // 30 polls a second
  delay(33);

  dial_move = 0;
  noInterrupts();
  while(dial_start_index != dial_end_index) {
    dial_move += dial_moves[dial_start_index];
    ++dial_start_index;

    if(dial_start_index == DIAL_STACK_SIZE) {
      dial_start_index = 0;
    }
  }
  interrupts();

//  if(digitalRead(BUTTON_1_PIN)) {
//    Serial.println("Button 1");
//  }
//  if(digitalRead(BUTTON_2_PIN)) {
//    Serial.println("Button 2");
//  }
//  int joy_x = joystickRead(JOY_X_PIN);
//  int joy_y = joystickRead(JOY_Y_PIN);
//  if(abs(joy_x) > 80 || abs(joy_y) > 80) {
//    Serial.print(joy_x);
//    Serial.print(", ");
//    Serial.println(joy_y);
//  }
  int had_input = 0;
  if(dial_move != 0) {
    had_input = 1;
    //Serial.println(dial_move);
  }

  noInterrupts();
  if(had_input) reportInputEvent();
  interrupts();
}

int joystickRead(int pin) {
  return (int)((analogRead(pin) - 512) / 5.12);
}

void reportInputEvent() {

  // Prepare data to be transfered
  int data = 0x8 + dial_move; // Offset to support -8 to 7 rotations
  int data_index = 0;

  // Notify Raspberry Pi data is ready
  digitalWrite(DATA_READY_PIN, HIGH);

  // Continue looping checking for the rising edge of
  // the data clk line until all data has been transfered.
  int old_clk_read = digitalRead(DATA_CLK_PIN);
  while(data_index != DATA_N_BITS) {
    // Check for rising edge of clock line
    int new_clk_read = digitalRead(DATA_CLK_PIN);
    if(old_clk_read == 0 && new_clk_read == 1) {
      Serial.println(data);
      int data_write_value = (data & 0x1) ? HIGH : LOW;
      digitalWrite(DATA_PIN, data_write_value);
      data = data >> 1;
      data_index++;
    }

    old_clk_read = new_clk_read;
  }

  digitalWrite(DATA_READY_PIN, LOW);
}

//
// Setup the rotary dial input handler
//
int change_array[] = { 0, -1,  1,  0,
                       1,  0,  0, -1,
                      -1,  0,  0,  1,
                       0,  1, -1,  0};

volatile int old_state = 0xFF;
volatile int clw_ticks = 0;
void dial_int() {
  int dial = (digitalRead(DIAL_PIN_A) << 1) | digitalRead(DIAL_PIN_B);
  int state = ((old_state & 0x3) << 2) | dial;

  clw_ticks += change_array[state];

  if(clw_ticks > 3 || clw_ticks < -3) {
    if(clw_ticks < 0) {
      storeDialMove(DIAL_LEFT);
    }
    else {
      storeDialMove(DIAL_RIGHT);
    }
    clw_ticks = 0;
  }

  old_state = state;
}

void storeDialMove(int dir) {
  dial_moves[dial_end_index] = dir;
  dial_end_index += 1;

  if(dial_end_index == DIAL_STACK_SIZE) {
    dial_end_index = 0;
  }
}

