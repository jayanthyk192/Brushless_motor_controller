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
