/*
 * File:        Targeting System
 * Author:      Tyler Thompson (tt395) & Steve Slaughter (sts55)
 * Target PIC:  PIC32MX250F128B
 */

#define CCLK    (40000000)
#define PBCLK   (CCLK/4)
#define Fsck    375000
#define BRG_VAL (PBCLK/2/Fsck)




////////////////////////////////////
// clock AND protoThreads configure!
// You MUST check this file!
#include "config.h"
// threading library
#include "pt_cornell_1_2_1.h"
#pragma config OSCIOFNC = ON


////////////////////////////////////
// graphics libraries
#include "tft_master.h"
#include "tft_gfx.h" //Utilized for testing


#define x_num 320
#define y_num 240
unsigned char pix, capture;
unsigned short state_VS, state_HR, state_PCLK;
unsigned short last_state_VS, last_state_HR, last_state_PCLK;
unsigned char pixel_flag;
unsigned int x, y;
unsigned char pixel;
unsigned short pixels[88][145];
unsigned int medX[3];
unsigned int medY[3];
unsigned int iterator;
unsigned int sumX = 0;
unsigned int sumY = 0;
unsigned int countX = 0;
unsigned int countY = 0;
int dist;
int dist2;
int first = 15;
char buffer[128];
unsigned int temp;
////////////////////////////////////


static unsigned int read(unsigned char add){ //Read SCCB utilizing i2c methods
    unsigned char test;
    OpenI2C1( I2C_ON | I2C_NACK, BRG_VAL);
    StartI2C1();
    IdleI2C1();
    MasterWriteI2C1(0x42);
    IdleI2C1();
    MasterWriteI2C1(add);
    IdleI2C1();
    StopI2C1();
    IdleI2C1();
    StartI2C1();
    IdleI2C1();
    MasterWriteI2C1(0x43);
    IdleI2C1();
    test = MasterReadI2C1();
    IdleI2C1();
    StopI2C1();
    IdleI2C1();
    CloseI2C1(); 
    return test;
}

static void write(unsigned char reg, unsigned char data){ //Write SCCB utilizing i2c methods
    OpenI2C1( I2C_ON, BRG_VAL);
    StartI2C1();
    IdleI2C1();
    MasterWriteI2C1(0x42);
    IdleI2C1();
    MasterWriteI2C1(reg);
    IdleI2C1();
    MasterWriteI2C1(data);
    IdleI2C1();
    StopI2C1();
    IdleI2C1();
    CloseI2C1();
}

int checkDif(unsigned int *pts){ //Verify distance between two points is < 10
    int dif1;
    dif1 = abs(pts[0] - pts[1]);
    
    return (dif1 < 10);
}

void trigger(unsigned int xs, unsigned int ys){ //Aim servos at desired pixel location
    int yTarg;
    int xTarg;
    int i;
    if(ys > 50){
        xTarg = 1520 - (float)(1500-1088) * (1.-((145.-ys*1.05-30)/145.));
    }
    else{
        xTarg = 1520 - (float)(1500-1088) * (1.-((145.-ys*1.05)/145.));
    }
    
    if(xs > 40){
         yTarg = 1800 + (float)(2000-1800) * (1.-((88. - xs*1.1 - 30)/88.));
    }
    else{
         yTarg = 1800 + (float)(2000-1800) * (1.-((88. - xs*0.8 - 30)/88.));
    }
   
    for(i = 0; i < 15 + first; i++){
        mPORTBSetBits(BIT_0);
        mPORTBSetBits(BIT_1);
        if(xTarg > yTarg) {
            delay_us(yTarg);
            mPORTBClearBits(BIT_1);
            delay_us(xTarg-yTarg);
            mPORTBClearBits(BIT_0);
            delay_us(20000 - xTarg);
        }
        else{
            delay_us(xTarg);
            mPORTBClearBits(BIT_0);
            delay_us(yTarg-xTarg);
            mPORTBClearBits(BIT_1);
            delay_us(20000 - yTarg);
        }
    }
    mPORTBSetBits(BIT_2);
    first = 0;
}


