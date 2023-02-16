#include <pin_define.h>
extern Bounce2::Button limitBtn;
extern bool runOnce,sendFlag,doorStart,gameready;


bool checkSwitches(bool doorShut)
{
    bool reedOn;
    reedOn = digitalRead(reedIn);

    #ifdef debugverbose
    Serial.println("in Safety Functions");
    Serial.print("Check Reed: "); 
    Serial.println(reedOn);
    #endif
    
    //update button presses
    limitBtn.update();
    doorShut = limitBtn.isPressed();
    #ifdef debugverbose
    Serial.print("Check Limit Switch: ");    
    Serial.println(doorShut);
    #endif

    //clear the door start once the door is open
    if(doorStart == 1 && reedOn == false && doorShut == false)
    {  
       //Serial.println("Door is cleared");
       doorStart = 0; 
     }

    if (reedOn == true && doorShut == true && runOnce == false && doorStart == 0) 
    {   
        #ifdef debugverbose
        Serial.println("The Door is shut");
        #endif
        digitalWrite(doorLatchpin,HIGH);
        return doorShut = true;
        
    }
    else if (reedOn == false && doorShut == false)
    {
        //Keep maglock off && pass back that the door is open 
        //The motor should not be able to run
        digitalWrite(doorLatchpin,LOW);
        runOnce = false;
        sendFlag = false;

        #ifdef debugverbose
        Serial.print("runOnce in Safety:");
        Serial.println(runOnce);
        #endif
        
        return doorShut = false;
    }

    //assume the door is open
     return doorShut = false;  
}

void lockDoor(bool locked)
{
    if(locked == true)
    {
    digitalWrite(doorLatchpin,HIGH);
    digitalWrite(lockedLight,HIGH);
    digitalWrite(unlockedLight,LOW);
    }else
    {
    digitalWrite(doorLatchpin,LOW);
    digitalWrite(lockedLight,LOW);
    digitalWrite(unlockedLight,HIGH);   
    }
    
}