// Sound.c
// Runs on MSPM0
// Sound assets in sounds/sounds.h
// your name
// your data 
#include <stdint.h>
#include <ti/devices/msp/msp.h>
//#include "Sound.h"
#include "../inc/DAC.h"
//#include "sounds/sounds.h"
#include "SUISounds/refereeWhistle.txt"
#include "SUISounds/kickEffect.txt"


#define BUS_HZ 80000000
#define SAMPLE_HZ 11025

static const uint16_t *wave;
static uint32_t wave_len;
static uint32_t wave_i;

void SysTick_IntArm(uint32_t period, uint32_t priority){
  SysTick->CTRL = 0;
  SysTick->LOAD = period - 1;
  SCB->SHP[1] = (SCB->SHP[1] & ~(0xC0000000)) | (priority << 30);
  SysTick->VAL = 0;
  SysTick->CTRL = 0x07;
}
// initialize a 11kHz SysTick, however no sound should be started
// initialize any global variables
// Initialize the 5-bit DAC
void Sound_Init(void){
  DAC_Init();
  wave = 0;
  wave_len = 0;
  wave_i = 0;
  SysTick->CTRL = 0;
  DAC_Out(2048);
}
void SysTick_Handler(void){ // called at 11 kHz
  if ((wave == 0) || (wave_i >= wave_len)){
    DAC_Out(2048);
    SysTick->CTRL = 0;
    wave = 0;
    return;
  }
  DAC_Out(wave[wave_i]);
  wave_i++;
  if (wave_i >= wave_len){
    DAC_Out(2048);
    SysTick->CTRL = 0;
    wave = 0;
  }
}

//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the SysTick interrupt.
// It starts the sound, and the SysTick ISR does the output
// feel free to change the parameters
// Sound should play once and stop
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Sound_Start(const uint16_t *pt, uint32_t count){
  if ((pt == 0) || (count == 0)){
    return;
  }
  __disable_irq();
  wave = pt;
  wave_len = count;
  wave_i = 0;
  __enable_irq();
  SysTick_IntArm(BUS_HZ / SAMPLE_HZ, 0);
}

void Sound_RefereeWhistle(void) {
  Sound_Start(refereeWhistle + 1936, 8144);
}

void Sound_Shoot(void){
  Sound_Start(kickEffect, 1800);
}
void Sound_Killed(void){
// write this
}
void Sound_Explosion(void){
// write this
}

void Sound_Fastinvader1(void){
// write this
}
void Sound_Fastinvader2(void){
// write this
}
void Sound_Fastinvader3(void){
// write this
}
void Sound_Fastinvader4(void){
// write this
}
void Sound_Highpitch(void){
// write this
}
