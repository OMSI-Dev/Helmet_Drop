#include <pin_define.h> 

int steps = 200;
byte microstep = 4;
Stepper hammerStep(steps * microstep, motorStepPos,motorStepNeg,motorDirPos, motorDirNeg);
byte maxCount;
byte driftCount;

//Declare Accel
Adafruit_LSM6DSO32 dso32;

extern bool sendFlag, runOnce, gameready;
int homeSpeed = -50; //how many steps to take each pass during homing
int homepos = (homeSpeed*-1); //Set to home speed to start. After homing it sets to 6400 This is the best position for the Hammer.
int totalsteps = 6400; //this is based off of the stepper counting by 50
byte homeCount = 0;
//unsigned int btnWait = 1200; Regular value; trying faster value.
unsigned int btnWait = 500;

bool reedSense;
bool safePress;
bool ranAtStart;
bool setOnce;

//data mapping
//only for high value; low is always 0
int highMap = 300;
int scalar = 1;
const int NUM_TO_MODIFY = 44;
const double PERCENT_REDUCTION = 0.35; 

bool stallFlag = false;
MoToTimer resetTimer;

/* Stepper Driver Steps to PA Setting*/ //OLD MOTOR
/* Driver Setting         StepTotal(on Arduino)     speed(RPM in setup)   */
/*    400                  1600           25                  */
/*    800                  3200           20                  */
/*    1600                 6400           20                  */
/*    3200                 12800          45                  */


void findHome()
{    
    if(ranAtStart == true && setOnce ==false )
    {
    //Reset Code to rehome
    homeSpeed = -50; //how many steps to take each pass during homing
    homepos = (homeSpeed*-1); //Set to home speed to start. After homing it sets to 6400 This is the best position for the Hammer.
    homeCount = 0;
    setOnce = true;
    }

    //This runs on start up so that the motor always starts in the same position.
    // this may be off a 10-20 steps but close enough to start the program
    hammerStep.setSpeed(100);
    bool irBool;
    bool startup = false;
    bool Pressed;

do
{   
    //Waits to run homing until the door is shut
    limitBtn.update();
    Pressed = limitBtn.isPressed();
    #ifdef debug
    Serial.print("Pressed: ");
    Serial.println(Pressed);
    #endif

}while(Pressed == LOW);
//locks the door for homing

    lockDoor(true); 
    #ifdef debug
    Serial.println("Find Home");
    Serial.print("Startup:");
    Serial.println(startup);
    #endif


    do
    {       
        irBool = digitalRead(irIN);

        #ifdef debug
        Serial.println("In startup, locating edge of hammer...");
        Serial.print("IR:");
        Serial.println(irBool);
        #endif

        if(irBool != 0)
        {
            startup = true;
        } else
        {
            hammerStep.step(homeSpeed);
        }   

    } 
    while(startup == false);

//Find the top most position

    if(startup == true)
    {  

        //this is a delay to help clean up readings.
        delay(1);
        irBool = digitalRead(irIN);
        
        #ifdef debug
        Serial.println("Found edge...");
        Serial.print("IR:");
        Serial.println(irBool);
        #endif

        do
        {
            delay(1);
            irBool = digitalRead(irIN);        
            #ifdef debug
            Serial.print("IR:");
            Serial.println(irBool);
            Serial.println("moving to 0 position");
            #endif
            hammerStep.step(homeSpeed);
            #ifdef debug
            Serial.print("Home Step Count: ");
            Serial.println(homepos);
            Serial.print("Home Count: ");
            Serial.println(homeCount);
            #endif
            homepos = homepos + 50;
            if(irBool == 0 && homeCount != 1){homepos = 0;homeCount++;}; //this finds the start of the edge of the hammer      
            if(irBool == 1 && homeCount == 1){homepos = 0;homeCount++;}; //this finds the end of the edge of the hammer (should be ~1300 steps) 
            if((totalsteps-homepos) <= 6000 && irBool == 0){homeCount++;} //this advances the count to leave the loop
            if(homeCount == 2){homepos = totalsteps;} //6400 is a full rotation
        }while(homeCount != 2);
       
         homepos = (homepos * -1); // convert to negative so the motor spins in the correct direction
         hammerStep.step(homepos); // this sets the hammer to be raised
         ranAtStart = true;        
        #ifdef debug
        Serial.print("Home Step: ");
        Serial.println(homepos);
        #endif

    }
}

