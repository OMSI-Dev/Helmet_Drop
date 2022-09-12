// PINOUT
// IR - D8
// Reed - D4
// Limit Switch - D0
// Locked Light - D12
// Unlocked Light - D11
// Mag Lock - D13
// Motor Dir - D10
// Motor Step - D9
// Motor En - D1



/*Stepper Motor
Pul - (GND)
Pul + (9/Step)
Dir - (GND)
Dir + (10 DIR)

*/

#define irIN 8
#define limitSwitch 0
#define reedIn 4
#define doorLatchpin 13
#define lockedLight 12
#define unlockedLight 11
#define motorStep 9
#define motorDir 10
#define motorEnb 1

//#define debug 