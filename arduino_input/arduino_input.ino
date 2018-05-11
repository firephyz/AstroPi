// How long we wait in milliseconds between each transmit attempt.
#define TRANSMIT_CYCLE_TIME 100

#define DIAL_PIN_A    2
#define DIAL_PIN_B    3
#define BUTTON_PIN_1  5
#define BUTTON_PIN_2  4

#define JOYSTICK_PIN_X A0
#define JOYSTICK_PIN_Y A1

#define DATA_READY_PIN 8
#define DATA_ACK_PIN 9
#define DATA_OUT_PIN 10
#define DATA_CLK_PIN 11

typedef struct {
  int8_t scroll;
  int8_t joyx;
  int8_t joyy;
  uint8_t buttons;
} InputPacket;

volatile int dial_move = 0;

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_PIN_1, INPUT);
  pinMode(BUTTON_PIN_2, INPUT);

  pinMode(DATA_READY_PIN, OUTPUT);
  pinMode(DATA_ACK_PIN, INPUT);
  pinMode(DATA_OUT_PIN, OUTPUT);
  pinMode(DATA_CLK_PIN, INPUT);
  
  pinMode(DIAL_PIN_A, INPUT);
  pinMode(DIAL_PIN_B, INPUT);// put your setup code here, to run once:
  attachInterrupt(digitalPinToInterrupt(DIAL_PIN_A), dial_int, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DIAL_PIN_B), dial_int, CHANGE);
}

long int start_time;
void loop() {
  static InputPacket input;

  start_time = millis();
  store_dial_input(&input);
  read_buttons(&input);
  read_joystick(&input);

  if(transmit_data(&input)) {
    input = (InputPacket){0, 0, 0, 0};
  }

  // After transmitting, wait for the remainder of the TRANSMIT_CYCLE_TIME
  while(millis() - start_time < TRANSMIT_CYCLE_TIME);
}

int transmit_data(InputPacket * input) {

  // Wait for the pi to acknowledge us
  digitalWrite(DATA_READY_PIN, HIGH);
  while(!digitalRead(DATA_ACK_PIN)) {
    if(millis() - start_time > TRANSMIT_CYCLE_TIME) return 0;
  }
  digitalWrite(DATA_READY_PIN, LOW);

  uint8_t * data = (uint8_t *)input;
  uint8_t mask = 0x1;
  for(int i = 0; i < sizeof(InputPacket) * 8; ++i) {
    digitalWrite(DATA_OUT_PIN, LOW);

    // Wait for the falling edge of the clk to put data on
    while(digitalRead(DATA_CLK_PIN) == 1) {
      // If Arduino stops acknowledging us, stop transmitting
      if(digitalRead(DATA_ACK_PIN) == 0) {
        return 0;
      }
    }

    // Output the data
    int index = i / 8;
    if(data[index] & mask) {
      digitalWrite(DATA_OUT_PIN, HIGH);
    }
    else {
      digitalWrite(DATA_OUT_PIN, LOW);
    }
    mask = mask << 1;
    if(mask == 0) mask = 0x1;

    // Wait for rising edge to move on
    while(digitalRead(DATA_CLK_PIN) == 0) {
      // If Arduino stops acknowledging us, stop transmitting
      if(digitalRead(DATA_ACK_PIN) == 0) {
        if(i == sizeof(InputPacket) * 8 - 1) {
          return 1;
        }
        return 0;
      }
    }
  }

  return 1;
}

void read_buttons(InputPacket * input) {
  input->buttons = (digitalRead(BUTTON_PIN_1) << 1) | digitalRead(BUTTON_PIN_2);
}

void read_joystick(InputPacket * input) {
  input->joyx = (int8_t)((analogRead(JOYSTICK_PIN_X) - 512) / 5.12);
  input->joyy = (int8_t)((analogRead(JOYSTICK_PIN_Y) - 512) / 5.12);
}

void store_dial_input(InputPacket * input) {
  noInterrupts();
  // Add the dial_move so we don't lose turns if the Arduino doesn't acknowledge first time around.
  input->scroll += dial_move;
  dial_move = 0;
  interrupts();
}

//
// Setup the rotary dial input handler
//

void dial_int() {
  volatile static int old_state = 0xFF;
  volatile static int clw_ticks = 0;
  static int change_array[] = { 0, -1,  1,  0,
                                1,  0,  0, -1,
                               -1,  0,  0,  1,
                                0,  1, -1,  0};
  
  int dial = (digitalRead(DIAL_PIN_A) << 1) | digitalRead(DIAL_PIN_B);
  int state = ((old_state & 0x3) << 2) | dial;

  clw_ticks += change_array[state];

  if(clw_ticks > 3 || clw_ticks < -3) {
    if(clw_ticks < 0) {
      --dial_move;
    }
    else {
      ++dial_move;
    }
    clw_ticks = 0;
  }

  old_state = state;
}

