/**
*  @file   Video.c
*  @author Rob Gee
*  @date   November 2017
*  @brief  compositite video signal generator, for STM32
*/

/***** Include files  *********************************************/
#include "stm32f10x.h"
#include "bsp.h"
#include "Video.h"

/***** Constants  *************************************************/

/*   PAL constants      */
#define SCANLINE_US        (1000000UL/(625UL * (FRAME_RATE/2)))
#define SCANFREQ           ((1000000UL)/SCANLINE_US)        /* line frequency */

/* NB the following times all in ns */
#define H_SYNC             4700UL
#define LONG_SYNC_GAP      (H_SYNC)
#define LONG_SYNC          ((SCANLINE_US*500UL)-LONG_SYNC_GAP)
#define SHORT_SYNC         (H_SYNC/2UL)
#define BACK_PORCH         5700UL
#define FRONT_PORCH        1650UL
#define BACK_PORCH         5700UL
#define TEXT_OFFSET        3400UL
//#define TEXT_WIDTH         45715UL
#define TEXT_WIDTH         (((NUM_X_PIXELS*1000000UL)+(SPI_CLOCK/2000UL))/(SPI_CLOCK/1000UL))
#define TEXT_START         (BACK_PORCH+TEXT_OFFSET)
#define TEXT_END           (TEXT_START+TEXT_WIDTH)
#define FIRST_ACTIVE_LINE  6U          /* Inbetween these are teletext data */
#define NUM_LINES          NUM_Y_PIXELS
#define DISP_START_LINE    23U
#define TEXT_START_LINE    49U
#define TEXT_END_LINE      (TEXT_START_LINE+NUM_LINES-1U)
#define DISP_END_LINE      309U
#define LAST_ACTIVE_LINE   309U

/* NB During frame sync and short sync sections, increase PWM freq to 2X line freq   */
#define NUM_BROAD_SYNC        5U
#define NUM_PRE_FRAME_SYNC    5U
#define NUM_POST_FRAME_SYNC   6U

/* STM32 timing constants */
#define AHB2_CLOCK      56000000UL
#define AHB1_CLOCK      28000000UL
#define TIMER_PSC       7UL
#define TIMER_CLOCK     (AHB2_CLOCK/TIMER_PSC)
#define TIMER_ARR       (TIMER_CLOCK/SCANFREQ)
#define TIMER_ARR_SYNC  (TIMER_ARR/2UL)
#define TIMER_TICK      ((1000000000UL)/TIMER_CLOCK)
#define SPI_PSC         4
#define SPI_CLOCK       (AHB1_CLOCK/SPI_PSC)

#define TIMER_TICKS(ns) (((ns)+(TIMER_TICK/2))/TIMER_TICK)

/* STM32 peripheral constants  */
#define DMA_CHAN_SPI2_TX   DMA1_Channel5

/* Test patterns  */
//#define TEST_PATTERN_ACTIVE
#define TEST_HATCH_SIZE    20

#define TIMING_TEST

/***** Types      *************************************************/
typedef enum {FRAME_SYNC, PRE_FRAME_SHORT, FRAME_ACTIVE, POST_FRAME_SHORT} Sync_State_e;

/***** Storage    *************************************************/
uint8_t FrameBuff[NUM_Y_PIXELS][NUM_X_BYTES] = {0};      /* Extra zero byte at end */

static void (*pVerticalBlankingCallback)(uint8_t) = 0;

/***** Local prototypes    ****************************************/
static void GPIO_Configuration(void);
static void TIM_Configuration(void);
static void NVIC_Configuration(void);
static void SPI_Configuration(void);
static void DMA_Configuration(void);
static void TriggerLine(uint16_t line);
#ifdef TEST_PATTERN_ACTIVE
static void GenerateScreenTest(void);
#endif
#ifdef TEST
static void SPI_Test(void);
#endif


/***** Exported functions  ****************************************/
/**
*  @fn     VideoInit
*  @brief  Initialises Video generator
*/
void VideoInit(void)
{
   #ifdef TEST
   volatile uint32_t test;
   test = TEXT_WIDTH;
   test = NUM_LINES;
   test = SPI_CLOCK;
   test = TEXT_WIDTH;
   test = NUM_X_PIXELS;
   #endif

   #ifdef TEST_PATTERN_ACTIVE
   GenerateScreenTest();
   #endif

   GPIO_Configuration();
   NVIC_Configuration();
   SPI_Configuration();

   DMA_Configuration();
   TIM_Configuration();

}

/**
*  @fn     setVerticalBlankingCallback
*  @brief  Sets call back function for vertical blanking event
*/
void setVerticalBlankingCallback(void (*pCallback)(uint8_t))
{
   pVerticalBlankingCallback = pCallback;
}

/***** Local    functions  ****************************************/

