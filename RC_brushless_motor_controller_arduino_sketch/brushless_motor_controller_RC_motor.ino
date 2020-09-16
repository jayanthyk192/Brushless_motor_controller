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

/* Circuit to run the RC Brushless motor can be found in schematics. It's the same 
 * as the Hard drive motor circuit. But the code is significantly changed. 
 *
 * Make sure you power all the ciruits with an SMPS which shuts down when there is
 * a current surge. DO-NOT run this with a LiPo battery. If there are shot-circuits, 
 * you'll end up with a lot of damage.
 * 
 * DO-NOT power the mosfets with voltages below 10V!
 * 
 * DO-NOT run the motor without the big capacitor shown in the schematic, 
 * you'll kill the mosfets very soon!
 */

//Set the Pins on the Arduino that control the N-Mosfets
#define NMOS_A_PWM  3
#define NMOS_B_PWM  5
#define NMOS_C_PWM  6

//Set the Pins on the Arduino that control the P-Mosfets
#define PMOS_A  11
#define PMOS_B  12
#define PMOS_C  13

//Set the Pins on the Arduino that read the Back EMF signals.
#define BEMF_A  A0
#define BEMF_B  A1
#define BEMF_C  A2

#define PWM_A OCR2B
#define PWM_B OCR0B
#define PWM_C OCR0A

#define Ph_A  0
#define Ph_B  1
#define Ph_C  2

int pwmVal = 240;
int disableMosfets = 0; // Set this to 1 to disable all the mosfets

void setup() {
  //These pins are used to control the NMOS transistors
  pinMode(NMOS_A_PWM, OUTPUT);           // set pin to output
  pinMode(NMOS_B_PWM, OUTPUT);           // set pin to output
  pinMode(NMOS_C_PWM, OUTPUT);           // set pin to output

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
  // Switch off all the NMOS (255 is 0% duty cycle, 0 is 100% duty cycle)
  PWM_A = PWM_B = PWM_C = 255;

  //Switch off all the PMOS transistors.
  digitalWrite(PMOS_A, HIGH);
  digitalWrite(PMOS_B, HIGH);
  digitalWrite(PMOS_C, HIGH);

  if (!disableMosfets) {
    switch (Ph_low) {
      case Ph_A:  PWM_A = pwmVal; //Set PWM on Phase A only
        break;
      case Ph_B:  PWM_B = pwmVal; //Set PWM on Phase B only
        break;
      case Ph_C:  PWM_C = pwmVal; //Set PWM on Phase C only
        break;
    }
    digitalWrite(Ph_high + 11, LOW); //Switch ON the required PMOS
  }
}

void waitBEMF(int BEMF_n) {
  //Wait while the Back EMF changes from 0 to 1.
  //for(char a = 0; a < 20; a++)
  while (!digitalRead(BEMF_n));
}

void start_motor() {
  long del = 25000; //Set this depending on the motor.
  while ((del -= 300) > 1000) {
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
         A           C          B
         B           C          A
         B           A          C
         C           A          B
         C           B          A
  */
  char rec;
  while (1) {
    setPhasesPWM(Ph_A, Ph_B);
    waitBEMF(BEMF_C);
    setPhasesPWM(Ph_A, Ph_C);
    waitBEMF(BEMF_B);
    setPhasesPWM(Ph_B, Ph_C);
    waitBEMF(BEMF_A);
    setPhasesPWM(Ph_B, Ph_A);
    waitBEMF(BEMF_C);
    setPhasesPWM(Ph_C, Ph_A);
    waitBEMF(BEMF_B);
    setPhasesPWM(Ph_C, Ph_B);
    waitBEMF(BEMF_A);

    if (Serial.available() > 0) {
      rec = Serial.read();
      switch (rec) {
        case 'a': pwmVal += 10;
          if (pwmVal > 250)
            pwmVal = 250;
          break;
        case 's': pwmVal -= 1;
          if (pwmVal < 0)
            pwmVal = 0;
          break;
        case ' ': disableMosfets = 1;
          break;
      }
      Serial.println(pwmVal);
    }
  }
}

void setupPWM() {
  //Setup timers to generate PWM

  /* This generates PWM on OC0A and OC0B pins.

      [  OC0A Pin ] [ OC0B Pin  ]
      COM0A1 COM0A0 COM0B1 COM0B0   –   –   WGM11 WGM10        TCCR0A
        1       1     1       1     0   0     1     1
         [PWM Inverting mode]              [Top -> 0xff]
  */
  TCCR0A = 0b11110011; //OC0A + OC0B
  TCCR0B = 0b010; // Divide clock by 8

  /*  This generates PWM on OC2B pin.

      [  OC2A Pin ] [ OC2B Pin  ]
      COM2A1 COM2A0 COM2B1 COM2B0   –   –   WGM11 WGM10        TCCR2A
        0       0     1       1     0   0     1     1
         [PWM Inverting mode]              [Top -> 0xff]
  */
  TCCR2A = 0b00110011; //OC2B
  TCCR2B = 0b010; // Divide clock by 8

  /* PWM Frequency:
      Arduino clock -> 8MHZ
      Prescalar -> 1/8
      Counter Max -> 256
      PWM Freqency = 8M /(8 * 256) = 3.906 KHz (Limited by L293d switching frequency)
  */

  /* PWM Inverting mode:
      OCR0A = 0   -> 100% duty cycle
      OCR0A = 255 -> 0% duty cycles
      Same for OCR0B and OCR2B
  */
}

void testMotor() {
  pwmVal = 240; // Set low PWM duty cycle
  long del = 13000; //Set this depending on the motor
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
  pwmVal = 253; // Set low PWM duty cycle
  setPhasesPWM(Ph_A, Ph_C);
  while (1) {
    Serial.println(digitalRead(BEMF_C));
  }
}

void loop() {
  setupPWM();

  //testPhases();
  //testMotor();

  start_motor();
  run_motor();
}
