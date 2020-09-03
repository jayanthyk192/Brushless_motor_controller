/*
 * MIT License

Copyright (c) 2020 jayanthyk192

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

/* Tested with Arduino Uno.
 * 
 * This runs the motor in full voltage and does not have a PWM voltage control!!!
 * 
 * Intended to be used with 12V hard drive motors. DO-NOT use this code/circuit 
 * with an RC Brushless motor. You'll burn the motor and the circuit. 
 * Code/circuit to run RC Brushless motors will be added soon!
 * 
 * Make sure you power all the ciruits with an SMPS which shuts down when there is
 * a current surge. DO-NOT run this with a LiPo battery. If there are shot-circuits, 
 * you'll end up with a lot of damage.
 * 
 * DO-NOT power the mosfets with voltages below 10V!
 * 
 * DO_NOT run the motor without the big capacitor shown in the schematic, 
 * you'll kill the mosfets very soon!
 */

//Set the Pins on the Arduino that control the N-Mosfets
#define NMOS_A  4
#define NMOS_B  5
#define NMOS_C  6

//Set the Pins on the Arduino that control the P-Mosfets
#define PMOS_A  11
#define PMOS_B  12
#define PMOS_C  13

//Set the Pins on the Arduino that read the Back EMF signals.
#define BEMF_A  A0
#define BEMF_B  A1
#define BEMF_C  A2

#define Ph_A  0
#define Ph_B  1
#define Ph_C  2

int disableMosfets = 1; // Set this to 1 to disable all the mosfets

void setup() {
  //These pins are used to control the NMOS transistors
  pinMode(NMOS_A, OUTPUT);           // set pin to output
  pinMode(NMOS_B, OUTPUT);           // set pin to output
  pinMode(NMOS_C, OUTPUT);           // set pin to output

  //These pins are used to control the PMOS transistors
  pinMode(PMOS_A, OUTPUT);           // set pin to output
  pinMode(PMOS_B, OUTPUT);           // set pin to output
  pinMode(PMOS_C, OUTPUT);           // set pin to output

  //These pins are used to read the Back EMF signals
  pinMode(BEMF_A, INPUT);           // set pin to input
  pinMode(BEMF_B, INPUT);           // set pin to input
  pinMode(BEMF_C, INPUT);           // set pin to input

  //Set the gates of all the PMOS to high (PMOS will be switched off)
  digitalWrite(PMOS_A, HIGH);
  digitalWrite(PMOS_B, HIGH);
  digitalWrite(PMOS_C, HIGH);

  //Set the gates of all the PMOS to high (PMOS will be switched off)
  digitalWrite(NMOS_A, LOW);
  digitalWrite(NMOS_B, LOW);
  digitalWrite(NMOS_C, LOW);
  
  //Start serial interface at 115200 baud.
  Serial.begin(115200);
}

void myMicroDelay(long del) {
  /* The delayMicroseconds can't be used beyond 160000 (16ms).
     Hence split the delays to multiples of 16000.
  */
  for (int i = 0; i < del / 16000; i++) {
    delayMicroseconds(16000);
  }
  delayMicroseconds(del % 16000);
}

void setPhasesPWM(int Ph_high, int Ph_low)
{
  //Switch off all the transistors.
  digitalWrite(PMOS_A, HIGH);
  digitalWrite(PMOS_B, HIGH);
  digitalWrite(PMOS_C, HIGH);

  digitalWrite(NMOS_A, LOW);
  digitalWrite(NMOS_B, LOW);
  digitalWrite(NMOS_C, LOW);

  if (!disableMosfets) {
    digitalWrite(Ph_low  + 4, HIGH); //Switch ON the required NMOS    
    digitalWrite(Ph_high + 11, LOW); //Switch ON the required PMOS
  }
}

void waitBEMF(int BEMF_n) {
  //Wait while the Back EMF changes from 0 to 1.
  while (digitalRead(BEMF_n));
}

void start_motor() {
  long del = 80000; //Set this depending on the motor.
  while ((del -= 1000) > 10000) {
    setPhasesPWM(Ph_A, Ph_B);
    myMicroDelay(del);
    setPhasesPWM(Ph_B, Ph_C);
    myMicroDelay(del);
    setPhasesPWM(Ph_C, Ph_A);
    myMicroDelay(del);
  }
}

void run_motor() {
  /* Run the motor in half step mode. Switching sequence:
      Positive    Negative    Open
         A           B          C
         B           C          A
         C           A          B   
  */
  
  while (1) {
    setPhasesPWM(Ph_A, Ph_B);
    waitBEMF(BEMF_C);
    setPhasesPWM(Ph_B, Ph_C);
    waitBEMF(BEMF_A);
    setPhasesPWM(Ph_C, Ph_A);
    waitBEMF(BEMF_B);
  }
}

void testMotor() {
  long del = 130000; //Set this depending on the motor
  while (1) {
    setPhasesPWM(Ph_A, Ph_B);
    myMicroDelay(del);
    setPhasesPWM(Ph_B, Ph_C);
    myMicroDelay(del);
    setPhasesPWM(Ph_C, Ph_A);
    myMicroDelay(del);
  }
}

void testPhases() {
  setPhasesPWM(Ph_A, Ph_C);
  while (1) {
    Serial.println(digitalRead(BEMF_B));
  }
}

void loop() {
  //testPhases();
  //testMotor();

  start_motor();
  run_motor();
}
