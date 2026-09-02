// Lab9Main.c
// Runs on MSPM0G3507
// Lab 9 ECE319K
// Imran Abdullah
// Last Modified: April 20, 2026

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/UART.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/ADC1.h"
#include "../inc/Arabic.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
//#include "images/images.h"
#include "images/SUISprites.h"

// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

Arabic_t ArabicAlphabet[]={
alif,ayh,baa,daad,daal,dhaa,dhaal,faa,ghayh,haa,ha,jeem,kaaf,khaa,laam,meem,noon,qaaf,raa,saad,seen,sheen,ta,thaa,twe,waaw,yaa,zaa,space,dot,null
};
Arabic_t Hello[]={alif,baa,ha,raa,meem,null}; // hello
Arabic_t WeAreHonoredByYourPresence[]={alif,noon,waaw,ta,faa,raa,sheen,null}; // we are honored by your presence
int main0(void){ // main 0, demonstrate Arabic output
  Clock_Init80MHz(0);
  LaunchPad_Init();
  ST7735_InitR(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  ST7735_FillScreen(ST7735_WHITE);
  Arabic_SetCursor(0,15);
  Arabic_OutString(Hello);
  Arabic_SetCursor(0,31);
  Arabic_OutString(WeAreHonoredByYourPresence);
  Arabic_SetCursor(0,63);
  Arabic_OutString(ArabicAlphabet);
  while(1){
  }
}
uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}

int32_t goalkeeperX = 53;
int32_t goalkeeperVel = 3;
int32_t prevGoalkeeperX;
int32_t prevPlayerX;
int32_t prevBallX;
int32_t prevBallY;
int32_t prevGoalTargetX;
int32_t prevGoalTargetY;
int32_t drawPrevGoalkeeper;
int32_t drawPrevPlayer;
int32_t drawPrevBall;
int32_t drawPrevGoalTarget;
volatile int32_t goalkeeperMove = 1;

void goalkeeperPatrol(void){
  goalkeeperX += goalkeeperVel;
  if(goalkeeperX <= 28){
    goalkeeperX = 32;
    goalkeeperVel = 3;
  }
  if(goalkeeperX >= 72){
    goalkeeperX = 68;
    goalkeeperVel = -3;
  }
}

int32_t ballX = 59;
int32_t ballY = 136;
int32_t ballInFlight = 0;

int32_t numGoals = 0;
int32_t numShots = 0;

int32_t goalTargetX = 50;
/* Bottom row of sprite sits on goal mouth (see ballMovement stop at ballY < 70). */
int32_t goalTargetY = 70;

void placeNewGoalTarget(void){
  int32_t minX = 20;
  int32_t maxX = 100 - TARGETSPRITE_WIDTH;
  if(maxX < minX){
    maxX = minX;
  }
  goalTargetX = minX + Random((maxX - minX + 1));
  goalTargetY = 70;
}

void shotProcessAndReset(void){
  int32_t pastKeeper = (ballX > (goalkeeperX + 17) && ballX < 112) || (ballX < (goalkeeperX - 5) && ballX > 16);
  int32_t ballRight = ballX + 12 - 1;
  int32_t ballTop = ballY - 12 + 1;
  int32_t targetRight = goalTargetX + TARGETSPRITE_WIDTH - 1;
  int32_t targetTop = goalTargetY - TARGETSPRITE_HEIGHT + 1;
  int32_t hitsTarget = (ballX <= targetRight) && (ballRight >= goalTargetX) && (ballTop <= goalTargetY) && (ballY >= targetTop);
  if(pastKeeper && hitsTarget){
    numGoals++;
  }
  numShots++;

  Clock_Delay1ms(1000);

  goalkeeperX = 53;
  ballX = 59;
  ballY = 136;
  goalkeeperMove = 1;
  placeNewGoalTarget();
}

