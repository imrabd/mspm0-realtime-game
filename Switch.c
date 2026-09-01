/*
 * Switch.c
 *
 *  Created on: April 12, 2026
 *      Author: Imran Abdullah
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"

#define KICK  (1 << 17)
#define PAUSE (1 << 19)

// initialize your switches
void Switch_Init(void){
  // write this
  IOMUX->SECCFG.PINCM[PB17INDEX] = 0x00040081;
  IOMUX->SECCFG.PINCM[PB19INDEX] = 0x00040081;
  
  GPIOB->DOE31_0 &= ~(KICK | PAUSE);
}

// return current state of switches
uint32_t Switch_In(void){
  // write this
  uint32_t current = GPIOB->DIN31_0;
  uint32_t result = 0;
  if ((current & KICK) != 0){ //kick is pb17
    result |= 1;
  }
  if ((current & PAUSE) != 0){ //pause or other is pb19
    result |= 2;
  }
  return result;
}
