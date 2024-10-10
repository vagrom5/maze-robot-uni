#include <Arduino.h>

// Define the motor control pins
#define MOTOR_FORWARD 5
#define MOTOR_BACKWARD 6
#define MOTOR_LEFT 7
#define MOTOR_RIGHT 8

// UART settings
#define UART_BAUD_RATE 9600

// Constants for PWM range
#define PWM_MIN 0
#define PWM_MAX 255

// GPIO input pins
#define GPIO_FORWARD 9
#define GPIO_BACKWARD 10
#define GPIO_LEFT 11
#define GPIO_RIGHT 12

void setup() {
  pinMode(MOTOR_FORWARD, OUTPUT);
  pinMode(MOTOR_BACKWARD, OUTPUT);
  pinMode(MOTOR_LEFT, OUTPUT);
  pinMode(MOTOR_RIGHT, OUTPUT);

  pinMode(GPIO_FORWARD, INPUT);
  pinMode(GPIO_BACKWARD, INPUT);
  pinMode(GPIO_LEFT, INPUT);
  pinMode(GPIO_RIGHT, INPUT);

  Serial.begin(UART_BAUD_RATE);
}

void handleUARTCommand(String command) {
  // Parse the joystick axis values from the command
  int axis0, axis1;
  if (sscanf(command.cSTR(), "%d, %d", &axis0, &axis1) != 2) {
    Serial.print("Invalid command format");
    return;
  }

  // Map joystick values to PWM signals
  int pwm_left = map(axis0, -32768, 32767, PWM_MIN, PWM_MAX);
  int pwm_right = map(axis0, -32768, 32767, PWM_MIN, PWM_MAX);
  int pwm_forward = map(axis1, -32768, 32767, PWM_MIN, PWM_MAX);
  int pwm_backward = map(axis1, -32768, 32767, PWM_MIN, PWM_MAX);

  // Ensure the PWM values are within the valid range
  pwm_left = constrain(pwm_left, PWM_MIN, PWM_MAX);
  pwm_right = constrain(pwm_right, PWM_MIN, PWM_MAX);
  pwm_forward = constrain(pwm_forward, PWM_MIN, PWM_MAX);
  pwm_backward = constrain(pwm_backward, PWM_MIN, PWM_MAX);

  // Set the PWM values for the motors
  analogWrite(MOTOR_LEFT, pwm_left);
  analogWrite(MOTOR_RIGHT, pwm_right);
  analogWrite(MOTOR_FORWARD, pwm_forward);
  analogWrite(MOTOR_BACKWARD, pwm_backward);

  // Log the current state
  Serial.print("Axis0: ");
  Serial.print(axis0);
  Serial.print(", Axis1: ");
  Serial.print(axis1);
  Serial.print(", PWM Left: ");
  Serial.print(pwm_left);
  Serial.print(", PWM Right: ");
  Serial.print(pwm_right);
  Serial.print(", PWM Forward: ");
  Serial.print(pwm_forward);
  Serial.print(", PWM Backward: ");
  Serial.println(pwm_backward, '\n');
}

void handleGPIOCommand() {
  int gpio_forward = digitalRead(GPIO_FORWARD);
  int gpio_backward = digitalRead(GPIO_BACKWARD);
  int gpio_left = digitalRead(GPIO_LEFT);
  int gpio_right = digitalRead(GPIO_RIGHT);

  digitalWrite(MOTOR_FORWARD, gpio_forward);
  digitalWrite(MOTOR_BACKWARD, gpio_backward);
  digitalWrite(MOTOR_LEFT, gpio_left);
  digitalWrite(MOTOR_RIGHT, gpio_right);

  Serial.print("GPIO: Forward: ");
  Serial.print(gpio_forward);
  Serial.print(", backward: ");
  Serial.print(gpio_backward);
;
  Serial.print(", left: ");
  Serial.print(gpio_left);
  Serial.print(", right: ");
  Serial.println(gpio_right, '\n');
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    handleUARTCommand(command);
    //handleGPIOCommand();
  }
}