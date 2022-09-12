#include <pin_define.h> 
extern Stepper hammerStep;
extern bool sendFlag, runOnce, gameready;

void findHome()
{    
    //This runs on start up so that the motor always starts in the same position.
    // this may be off a 1-3 steps but close enough to start the program
    int irSense;
    bool irBool;
    bool startup = false;
    bool Pressed;
    bool home = 1;    


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

    //push the gear forward to cause homing to happen at least once
    if(home == 1)
    {
    hammerStep.step(-1000);
    //hard delay to wait for the motor to catch up to the code
    delay(500);
    home = 0;
    }
    
    #ifdef debug
    Serial.println("Find Home");
    Serial.print("Startup:");
    Serial.println(startup);
    #endif

//locks the door for homing
digitalWrite(doorLatchpin,HIGH);
digitalWrite(lockedLight,HIGH);
digitalWrite(unlockedLight,LOW);

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
            hammerStep.step(-10);   
        }   

    } 
    while(startup == false);

    if(startup == true)
    {  
        //this is a delay to help clean up readings.
        delay(5);
        irBool = digitalRead(irIN);
        
        #ifdef debug
        Serial.println("Found edge...");
        Serial.print("IR:");
        Serial.println(irSense);
        #endif

        do
        {
        delay(5);
        irBool = digitalRead(irIN);        

        #ifdef debug
        Serial.print("IR:");
        Serial.println(irBool);
        Serial.println("moving to 0 position");
        #endif
        hammerStep.step(-1);
        }while(irBool == 1);
       
        //move the gear to dropping position
        hammerStep.step(-1500);

    }
}

void hammerDrop()
{   
    //Lock the door and turn on/off the lights
    digitalWrite(doorLatchpin,HIGH);
    digitalWrite(lockedLight,HIGH);
    digitalWrite(unlockedLight,LOW);
    //used to read if the IR is sensing the arm
    #define irIN 8
    //bool irSense;


    //send signal to reset timer
    Serial.write(36);
    Serial.write(10);
    
    //Serial.println(irSense = digitalRead(irIN));
    //irSense = digitalRead(irIN);

         #ifdef debug
        Serial.print("IR:");
        Serial.println(irSense);
        Serial.println("moving to 0 position");
        #endif

    //this is just to make sure the hammer isnt already dropped.
    // if(irSense == 0)
    // {   
        //move the hammer 100 steps to cause it to drop
        //and move back to 0/home                    
        hammerStep.step(-100); 

        //SensorVal Array
        uint16_t arraySize = 200;        
        int sensorVal[arraySize];

        for(byte i=0; i<=199; i++)
        {
         sensors_event_t accel;
         sensors_event_t gyro;
         sensors_event_t temp;

         dso32.getEvent(&accel,&gyro,&temp);
         int smallG = accel.acceleration.z *8; 
         sensorVal[i] = constrain(abs(smallG), 0, 3000);
        
        }
     

        //Make sure to only send data 1 time per run
        if(sendFlag == false & gameready == true)
        {
            //Serial.println("Getting Data: ");
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
        hammerStep.step(-1500);

        //Hard Delay....Timer code was not working
        //Hard Delay could stay the arduino controls the game state
        //Processing will wait for data to be passed to it
        delay(2000);
        //unlock the maglock
        digitalWrite(doorLatchpin,LOW);
        digitalWrite(lockedLight,LOW);
        digitalWrite(unlockedLight,HIGH);        

    //} 
}