void ballMovement(int32_t ballChangeX){
  ballY -= 10;
  ballX += ballChangeX;
  if(ballY < 70){
    ballInFlight = 0;
    goalkeeperMove = 0;
    shotProcessAndReset();
  }
}



volatile uint32_t frame_flag;
volatile uint32_t frame_slidePot;
volatile int32_t frame_goalkeeperX;
volatile uint32_t frame_switchBits;
volatile uint8_t playTimerArmed;
volatile uint32_t playTimerFramesLeft;

// games engine runs at 30Hz
void TIMG12_IRQHandler(void){uint32_t slidePotSample, switchBits;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    // game engine goes here
    // 1) sample slide pot
    slidePotSample = ADCin();
    // 2) read input switches
    switchBits = Switch_In();
    if((playTimerArmed != 0) && (playTimerFramesLeft > 0)){
      playTimerFramesLeft = playTimerFramesLeft - 1;
    }
    // 3) move sprites
    if(goalkeeperMove){
      goalkeeperPatrol();
    }
    frame_slidePot = slidePotSample;
    frame_goalkeeperX = goalkeeperX;
    frame_switchBits = switchBits;
    // 4) start sounds
    // 5) set semaphore
    frame_flag = 1;
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
}
uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
char Hello_English[] ="Hello";
char Hello_Spanish[] ="\xADHola!";
char Hello_Portuguese[] = "Ol\xA0";
char Hello_French[] ="All\x83";
char Goodbye_English[]="Goodbye";
char Goodbye_Spanish[]="Adi\xA2s";
char Goodbye_Portuguese[] = "Tchau";
char Goodbye_French[] = "Au revoir";
char Language_English[]="English";
char Language_Spanish[]="Espa\xA4ol";
char Language_Portuguese[]="Portugu\x88s";
char Language_French[]="Fran\x87" "ais";
char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};
// use main1 to observe special characters
int main1(void){ // main1
    char l;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  ST7735_FillScreen(0x0000);            // set screen to black
  for(phrase_t myPhrase=HELLO; myPhrase<= GOODBYE; myPhrase++){
    for(Language_t myL=English; myL<= French; myL++){
         ST7735_OutString(Phrases[LANGUAGE][myL]);
      ST7735_OutChar(' ');
         ST7735_OutString(Phrases[myPhrase][myL]);
      ST7735_OutChar(13);
    }
  }
  Clock_Delay1ms(3000);
  ST7735_FillScreen(0x0000);       // set screen to black
  l = 128;
  while(1){
    Clock_Delay1ms(2000);
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  }
}

/*
// use main2 to observe graphics
int main2(void){ // main2
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_DrawBitmap(22, 159, PlayerShip0, 18,8); // player ship bottom
  ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
  ST7735_DrawBitmap(42, 159, PlayerShip1, 18,8); // player ship bottom
  ST7735_DrawBitmap(62, 159, PlayerShip2, 18,8); // player ship bottom
  ST7735_DrawBitmap(82, 159, PlayerShip3, 18,8); // player ship bottom
  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);

  for(uint32_t t=500;t>0;t=t-5){
    SmallFont_OutVertical(t,104,6); // top left
    Clock_Delay1ms(50);              // delay 50 msec
  }
  ST7735_FillScreen(0x0000);   // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString("GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_OutString("Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("Earthling!");
  ST7735_SetCursor(2, 4);
  ST7735_OutUDec(1234);
  while(1){
  }
}
*/

