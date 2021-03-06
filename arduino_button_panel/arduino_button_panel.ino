#include <yeltrowEncoder.h>


//#include <RotaryEncoder.h>

#include <Modbusino.h>

/*
 * mb2hal_example_01_arduino.ino
 * Arduino sketch to connect to HAL's LinuxCNC using mb2hal component.
 * Tested 2012-10-12 with Arduino Mega 2560 R3.
 *
 * Victor Rocco, adapted from Stéphane Raimbault's source code wich is
 * Copyright © 2011-2012 Stéphane Raimbault <stephane.raimbault@gmail.com>
 * License ISC, see LICENSE for more details.
 */

/* Adapted by Matt Wortley for a custom arduino based hardwarepanel for
 * a Shizuoka AN-S Milling machine.  
 * 
 * Inputs conntected to buttons and knobs are in words 0-9
 * Outputs connection to lights and stuff are in words 10 and up
 */

#include <HardwareSerial.h>
#include <Modbusino.h>

#define MINPIN 22
#define MAXPIN 53

#define FEEDHOLD 53
#define PAUSE 26
#define STEP 27
#define RUN 28
#define STOP 29
#define MAXSPEED 15
#define JOGA 30
#define JOGB 31
#define SPINDLEOVERRIDEA 32
#define SPINDLEOVERRIDEB 33
#define XBUT 41
#define YBUT 40
#define ZBUT 43
#define ABUT 42
#define KNEEBUT 45
#define NONEBUT 44
#define FEEDOVERRIDEA 46
#define FEEDOVERRIDEB 47
#define COOLANTON 49
#define COOLANTOFF 48
#define SPINDLEON 51
#define SPINDLEOFF 50

#define DECIMAL_BITSHIFT 5
#define SUMMER_BITSHIFT 7

/* Initialize the slave with the ID 1 */
ModbusinoSlave modbusino_slave(1);

/* Allocate a mapping of 10 values */
uint16_t tab_reg[10];
RotaryEncoder spindle_enc(SPINDLEOVERRIDEA, SPINDLEOVERRIDEB, 5, 6, 3000);
RotaryEncoder feed_enc(FEEDOVERRIDEA, FEEDOVERRIDEB, 5, 6, 3000);
RotaryEncoder jog_enc(JOGA, JOGB, 5, 6, 3000);

void setup() {
    /* The transfer speed is set to 115200 bauds */
    modbusino_slave.setup(115200);    
    pinMode(FEEDHOLD,INPUT_PULLUP);
    pinMode(PAUSE,INPUT_PULLUP);
    pinMode(STEP,INPUT_PULLUP);
    pinMode(RUN,INPUT_PULLUP);
    pinMode(STOP,INPUT_PULLUP);
    pinMode(MAXSPEED,INPUT_PULLUP);
    pinMode(JOGA,INPUT_PULLUP);
    pinMode(JOGB,INPUT_PULLUP);
    pinMode(SPINDLEOVERRIDEA,INPUT_PULLUP);
    pinMode(SPINDLEOVERRIDEB,INPUT_PULLUP);
    pinMode(XBUT,INPUT_PULLUP);
    pinMode(YBUT,INPUT_PULLUP);
    pinMode(ZBUT,INPUT_PULLUP);
    pinMode(ABUT,INPUT_PULLUP);
    pinMode(KNEEBUT,INPUT_PULLUP);
    pinMode(NONEBUT,INPUT_PULLUP);
    pinMode(FEEDOVERRIDEA,INPUT_PULLUP);
    pinMode(FEEDOVERRIDEB,INPUT_PULLUP);
    pinMode(COOLANTON,INPUT_PULLUP);
    pinMode(COOLANTOFF,INPUT_PULLUP);
    pinMode(SPINDLEON,INPUT_PULLUP);
    pinMode(SPINDLEOFF,INPUT_PULLUP);
    
}

 
void loop() {
    static long spindleoverride=(1L<<30);
    static long feedoverride=(1L<<30);
    static long jog=(1L<<30);
    static long i;
    int linuxcnchb = 0;
    static long maxVelSum = 0;
    
    i++;
    static long pos;
        /* Launch Modbus slave loop with:
       - pointer to the mapping
       - max values of mapping */
    modbusino_slave.loop(tab_reg, 10);

    // process any messages from modbus 
    linuxcnchb = (tab_reg[9] & 0x0001);
    
    // Pack in all of the single bit values
    tab_reg[0] = digitalRead(FEEDHOLD);
    tab_reg[0]=(tab_reg[0]<<1)+digitalRead(PAUSE);
    tab_reg[0]=(tab_reg[0]<<1)+digitalRead(STEP);
    tab_reg[0]=(tab_reg[0]<<1)+digitalRead(RUN);
    tab_reg[0]=(tab_reg[0]<<1)+digitalRead(STOP);
    tab_reg[0]=(tab_reg[0]<<1)+digitalRead(XBUT);
    tab_reg[0]=(tab_reg[0]<<1)+digitalRead(YBUT);
    tab_reg[0]=(tab_reg[0]<<1)+digitalRead(ZBUT);
    tab_reg[0]=(tab_reg[0]<<1)+digitalRead(ABUT);
    tab_reg[0]=(tab_reg[0]<<1)+digitalRead(KNEEBUT);
    tab_reg[0]=(tab_reg[0]<<1)+digitalRead(NONEBUT);
    tab_reg[0]=(tab_reg[0]<<1)+digitalRead(COOLANTON);
    tab_reg[0]=(tab_reg[0]<<1)+digitalRead(COOLANTOFF);
    tab_reg[0]=(tab_reg[0]<<1)+digitalRead(SPINDLEON);
    tab_reg[0]=(tab_reg[0]<<1)+digitalRead(SPINDLEOFF);
    // Heartbeat occupies the bottom 1 bit - It is the same as the value
    // passed to us.  Linuxcnc is responsible for inverting and sending
    // it again.  This is to eliminate aliasing effects caused by having
    // a free runnig asynchromous timier in our logic.
    tab_reg[0]=(tab_reg[0]<<1)+linuxcnchb;
    // Heartbeat gets the whole next register for now...
    //tab_reg[1] = ((i>>6) & 0x0007);
    tab_reg[1] = tab_reg[9];
    spindleoverride=spindleoverride+spindle_enc.readEncoder();
    // Map bottom 16 bits of encoder in
    tab_reg[2] = 0xFFFFL & spindleoverride;
    // Map top 16 bits of encoder in
    tab_reg[3] = spindleoverride>>16;
    feedoverride=feedoverride+feed_enc.readEncoder();
    tab_reg[4] = 0xFFFFL & feedoverride;
    tab_reg[5] = feedoverride>>16;
    jog=jog+jog_enc.readEncoder();
    tab_reg[6] = 0xFFFFL & jog;
    tab_reg[7] = jog>>16;
    //   Use a moving average filter on the data
    //   Bitshifts are used instead of divides because they are faster
    //   If we have N samples in the average, we subtract 1/N * the sum like we
    //   are getting rid of a single sample.  
    maxVelSum = maxVelSum - ( maxVelSum >> SUMMER_BITSHIFT);
    // We multiply the analog value by a fixed power of two to increase the resolution
    // of the running average, like fixed decimal but with binary.
    maxVelSum = maxVelSum + ((long)analogRead(A15) << DECIMAL_BITSHIFT);
    tab_reg[8] = (maxVelSum >> (SUMMER_BITSHIFT));
    
}