// === Main  ======================================================
void main(void) {
  
  ANSELA = 0; ANSELB = 0; 
SYSTEMConfig(40000000, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
  // init the display if testing
  //tft_init_hw();
  //tft_begin();
  //tft_fillScreen(ILI9340_BLACK);
  //240x320 vertical display
  //tft_setRotation(1); // Use tft_setRotation(1) for 320x240

  PT_setup();
  INTEnableSystemMultiVectoredInt();
  
  
  /*PPSInput(3, INT2, RPA4);//Hsync
  PPSInput(1, INT4, RPB7);//Pclk
  PPSInput(2, INT3, RPB5);//Vsync*/
 
  PORTSetPinsDigitalIn(IOPORT_A, BIT_4);//Hsync
  PORTSetPinsDigitalIn(IOPORT_B, BIT_5);//VSync
  PORTSetPinsDigitalIn(IOPORT_B, BIT_7);//Pclk
  PORTSetPinsDigitalIn(IOPORT_B, BIT_10);//Digit0
  PORTSetPinsDigitalIn(IOPORT_A, BIT_0);//Digit1
  PORTSetPinsDigitalIn(IOPORT_A, BIT_1);//Digit2
  PORTSetPinsDigitalIn(IOPORT_B, BIT_3);//Digit3
  PORTSetPinsDigitalIn(IOPORT_B, BIT_4);//Digit4
  PORTSetPinsDigitalIn(IOPORT_B, BIT_13);//Digit5
  PORTSetPinsDigitalIn(IOPORT_A, BIT_2);//Digit6
  PORTSetPinsDigitalIn(IOPORT_B, BIT_15);//Digit7
  mPORTAClearBits(BIT_4 | BIT_0| BIT_1 | BIT_2);
  mPORTBClearBits(BIT_5 | BIT_7 | BIT_10 | BIT_3 | BIT_4 | BIT_13| BIT_15);
  
  PORTSetPinsDigitalOut(IOPORT_B, BIT_0);//Bot
  PORTSetPinsDigitalOut(IOPORT_B, BIT_1);//Vert
  PORTSetPinsDigitalOut(IOPORT_B, BIT_2);//Laser

  
  
  write(0x11, 0xBF);
  write(0x12, 0x0C);
  write(0x3A, read(0x3A) &0xDF);
  
  write(0x40, read(0x40)| 0xD0);
  write(0x15, (1 << 5));
  temp = read(0x0C);
  write(0x0C, temp | 0x04);


  while (1){
      x=0;
      y=0;
      while(!mPORTBReadBits(BIT_5));
      while(mPORTBReadBits(BIT_5));
      while(!mPORTBReadBits(BIT_5)){
          while(!mPORTBReadBits(BIT_5) && !mPORTAReadBits(BIT_4));
          if(mPORTAReadBits(BIT_4)) x = 0;
          while(!mPORTBReadBits(BIT_5) && mPORTAReadBits(BIT_4)){
              while(mPORTAReadBits(BIT_4) && !mPORTBReadBits(BIT_7));
              pixels[x][y] = mPORTBReadBits(BIT_10) >> 2 | mPORTAReadBits(BIT_0) << 9 | mPORTAReadBits(BIT_1) << 9
  | mPORTBReadBits(BIT_3) << 8;
              while(mPORTAReadBits(BIT_4) && mPORTBReadBits(BIT_7));
              while(mPORTAReadBits(BIT_4) && !mPORTBReadBits(BIT_7));
              pixels[x][y]  |= mPORTBReadBits(BIT_10) >> 10 | mPORTAReadBits(BIT_0) << 1 | mPORTAReadBits(BIT_1) << 1
  | mPORTBReadBits(BIT_3);
              while(mPORTAReadBits(BIT_4) && mPORTBReadBits(BIT_7));
              while(mPORTAReadBits(BIT_4) && !mPORTBReadBits(BIT_7));
             pixels[x][y] |= mPORTBReadBits(BIT_4) << 8 | mPORTBReadBits(BIT_13) | mPORTAReadBits(BIT_2) << 12
  | mPORTBReadBits(BIT_15);
              while(mPORTAReadBits(BIT_4) && mPORTBReadBits(BIT_7));
              while(mPORTAReadBits(BIT_4) && !mPORTBReadBits(BIT_7));
              pixels[x][y] |= mPORTBReadBits(BIT_4) | mPORTBReadBits(BIT_13) >> 8 | mPORTAReadBits(BIT_2) << 4
  | mPORTBReadBits(BIT_15) >> 8;
              while(mPORTAReadBits(BIT_4) && mPORTBReadBits(BIT_7));
              x++;    
          }
          y++;
     }
      sumX = 0;
      sumY = 0;
      countX = 0;
      countY = 0;
      for(x = 0; x < 88; x++){
          for(y = 0; y < 145; y++){
              if(pixels[x][y] > 0xF800){
              //    tft_drawPixel(x,y,ILI9340_WHITE);
                  sumX +=x;
                  sumY +=y;
                 countX++;
                  countY++;
              }
              /*else{
                  tft_drawPixel(x,y,ILI9340_BLACK);
              }*/
              
          }
      }
      if(countX != 0 && countY != 0){
        medX[iterator] = sumX / countX;
        medY[iterator] = sumY / countY;
      }
      else{
          medX[iterator] = sumX;
          medY[iterator] = sumY;
      }
      
      if(checkDif(medX) && checkDif(medY)){
          //tft_drawCircle(medX[iterator], medY[iterator], 3, ILI9340_RED);
          trigger(medX[iterator], medY[iterator]);
      }
        else{
          mPORTBClearBits(BIT_2);
      }
     iterator = (iterator + 1) % 2;
      
  }
} // main

// === end  ======================================================