// use main3 to test switches and LEDs
int main3(void){ // main3
  uint32_t last,now;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ADCinit();
  UART_Init();
  Switch_Init(); // initialize switches
  UART_OutString("Lab9 main3 UART OK\r\n");
  last = Switch_In();
  UART_OutString("Switch= 0x"); UART_OutUHex(last); UART_OutString("\r\n");
  while(1){
    // write code to test switches and LEDs
    now = Switch_In(); // Your Lab4 input
    if(now != last){ // change
      UART_OutString("Switch= 0x"); UART_OutUHex(now); UART_OutString("\n\r");
    }
    last = now;
    Clock_Delay(800000); // 10ms, to debounce switch
  }
}
/*
// use main4 to test sound outputs
int main4(void){ uint32_t last=0,now;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
  __enable_irq();
  while(1){
    now = Switch_In(); // one of your buttons
    if((last == 0)&&(now == 1)){
      Sound_Shoot(); // call one of your sounds
    }
    if((last == 0)&&(now == 2)){
      Sound_Killed(); // call one of your sounds
    }
    if((last == 0)&&(now == 4)){
      Sound_Explosion(); // call one of your sounds
    }
    if((last == 0)&&(now == 8)){
      Sound_Fastinvader1(); // call one of your sounds
    }
    // modify this to test all your sounds
  }
}
*/

void drawCompositeSprite(int32_t x, int32_t y, const uint16_t *spriteImage, int32_t spriteW, int32_t spriteH, int32_t *prevX, int32_t *drawPrev, int32_t opaqueOnly, int32_t *prevY){
  int32_t topY = y - spriteH + 1;
  int32_t rowTop = topY;
  int32_t rowBottom = y;
  int32_t xLeft = x;
  int32_t xRight = x + spriteW - 1;
  int32_t priorX;
  int32_t priorBottomY;
  int32_t screenY;
  int32_t screenX;
  uint32_t bgRowOffset;
  int32_t localX;
  int32_t localY;
  uint16_t fg;
  uint16_t bg;
  if(*drawPrev){
    priorX = *prevX;
    if(priorX < xLeft){
      xLeft = priorX;
    }
    if(priorX + spriteW - 1 > xRight){
      xRight = priorX + spriteW - 1;
    }
    if(prevY != NULL){
      priorBottomY = *prevY;
      if(priorBottomY - spriteH + 1 < rowTop){
        rowTop = priorBottomY - spriteH + 1;
      }
      if(priorBottomY > rowBottom){
        rowBottom = priorBottomY;
      }
    }
  }
  if(xLeft < 0){
    xLeft = 0;
  }
  if(xRight >= 128){
    xRight = 127;
  }
  for(screenY = rowTop; screenY <= rowBottom; screenY = screenY + 1){
    if(screenY < 0 || screenY >= 160){
      continue;
    }
    bgRowOffset = (159 - screenY) * 128;
    for(screenX = xLeft; screenX <= xRight; screenX = screenX + 1){
      localX = screenX - x;
      localY = screenY - topY;
      fg = 0;
      if(localX >= 0 && localX < spriteW && localY >= 0 && localY < spriteH){
        fg = spriteImage[((spriteH - 1) - localY) * spriteW + localX];
      }
      bg = background[bgRowOffset + screenX];
      if(opaqueOnly){
        if(fg){
          ST7735_DrawPixel(screenX, screenY, fg);
        }
      } else {
        if(fg){
          ST7735_DrawPixel(screenX, screenY, fg);
        } else {
          ST7735_DrawPixel(screenX, screenY, bg);
        }
      }
    }
  }
  *prevX = x;
  if(prevY != NULL){
    *prevY = y;
  }
  *drawPrev = 1;
}

bool inPortuguese = false;

void drawMenuButtonsAndLabels(void){
  int i;
  char *s;
  ST7735_DrawBitmap(3, 144, buttons, 24, 52);
  if(inPortuguese){
    s = "Iniciar jogo";
    i = 0;
    while(s[i] != 0){
      ST7735_DrawChar(30 + 6 * i, 128, s[i], ST7735_WHITE, ST7735_BLACK, 1);
      i = i + 1;
    }
    s = "Trocar Idioma  ";
    i = 0;
    while(s[i] != 0){
      ST7735_DrawChar(30 + 6 * i, 101, s[i], ST7735_WHITE, ST7735_BLACK, 1);
      i = i + 1;
    }
  } else {
    s = "Start Game  ";
    i = 0;
    while(s[i] != 0){
      ST7735_DrawChar(30 + 6 * i, 128, s[i], ST7735_WHITE, ST7735_BLACK, 1);
      i = i + 1;
    }
    s = "Switch Language";
    i = 0;
    while(s[i] != 0){
      ST7735_DrawChar(30 + 6 * i, 101, s[i], ST7735_WHITE, ST7735_BLACK, 1);
      i = i + 1;
    }
  }
}

// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
typedef enum { GAME_STATE_MENU, GAME_STATE_PLAY, GAME_STATE_GAMEOVER } game_state_t;

#define PLAY_TIME_FRAMES 900

int main(void){ // final main
  game_state_t gameState = GAME_STATE_MENU;
  uint32_t prevSwitchBits = 0;
  uint8_t menuNeedsDraw = 1;
  uint8_t menuNeedButtonsOnly = 0;
  uint8_t gameOverNeedsDraw = 0;
  int32_t ballChangeX = 0;
  uint32_t game_over_phase = 0;
  int8_t game_over_dx[] = {
    -3, -2, -1, 0, 1, 2, 3, 2, 1, 0, -1, -2
  };
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  ST7735_FillScreen(ST7735_BLACK);
  ADCinit();     //PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
  TimerG12_IntArm(80000000/30,2);
  // initialize all data structures
  __enable_irq();

  while(1){
    // wait for semaphore
    while(frame_flag == 0) { }
       // clear semaphore
      frame_flag = 0;
      if(gameState == GAME_STATE_MENU){
        if(((frame_switchBits & 1) != 0) && ((prevSwitchBits & 1) == 0)){
          Sound_RefereeWhistle();
          gameState = GAME_STATE_PLAY;
          ST7735_SetTextColor(ST7735_YELLOW);
          ST7735_DrawBitmap(0, 159, background, 128, 160);
          placeNewGoalTarget();
          prevGoalkeeperX = 0;
          prevPlayerX = 0;
          prevBallX = 0;
          prevBallY = 0;
          prevGoalTargetX = 0;
          prevGoalTargetY = 0;
          drawPrevGoalkeeper = 0;
          drawPrevPlayer = 0;
          drawPrevBall = 0;
          drawPrevGoalTarget = 0;
          goalkeeperMove = 1;
          playTimerFramesLeft = PLAY_TIME_FRAMES;
          playTimerArmed = 1;
        }
        if(((frame_switchBits & 2) != 0) && ((prevSwitchBits & 2) == 0)){
          inPortuguese = !inPortuguese;
          menuNeedButtonsOnly = 1;
        }
      } else if(gameState == GAME_STATE_GAMEOVER){
        if((((frame_switchBits & 1) != 0) && ((prevSwitchBits & 1) == 0)) ||
           (((frame_switchBits & 2) != 0) && ((prevSwitchBits & 2) == 0))){
          gameState = GAME_STATE_MENU;
          menuNeedsDraw = 1;
          ballInFlight = 0;
          ballX = 59;
          ballY = 136;
          goalkeeperMove = 1;
          numGoals = 0;
          numShots = 0;
        }
      } else {
        if((playTimerArmed != 0) && (playTimerFramesLeft == 0)){
          gameState = GAME_STATE_GAMEOVER;
          playTimerArmed = 0;
          gameOverNeedsDraw = 1;
          ballInFlight = 0;
          goalkeeperMove = 0;
        } else {
          if(((frame_switchBits & 2) != 0) && ((prevSwitchBits & 2) == 0)){
            playTimerArmed = 0;
            gameState = GAME_STATE_MENU;
            menuNeedsDraw = 1;
            ballInFlight = 0;
            ballX = 59;
            ballY = 136;
            goalkeeperMove = 1;
            numGoals = 0;
            numShots = 0;
          }
          if(((frame_switchBits & 1) != 0) && ((prevSwitchBits & 1) == 0)){
            if(ballInFlight == 0){
              int32_t direction = ADCin();
              ballChangeX = ((direction * 19) / 4095) - 9;
              ballInFlight = 1;
              goalkeeperMove = 1;
              Sound_Shoot();
            }
          }
          if(ballInFlight) {
            ballMovement(ballChangeX);
          }
        }
      }
      prevSwitchBits = frame_switchBits;
      // update ST7735R
      if(gameState == GAME_STATE_MENU){
        if(menuNeedsDraw){
          ST7735_FillScreen(ST7735_BLACK);
          ST7735_DrawBitmap(7, 83, SUILogo, 112, 79);
          menuNeedsDraw = 0;
          menuNeedButtonsOnly = 0;
        }
        drawMenuButtonsAndLabels();
      } else if(gameState == GAME_STATE_GAMEOVER){
        if(gameOverNeedsDraw != 0){
          ST7735_FillScreen(ST7735_BLACK);
          ST7735_SetTextColor(ST7735_WHITE);
          ST7735_SetCursor(7, 9);
          if(inPortuguese){
            ST7735_OutString("Placar: ");
            ST7735_OutUDec(numGoals);
            ST7735_SetCursor(4, 12);
            ST7735_OutString("Pressione Para ");
            ST7735_SetCursor(6, 13);
            ST7735_OutString("Continuar");
          } else {
            ST7735_OutString("Score: ");
            ST7735_OutUDec(numGoals);
            ST7735_SetCursor(7, 12);
            ST7735_OutString("Press to ");
            ST7735_SetCursor(7, 13);
            ST7735_OutString("Continue");
          }
          gameOverNeedsDraw = 0;
          game_over_phase = 0;
        }
        {
          int8_t dx;
          int16_t x;
          int16_t y_top;
          dx = game_over_dx[game_over_phase % 12];
          game_over_phase = game_over_phase + 1;
          x = 18 + dx;
          y_top = 70 - GAMEOVERSPRITE_HEIGHT + 1;
          ST7735_FillRect(18 - 3, y_top, GAMEOVERSPRITE_WIDTH + 6, GAMEOVERSPRITE_HEIGHT, ST7735_BLACK);
          if(inPortuguese){
            ST7735_DrawBitmap(x, 70, gameOverPortuguese, GAMEOVERSPRITE_WIDTH, GAMEOVERSPRITE_HEIGHT);
          } else {
            ST7735_DrawBitmap(x, 70, gameOverSprite, GAMEOVERSPRITE_WIDTH, GAMEOVERSPRITE_HEIGHT);
          }
        }
      } else {
        drawCompositeSprite(frame_goalkeeperX, 95, goalkeeper, 32, 48, &prevGoalkeeperX, &drawPrevGoalkeeper, 0, NULL);
        drawCompositeSprite(12, 141, playerSprite, 47, 56, &prevPlayerX, &drawPrevPlayer, 1, NULL);
        drawCompositeSprite(ballX, ballY, ballSprite, 12, 12, &prevBallX, &drawPrevBall, 0, &prevBallY);
        drawCompositeSprite(goalTargetX, goalTargetY, targetSprite, TARGETSPRITE_WIDTH, TARGETSPRITE_HEIGHT, &prevGoalTargetX, &drawPrevGoalTarget, 0, &prevGoalTargetY);
        ST7735_SetCursor(0, 0);
        if(inPortuguese){
          ST7735_OutString("Chutes: ");
        } else {
          ST7735_OutString("Shots: ");
        }
        ST7735_OutUDec(numShots);
        ST7735_SetCursor(0, 1);
        if(inPortuguese){
          ST7735_OutString("Gols: ");
        } else {
          ST7735_OutString("Goals: ");
        }
        ST7735_OutUDec(numGoals);
        {
          uint32_t sec = (playTimerFramesLeft + 29) / 30;
          ST7735_SetCursor(18, 0);
          if(sec < 10){
            ST7735_OutString(" ");
          }
          ST7735_OutUDec(sec);
        }
      }
    // check for end game or level switch
  }
}