/**
*  @fn     TIM1_CC_IRQHandler
*  @brief  Interrupt handler for timer CC event
*/
void TIM1_CC_IRQHandler(void)
{    
   static Sync_State_e sync_state = FRAME_SYNC;
   static uint16_t scan_line_count;
   static uint16_t display_line_count;
   static uint16_t pulse_count = 0;

//   DBGMCU->CR |= DBGMCU_CR_DBG_TIM1_STOP;    /* stop timer 1 in debug mode */

   if(TIM_GetITStatus(TIM1 , TIM_IT_CC1) != RESET)
   {
      TIM_ClearITPendingBit(TIM1 , TIM_FLAG_CC1);
      switch(sync_state)
      {
         case FRAME_SYNC:
            if(++pulse_count >= NUM_BROAD_SYNC)
            {
               TIM1->CCR1 = TIMER_TICKS(SHORT_SYNC);
               sync_state = PRE_FRAME_SHORT;
               pulse_count = 0;
            }
         break;

         case PRE_FRAME_SHORT:
            if(++pulse_count >= NUM_PRE_FRAME_SYNC)
            {
               TIM1->CCR1 = TIMER_TICKS(H_SYNC);
               TIM1->ARR = TIMER_ARR;
               sync_state = FRAME_ACTIVE;
               scan_line_count = FIRST_ACTIVE_LINE;
               display_line_count = 0;
            }
         break;

         case FRAME_ACTIVE:
            if(scan_line_count >= LAST_ACTIVE_LINE)
            {
               TIM1->CCR1 = TIMER_TICKS(SHORT_SYNC);
               TIM1->ARR = TIMER_ARR_SYNC;
               pulse_count = 0;
               sync_state = POST_FRAME_SHORT;
            }
            /* Placemarker for vertical blanking interval - with present numbers we can have
               72 lines or 4.6ms */
            else if(scan_line_count == (TEXT_START_LINE-1))
            {
               /* TODO add call back for end of blanking interval  */
               if(pVerticalBlankingCallback != 0)
               {
                  pVerticalBlankingCallback(0);
                  #ifdef TIMING_TEST
                  GPIO_SetBits(DEBUG_PORT, DEBUG_PIN_2);
                  #endif
               }
            }
            else if(scan_line_count == (TEXT_END_LINE+1))
            {
               /* TODO add callback for start of blanking interval */
               if(pVerticalBlankingCallback != 0)
               {
                  pVerticalBlankingCallback(1);
                  #ifdef TIMING_TEST
                  GPIO_ResetBits(DEBUG_PORT, DEBUG_PIN_2);
                  #endif
               }
            }
            scan_line_count++;
         break;

         case POST_FRAME_SHORT:
            if(++pulse_count >= NUM_POST_FRAME_SYNC)
            {
               TIM1->CCR1 = TIMER_TICKS(LONG_SYNC);
               sync_state = FRAME_SYNC;
               pulse_count = 0;
            }
         break;
      }
   }
   else if(TIM_GetITStatus(TIM1 , TIM_IT_CC2) != RESET)
   {
      TIM_ClearITPendingBit(TIM1 , TIM_FLAG_CC2);
      if((scan_line_count >= TEXT_START_LINE) && (scan_line_count <= TEXT_END_LINE))
      {
         TriggerLine(display_line_count++);
      }

   }
}


/**
*  @fn     TIM1_CC_IRQHandler
*  @brief  Interrupt handler for timer CC event
*/
void DMA1_Channel5_IRQHandler(void)
{
   if(DMA_GetITStatus(DMA1_IT_TC5) != RESET)
   {
      DMA_ClearITPendingBit(DMA1_IT_TC5);
      DMA_Cmd(DMA_CHAN_SPI2_TX, DISABLE);
      while((SPI2->SR & SPI_SR_TXE) == 0);
      SPI2->DR = 0;
   }
}

/**
*  @fn     GPIO_Configuration
*  @brief  Set up sync output
*/
static void GPIO_Configuration(void)
{
   GPIO_InitTypeDef GPIO_InitStructure = {0};

   RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE); 						 

  /* Timer1 channel 1 PA8  */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; ; 
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   /* SPI2 MOSI - PB15     */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; ; 
   GPIO_Init(GPIOB, &GPIO_InitStructure);

}


