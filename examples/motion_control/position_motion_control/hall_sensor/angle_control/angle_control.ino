/**
 * 
 * Position/angle motion control example
 * Steps:
 * 1) Configure the motor and hall sensor  
 * 2) Run the code
 * 3) Set the target angle (in radians) from serial terminal
 * 
 * 
 * NOTE :
 * > This is for Arduino UNO example code for running angle motion control specifically
 * > Since Arduino UNO doesn't have enough interrupt pins we have to use software interrupt library PciManager.
 *  
 * > If running this code with Nucleo or Bluepill or any other board which has more than 2 interrupt pins 
 * > you can supply doIndex directly to the sensr.enableInterrupts(doA,doB,doC) and avoid using PciManger
 * 
 * 
 */
#include <SimpleFOC.h>
// software interrupt library
#include <PciManager.h>
#include <PciListenerImp.h>

// motor instance
BLDCMotor motor = BLDCMotor(9, 10, 11, 11, 8);

// hall sensor instance
HallSensor sensor = HallSensor(2, 3, 4, 11);

// Interrupt routine intialisation
// channel A and B callbacks
void doA(){sensor.handleA();}
void doB(){sensor.handleB();}
void doC(){sensor.handleC();}
// If no available hadware interrupt pins use the software interrupt
PciListenerImp listenC(sensor.pinC, doC);


void setup() {
  
  // initialize sensor hardware
  sensor.init();
  sensor.enableInterrupts(doA, doB); //, doC); 
  // software interrupts
  PciManager.registerListener(&listenC);
  // link the motor to the sensor
  motor.linkSensor(&sensor);

  // power supply voltage [V]
  motor.voltage_power_supply = 12;
  // aligning voltage [V]
  motor.voltage_sensor_align = 3;
  // index search velocity [rad/s]
  motor.velocity_index_search = 3;

  // set motion control loop to be used
  motor.controller = ControlType::angle;

  // contoller configuration 
  // default parameters in defaults.h

  // velocity PI controller parameters
  motor.PID_velocity.P = 0.2;
  motor.PID_velocity.I = 2;
  motor.PID_velocity.D = 0;
  // default voltage_power_supply
  motor.voltage_limit = 6;
  // jerk control using voltage voltage ramp
  // default value is 300 volts per sec  ~ 0.3V per millisecond
  motor.PID_velocity.output_ramp = 1000;
 
  // velocity low pass filtering time constant
  motor.LPF_velocity.Tf = 0.01;

  // angle P controller
  motor.P_angle.P = 20;
  //  maximal velocity of the position control
  motor.velocity_limit = 4;


  // use monitoring with serial 
  Serial.begin(115200);
  // comment out if not needed
  motor.useMonitoring(Serial);
  
  // initialize motor
  motor.init();
  // align sensor and start FOC
  motor.initFOC();


  Serial.println("Motor ready.");
  Serial.println("Set the target angle using serial terminal:");
  _delay(1000);
}

// angle set point variable
float target_angle = 0;

void loop() {
  // main FOC algorithm function
  // the faster you run this function the better
  // Arduino UNO loop  ~1kHz
  // Bluepill loop ~10kHz 
  motor.loopFOC();

  // Motion control function
  // velocity, position or voltage (defined in motor.controller)
  // this function can be run at much lower frequency than loopFOC() function
  // You can also use motor.move() and set the motor.target in the code
  motor.move(target_angle);

  // function intended to be used with serial plotter to monitor motor variables
  // significantly slowing the execution down!!!!
  // motor.monitor();
  
  // user communication
  serialReceiveUserCommand();
}

// utility function enabling serial communication with the user to set the target values
// this function can be implemented in serialEvent function as well
void serialReceiveUserCommand() {
  
  // a string to hold incoming data
  static String received_chars;
  
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the string buffer:
    received_chars += inChar;
    // end of user input
    if (inChar == '\n') {
      
      // change the motor target
      target_angle = received_chars.toFloat();
      Serial.print("Target angle: ");
      Serial.println(target_angle);
      
      // reset the command buffer 
      received_chars = "";
    }
  }
}