void hammerDrop()
{   
    maxCount = 0;
    //Lock the door and turn on/off the lights
    lockDoor(true);
    //used to read if the IR is sensing the arm    
    bool irSense;
  
    //read IR to confirm hammers position
    irSense = digitalRead(irIN);
    #ifdef debug
    Serial.print("IR:");
    Serial.println(irSense);
    Serial.println("moving to 0 position");
    #endif

    //this is just to make sure the hammer isnt already dropped.
    if(irSense != 1)
    {   
        #ifdef debug
        //Serial.print("Home:");
        //Serial.println(homepos);
        #endif
        //total steps: 6410
        int drop = -500;
        
        #ifdef debug
        Serial.print("homepos: ");
        Serial.println(homepos);
        Serial.print("drop");
        Serial.println(drop);
        #endif

        if(digitalRead(reedIn) == 1)
        {
            //send signal to reset timer
            Serial.write(36);
            Serial.write(10);

            do
            {
            lockDoor(true);            
            limitBtn.update();                       
            } while (limitBtn.isPressed() == false);  

            do
            {
             //check to make sure the door is shut for at least a set time   
             limitBtn.update();
             //Serial.println("Waiting for door to shut...");
            }while(limitBtn.currentDuration() < btnWait);

            if(limitBtn.currentDuration() >= btnWait && limitBtn.isPressed() == true)
            {
                hammerStep.setSpeed(110); //increase the speed right before the drop to get the motor out of the way
                hammerStep.step(drop); 
                
                //SensorVal Array that gets sent to Processing for graphing
                uint16_t arraySize = 200;        
                int sensorVal[arraySize];

                for(byte i=0; i<=199; i++)
                {
                /*Currently all sensors need to be called 
                in order to get accel data*/
                    sensors_event_t accel;
                    sensors_event_t gyro;
                    sensors_event_t temp;
                    dso32.getEvent(&accel,&gyro,&temp);            
                    int smallG = accel.acceleration.z * scalar; 
                    sensorVal[i] = constrain(abs(smallG),0,highMap);
                    if(sensorVal[i] == highMap)
                    {
                        maxCount++;
                    }
                    
                }

                //Checks to see to if any material is in if there is
                //modify the data to lower impact.
                if(maxCount != 4 && maxCount < 4)
                for (int i = 0; i < NUM_TO_MODIFY; ++i) 
                {
                    sensorVal[i] = sensorVal[i] * (1 - PERCENT_REDUCTION);
                }

                //Adds 1 to the first & Last position for processing to confirm that the array is filled
                sensorVal[0] = 1;
                sensorVal[199] = 1;
            
                //Make sure to only send data once per run
                if(sendFlag == false && gameready == true)
                {
                    for(byte i=0; i<=199; i++)
                    {                   
                        Serial.print(sensorVal[i]);

                        if(i != 199){Serial.print(",");}

                        if (i == 199)
                        {   
                            //Set Flags to disable running the motor until the door opens and closes again
                            sendFlag = true;
                            runOnce = true;
                            //Send the array and close the buffer in Processing app
                            Serial.write(10);
                        }
                    }
                    //Max Count could be used to infer if there is a material in the exhibit,
                    //This is within reason, if it is the same hardness as the base material the count would be
                    //basically the same
                    //Material Max count 
                    
                }
                //Move back into position after data is sent
                //Serial.print("Sending to Home");
                // Serial.print(homepos);
                hammerStep.setSpeed(motorSpeed);
                //homepos: 6410
                //drop: 500
                hammerStep.step(homepos - drop);                
                lockDoor(false);
                stallFlag = false;
                Serial.write(64);
                Serial.write(10);
            }//else{Serial.write(72); Serial.write(10);}

        }   
    }else
    {
         do
        {
            //Start Timer & Check IR. IF IR does not clear after a set amount of seconds.
            //Send to homeing sequence.
            if(stallFlag == false)
            {
            resetTimer.setTime(2000);
            stallFlag = true;
            }

            if(resetTimer.expired() && stallFlag == true)
            {
                setOnce = false;
                findHome(); 
               #ifdef debug
                Serial.println(" Resetting home");
               #endif            
            }
            irSense = digitalRead(irIN);

        } while (irSense == true);         
    } 
}