/**
*  @fn     TIM_Configuration
*  @brief  Configure timer used for sync generation
*/
static void TIM_Configuration(void)
{
   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure = {0};
   TIM_OCInitTypeDef        TIM_OCInitStructure = {0};
   
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 , ENABLE);
   TIM_DeInit(TIM1);
   TIM_TimeBaseStructure.TIM_Period=TIMER_ARR_SYNC;
   												
   TIM_TimeBaseStructure.TIM_Prescaler= TIMER_PSC-1;
   TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
   TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
   TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
   TIM_ClearFlag(TIM1, TIM_FLAG_Update);						
   TIM_ITConfig(TIM1,(TIM_IT_CC1 | TIM_IT_CC2),ENABLE);
   
   TIM_ARRPreloadConfig(TIM1, ENABLE);
   
   /* Enable PWM = Channel 1 generates sync */
   TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
   TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
   TIM_OCInitStructure.TIM_Pulse = TIMER_TICKS(LONG_SYNC);
   TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
   
   TIM_OC1Init(TIM1, &TIM_OCInitStructure);
   TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

   /* PWM channel 2 generates DMA trigger for pixel data */
   TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
   TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
   TIM_OCInitStructure.TIM_Pulse = TIMER_TICKS(TEXT_START);
 
   TIM_OC2Init(TIM1, &TIM_OCInitStructure);
   TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);

   /* Configure DMA trigger, on CCR2  */
//   TIM_DMAConfig(TIM1, TIM_DMABase_CCR2, TIM_DMABurstLength_1Byte);
//   TIM_DMACmd(TIM1, TIM_DMA_CC2, ENABLE);
   
   TIM_CtrlPWMOutputs(TIM1, ENABLE);
   TIM_Cmd(TIM1, ENABLE);			
}

/**
*  @fn     NVIC_Configuration
*  @brief  Configure NVIC
*/
static void NVIC_Configuration(void)
{
   NVIC_InitTypeDef NVIC_InitStructure; 
    
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  													
   NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;	  
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
   
   NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
   NVIC_Init(&NVIC_InitStructure);
}

/**
*  @fn     SPI_Configuration
*  @brief  Configure SPI
*/
static void SPI_Configuration(void)
{
   SPI_InitTypeDef  SPI_InitStructure;

   RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2 , ENABLE);

   SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
   SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
   SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
   SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
   SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
   SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
   SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
   SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
   SPI_InitStructure.SPI_CRCPolynomial = 7;
   SPI_Init(SPI2, &SPI_InitStructure);

   /* Enable SPI2 Tx request */
   SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

   /* Enable SPI2  */
   SPI_Cmd(SPI2, ENABLE);
}

/**
*  @fn     DMA_Configuration
*  @brief  Configure DMA
*/
static void DMA_Configuration(void)
{
   DMA_InitTypeDef  DMA_InitStructure;

   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);

   DMA_DeInit(DMA_CHAN_SPI2_TX);
   DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI2->DR;
   DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)FrameBuff;
   DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
   DMA_InitStructure.DMA_BufferSize = NUM_X_BYTES;
   DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
   DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
   DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
   DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
   DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
   DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
   DMA_ITConfig(DMA_CHAN_SPI2_TX, DMA_CCR5_TCIE, ENABLE); 
   DMA_Init(DMA_CHAN_SPI2_TX, &DMA_InitStructure);
}


/**
*  @fn        TriggerLine
*  @brief     Initiates output of a scan line
*  @param[IN] line number
*/
static void TriggerLine(uint16_t line)
{
   DMA_CHAN_SPI2_TX->CMAR = (uint32_t)&FrameBuff[line][0];
   DMA_CHAN_SPI2_TX->CNDTR = NUM_X_BYTES;
   DMA_Cmd(DMA_CHAN_SPI2_TX, ENABLE);
   SPI2->DR = FrameBuff[line][0];
}

#ifdef TEST_PATTERN_ACTIVE
/**
*  @fn        GenerateScreenTest
*  @brief     Generates cross hatch pattern
*/
static void GenerateScreenTest(void)
{
   uint16_t x,y;
   uint8_t byte;
 
   for(y = 0; y < NUM_Y_PIXELS; y++)
   {
      for(x = 0; x < NUM_X_BYTES; x++)
      {
         if( ((y % 16) == 0) || (y == NUM_Y_PIXELS-1))
         {
            byte = 0xf0;
         }
         else if (x == NUM_X_BYTES-1)
         {
//            byte = 0x01;
            byte = 0xF0;
         }
         else if( ((x % 2) == 0) || (x == 0))
         {
//            byte = 0x80;   
            byte = 0xF0;   
         }
         else
         {
            byte = 0;
         }
         FrameBuff[y][x] = byte;
      }
   }
}
#endif

#ifdef TEST
/**
*  @fn        SPI_Test
*  @brief     Sends a single line directly via SPI
*/
static void SPI_Test(void)
{
   uint8_t i;

   for(i = 0; i < NUM_X_BYTES; i++)
   {
      /* Wait for SPI2 Tx buffer empty */
      while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
      /* Send SPI2 data */
      SPI_I2S_SendData(SPI2, FrameBuff[0][i]);
      
   }
}
#endif




