/*************************************************** 
secureBikeLock.ino
Written by Tyler Holland

 ****************************************************/

#include "FPS_GT511C3.h"
#include "SoftwareSerial.h"

#include <LiquidCrystal.h>

unsigned long time;

/* ************ SET-UP PROCESSES ************ */

/* *********** LOCKING PROCESSES *********** */
void lock();
void unlock();

/* ********** ENROLLMENT PROCESSES ********** */
void enroll_first_print();
void enroll_additional_print();
void Enroll();

/* ********** CONSTRUCTORS ********** */
// Hardware setup - FPS connected to:
//    digital pin 4(arduino rx, fps tx)
//    digital pin 5(arduino tx - 560ohm resistor fps tx - 1000ohm resistor - ground)
//    this brings the 5v tx line down to about 3.2v so we dont fry our fps

FPS_GT511C3 fps(4, 5);  // TODO check these numbers

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  // TODO check these numbers

/* 
 *  hold button on for 10s, scan known print -> enroll a new one
 *  if no known fingerprints in database, enroll a new fingerprint **in setup**
 *  
 *  lcd comms
 *  get a fingerprint, activate(open) locking mechanism 
 *  lcd comms
 *  
 *  when to activate(close) locking mechanism?
 *    second button will close lock - no fingerprint required
 *  
 *    on power-up, engage lock (check lock position: open=>close, close=>close)
 *    on finger scan, disengage lock
 */

void setup()
{
  // TODO: load in prints?

    time = millis();
    Serial.begin(9600);  // open line to communicate via serial
    Serial.println("Initializing...");

    // init lcd screen
    lcd.begin(16, 2);  // TODO number check

    // init fingerprint scanner
    delay(100);
    fps.Open();  // init device
    fps.SetLED(true);  // activate backlight for reading

    if (fps.GetEnrollCount() <= 0) {  // no fingerprints enrolled
        enroll_first_print();  // enroll a new print and add to database
    }

    // init servo

    // init lock button (??)

    time = millis();
    Serial.println("Press finger to scanner...");
}

void loop()
{
    delay(50);  // don't need to run this at full speed...
    
    if (fps.IsPressFinger()) {  // a finger is pressed
        fps.CaptureFinger(false);  // low quality capture
        int id = fps.Identify1_N();
        if (id < 200)
        {
            Serial.print("Verified ID:");
            Serial.println(id);
            unlock();
        }
        else
        {
            Serial.println("Finger not found");
        }
    }
    
    /*else if (...) { // lock button is pressed
        lock();
    }//*/
    
    else if (millis() - time >= 10000) {  // waited 10+ seconds
        Serial.println("Entered enrollment mode");
        enroll_additional_print();
    }

}



/***********************************
ON START-UP INITIALIZATION PROCESSES
***********************************/
//..\\


/*************************************
SERVOMOTOR LOCKING/UNLOCKING PROCESSES
*************************************/
void lock() {
    Serial.println("Locking...");
    // servo locks
    Serial.println("Locked!");
}

void unlock() {
    Serial.println("Unlocking...");
    // servo unlocks here
    Serial.println("Unlock");
}
 
/****************************
LCD MONITOR DISPLAY PROCESSES
****************************/
//...\\


/****************************
LCD MESSAGE DISPLAY PROCESSES
****************************/
//...\\


/****************************
FINGERPRINT SCANNER PROCESSES
****************************/
//..\\


/***********************************
NEW FINGERPRINT ENROLLMENT PROCESSES
***********************************/
void enroll_first_print() {
    Serial.println("No authorized fingerprints.");
    Serial.println("Adding new user");
    Enroll();
}

void enroll_additional_print() {
    Serial.println("Adding new user...");
    Serial.println("Scan authorized print.");

    while(fps.IsPressFinger() == false)  // wait for a finger
        delay(100);
    fps.CaptureFinger(false);  // low quality capture
    
    int id = fps.Identify1_N();
    if (id < 200)
    {
        Serial.print("Verified ID:");
        Serial.println(id);
        Serial.println("Enrolling new print");
        Enroll();
    }
    else
    {
        Serial.println("Finger not found");
    }
}

void Enroll()
{
    // find an open enroll id
    int enrollid = 0;
    bool usedid = true;
    while (usedid == true)  // int enrollid = -1; while (fps.CheckEnrolled(++enrolledid));  // better?
    {
        usedid = fps.CheckEnrolled(enrollid);
        if (usedid==true) enrollid++;
    }
    if (fps.EnrollStart(enrollid) != 0) {  // cannot start enroll process
        Serial.println("Enrollment failed: database error");
        return;
    }
  
    // enroll!
    Serial.print("Press finger to enroll");
    while(fps.IsPressFinger() == false)  // wait for a finger
        delay(100);
    int iret = 0;
    bool bret = fps.CaptureFinger(true);  // take image of finger
    if (bret != false)
    {
        Serial.println("Remove finger");
        fps.Enroll1();  // process saved image
        while(fps.IsPressFinger() == true)  // make them remove finger to proceed
            delay(100);
        Serial.println("Press same finger again");
        while(fps.IsPressFinger() == false)
            delay(100);
        bret = fps.CaptureFinger(true);
        if (bret != false)
        {
            Serial.println("Remove finger");
            fps.Enroll2();
            while(fps.IsPressFinger() == true)
                delay(100);
            Serial.println("Press same finger yet again");
            while(fps.IsPressFinger() == false)
                delay(100);
            bret = fps.CaptureFinger(true);
            if (bret != false)
            {
                Serial.println("Remove finger");
                iret = fps.Enroll3();
                if (iret == 0)
                {
                    Serial.println("Enrolling Successfull");
                }
                else
                {
                    Serial.print("Enrolling Failed with error code:");
                    Serial.println(iret);
                }
            }
            else Serial.println("Failed to capture third finger");
        }
        else Serial.println("Failed to capture second finger");
    }
    else Serial.println("Failed to capture first finger");
}




