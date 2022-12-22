#include <pin_define.h> 
extern Stepper hammerStep;
extern bool sendFlag, runOnce, gameready;
int homeSpeed = -100;
int homepos = (homeSpeed*-1); //Set to home speed to start. After homing it sets to 10700. This is the best positino for the Hammer.
int totalsteps = 12800;
byte homeCount = 0;
/* Stepper Driver Steps to PA Setting*/
/* Driver Setting         StepTotal     speed(RPM in setup)   */
/*    400                  1600           25    */
/*    800                  3200           20    */
/*    1600                 6400           20    */
/*    3200                 12800          45    */


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
            //if(irBool == 0 && homeCount != 1){homepos = 0;};
            if((homepos-totalsteps) >= 10700 && irBool == 0){homeCount++;} 
            if(homeCount == 2){homepos = 10700;}
        }while(homeCount != 2);
       
        homepos = (homepos * -1); // convert to negative so the motor spins in the correct direction
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
        int drop = (homepos*-1) - totalsteps;
        #ifdef debug
        Serial.print("Drop Pos:");
        Serial.println(drop);
        #endif
        hammerStep.step(drop); 

        //SensorVal Array that gets sent to Processing for graphing
        uint16_t arraySize = 200;        
        int sensorVal[arraySize];

        for(byte i=0; i<=199; i++)
        {
         /*Currently all sensors need to be called 
         in order to get accel data*/
        //  sensors_event_t accel;
        //  sensors_event_t gyro;
        //  sensors_event_t temp;
        //  dso32.getEvent(&accel,&gyro,&temp);
        //  int smallG = accel.acceleration.z *8; 
        //sensorVal[i] = constrain(abs(smallG), 0, 3000);
            // if(i != 0){
            // sensorVal[i] = constrain(abs(smallG), 0, 3000);
            // }else{sensorVal[i]=1;} //Adds 1 to the first position for processing to confirm that the array is filled
            
            //random data to fill packet for testing.
            //Adds 1 to the first&Last position for processing to confirm that the array is filled
            if(i != 0 || i != 199){sensorVal[i] = analogRead(A0) + random(0,1000);}
            if(i == 199 || i == 0){sensorVal[i]=1;}
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
        hammerStep.step(homepos);

        //Hard Delay....Timer code was not working
        //Hard Delay could stay the arduino controls the game state
        //Processing will wait for data to be passed to it
        //delay(2000);
        //unlock the maglock
        lockDoor(false);

    } 
}

