/*
Basic operation:

Door Closes & sensed via a reed switch. The door locks via maglock.
Stepper motor rotates until hammer drops.
When the hammer drops read accelerometer and send array to the processing companion program
Motor resets position and unlocks door for next operation.

Reed switch Pinout
white - Common
brown - NC
black - NO  --> This should be conencted

PINOUT
IR - D8
Reed - D4
Limit Switch - D0
Motor Dir - D10
Motor Step - D9
Motor En - D1
Locked Light - D12
Unlocked Light - D11
Mag Lock - D13

bytes being sent or received:
44 , value seperate to be delimited in processing
35 # start of array
10 lf end of array
36 $ Not Used
37 % Not Used
38 & Not Used
33 ! Send Data

8/29/22: Moved all pin def to its own header
         Rewrote homing sequence to find hammer arm with IR at the top of its swing
*/

//include subroutines
#include <pin_define.h>
#include <safety_functions.h>
#include <hammer_functions.h>



//Limit switch, used as a back safety for the reed switch.
Bounce2::Button limitBtn = Bounce2::Button();

//may not use the change state varriables 
int reedState      = 0;     // current state of the button
int lastReedState  = 0;     // previous state of the button




int ByteRecv, ByteSend = 0; 
bool gameready = false;

const int BUFFER_SIZE = 1;
byte buf[BUFFER_SIZE];

//flags
bool doorShut = false, reedOn= false, lockedOn= false, dropHammer= false, sendFlag= false, runOnce = true,doorStart = false;
bool serialCheck = true;

void setup() 
{

    //set pinmode
  pinMode(unlockedLight, OUTPUT);
  digitalWrite(unlockedLight, LOW);
  pinMode(lockedLight, OUTPUT);
  pinMode(doorLatchpin, OUTPUT);
  pinMode(reedIn, INPUT_PULLUP); 


  //start serial
  Serial.begin(115200);
  Serial.setTimeout(5);

  //delay until connected  
  while (!Serial)
    delay(10);


//set Clock speed for accel
Wire.setClock(400000);

//set accelerometer I2C

  if (!dso32.begin_I2C()) 
  {    
    while (1) {      
      delay(10);
      #ifdef debug
      Serial.println("Can't find accel");
      #endif
    }
  }

//Sets accelerometer Range

dso32.setAccelRange(LSM6DSO32_ACCEL_RANGE_32_G);
//Sets accelerometer Data Rate
dso32.setAccelDataRate(LSM6DS_RATE_6_66K_HZ);



  //Set the motor to start enabled
  digitalWrite(motorEnb, LOW);
  hammerStep.setSpeed(motorSpeed);

  //saftey button setup
  limitBtn.attach(limitSwitch, INPUT_PULLUP);
  limitBtn.interval(5);
  limitBtn.setPressedState(LOW);

  #ifdef debug
  Serial.println("Homing...");
  #endif
  delay(500);
  findHome();

  #ifdef debug
  Serial.println("Home Set to IR Position");
  #endif
  //Keep the door unlocked if it is closed at startup
  //this should be the default state
  limitBtn.update();
  if(limitBtn.isPressed())
  {
    lockDoor(false);
    delay(20);
  }

  }

void loop() 
{

  //Check the sensor states
  //Reed switch(reedON) should be high && limit(doorShut) switch should be high

  //this is blocking code but has a 5ms timeout
  Serial.readBytesUntil('\n', buf, BUFFER_SIZE);
  doorShut = checkSwitches(doorShut);

  //Run the hammer drop routine if the door is shut
  //This also will start the sensor reading routine
  #ifdef debug
  Serial.print("doorShut: ");
  Serial.println(doorShut);
  #endif

  if(doorShut == true)
  {
    
    //Stop the video & start the game timer
    if(gameready == false && dropHammer == false)
    {    
      Serial.print("#");
      Serial.write(10);
      dropHammer = true;
      
    }



    if(buf[0] == 33)
    {
      buf[0] = 0;
      #ifdef debug
      Serial.println("Game Ready");
      #endif  
      gameready = true;           
    } 

 
    if(dropHammer == true && gameready == true)
    {
      #ifdef debugverbose
      Serial.println("Drop Hamer");
      #endif
      hammerDrop();     
    }
  }

    if(buf[0] == 37)
  {
    buf[0] = 0;  
    gameready = false; 
    dropHammer = false;      
    lockDoor(false);        
  }

}



