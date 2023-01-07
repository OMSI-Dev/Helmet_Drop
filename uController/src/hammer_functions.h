#include <pin_define.h> 
extern Stepper hammerStep;
extern bool sendFlag, runOnce, gameready;
int homeSpeed = -100; //how many steps to take each pass during homing
int homepos = (homeSpeed*-1); //Set to home speed to start. After homing it sets to 12000. This is the best position for the Hammer.
int totalsteps = 12800; //this is based off of the stepper counting by 100
byte homeCount = 0;
/* Stepper Driver Steps to PA Setting*/
/* Driver Setting         StepTotal(on Arduino)     speed(RPM in setup)   */
/*    400                  1600           25                  */
/*    800                  3200           20                  */
/*    1600                 6400           20                  */
/*    3200                 12800          45                  */


void findHome()
{    
    //This runs on start up so that the motor always starts in the same position.
    // this may be off a 1-3 steps but close enough to start the program
    int irSense;
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

    digitalWrite(doorLatchpin,HIGH);
    digitalWrite(lockedLight,HIGH);
    digitalWrite(unlockedLight,LOW);
    
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
        Serial.println(irSense);
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
            #endif
            homepos = homepos + 100;
            if(irBool == 0 && homeCount != 1){homepos = 0;homeCount++;}; //this finds the start of the edge of the hammer
            if(irBool == 1 && homeCount == 1){homepos = 0;homeCount++;}; //this finds the end of the edge of the hammer (should be 1300 steps)            
            if((homepos-totalsteps) >= 1300 && irBool == 0){homeCount++;} //this advances the count to leave the loop
            if(homeCount == 2){homepos =12300;} //12300 should bring the natualis gear to the edge of the hammer ready to drop
        }while(homeCount != 2);
       
        homepos = (homepos * -1); // convert to negative so the motor spins in the correct direction
        hammerStep.step(homepos); // this sets the hammer to be raised
        #ifdef debug
        Serial.print("Home Step: ");
        Serial.println(homepos);
        #endif

    }
}

void hammerDrop()
{   
    
    //Lock the door and turn on/off the lights
    lockDoor(true);
    //used to read if the IR is sensing the arm    
    bool irSense;
  
    //send signal to reset timer
    Serial.write(36);
    Serial.write(10);

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
        
        int drop = (homepos*-1) - (totalsteps-5);

        
        #ifdef debug
        Serial.print("homepos: ");
        Serial.println(homepos);
        Serial.print("drop");
        Serial.println(drop);
        #endif
        hammerStep.setSpeed(80); //increase the speed right before the drop to get the motor out of the way
        hammerStep.step(5);
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
            int smallG = accel.acceleration.z * 150; 
            //Adds 1 to the first & Last position for processing to confirm that the array is filled
            if(i == 0 || i == 199)
            {
            sensorVal[i] = 1;
            
            }else(sensorVal[i] = constrain(abs(smallG),0,3000));
        }
     
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
        }
        //Move back into position after data is sent
        //Serial.print("Sending to Home");
       // Serial.print(homepos);
        hammerStep.setSpeed(motorSpeed);
        hammerStep.step(homepos);

        //Hard Delay....Timer code was not working
        //Hard Delay could stay the arduino controls the game state
        //Processing will wait for data to be passed to it
        //delay(2000);
        //unlock the maglock
        lockDoor(false);

    } 
}

