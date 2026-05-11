#include "stm32f0xx_hal.h"
#include <string.h>
#include <stdint.h>

#define MY_EMAIL   "thomasdvorak76@gmail.com" //set this to your email address before building


/* E-Paper */
#define EPD_CLK_PORT   GPIOB
#define EPD_CLK_PIN    GPIO_PIN_11

#define EPD_DIN_PORT   GPIOB
#define EPD_DIN_PIN    GPIO_PIN_15

#define EPD_CS_PORT    GPIOA
#define EPD_CS_PIN     GPIO_PIN_8

#define EPD_DC_PORT    GPIOB
#define EPD_DC_PIN     GPIO_PIN_10

#define EPD_RST_PORT   GPIOA
#define EPD_RST_PIN    GPIO_PIN_9

#define EPD_BSY_PORT   GPIOB
#define EPD_BSY_PIN    GPIO_PIN_13

/* ── NFC ── */
#define NTAG_ADDR      (0x55u << 1)   

/* Blue LED */
#define LED_B_PORT     GPIOB
#define LED_B_PIN      GPIO_PIN_12

#define EPD_W          250
#define EPD_H          122
#define EPD_BPR        16                  
#define EPD_BUFSZ      (EPD_W * EPD_BPR) 

static uint8_t g_fb[EPD_BUFSZ];


static const uint8_t FONT[5][16] = {
    /* T */ { 0xFF,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
              0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00 },
    /* o */ { 0x00,0x00,0x3C,0x66,0x66,0x66,0x66,0x66,
              0x66,0x3C,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* m */ { 0x00,0x00,0x6C,0x7E,0x76,0x66,0x66,0x66,
              0x66,0x66,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* a */ { 0x00,0x00,0x3E,0x60,0x60,0x7E,0x66,0x66,
              0x66,0x7E,0x00,0x00,0x00,0x00,0x00,0x00 },
    /* s */ { 0x00,0x00,0x3C,0x66,0x06,0x1C,0x38,0x60,
              0x66,0x3C,0x00,0x00,0x00,0x00,0x00,0x00 },
};
static I2C_HandleTypeDef hi2c1;


static void SystemClock_Config(void);
static void GPIO_Init(void);
static void I2C1_Init(void);

/* e-paper */
static void epd_spi_byte(uint8_t b);
static void epd_cmd(uint8_t c);
static void epd_dat(uint8_t d);
static void epd_busy_wait(void);
static void epd_hw_reset(void);
static void epd_init(void);
static void epd_clear_fb(uint8_t colour);   
static void epd_set_px(int x, int y);        
static void epd_draw_rect(int x, int y, int w, int h, int thick);
static void epd_draw_glyph(int x, int y, int gi, int sc);
static void epd_flush(void);
static void epd_sleep(void);

/* NFC */
static void ntag_write_ndef_email(const char *email);

/* Blue LED */
static void led_success_sequence(void);

/*  MAIN  */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    I2C1_Init();

    const int SC   = 3;
    const int GW   = 8  * SC;           
    const int GH   = 16 * SC;        
    const int GAP  = 3;
    const int tx   = (EPD_W - (5 * GW + 4 * GAP)) / 2; 
    const int ty   = (EPD_H - GH) / 2;              

    epd_clear_fb(0xFF);                 

    for (int i = 0; i < 5; i++)
        epd_draw_glyph(tx + i * (GW + GAP), ty, i, SC);

    epd_draw_rect(0, 0, EPD_W, EPD_H, 2);

    epd_init();
    epd_flush();
    epd_sleep();  

    ntag_write_ndef_email(MY_EMAIL);

    led_success_sequence();


    while (1)
        __WFI();  
}


static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
    osc.HSI48State     = RCC_HSI48_ON;
    osc.PLL.PLLState   = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) { while(1); }

    clk.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                       | RCC_CLOCKTYPE_PCLK1;
    clk.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI48;
    clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_1) != HAL_OK) { while(1); }
}


static void GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef g = {0};
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_HIGH;

    g.Pin = EPD_CLK_PIN;  HAL_GPIO_Init(EPD_CLK_PORT, &g);
    g.Pin = EPD_DIN_PIN;  HAL_GPIO_Init(EPD_DIN_PORT, &g);
    g.Pin = EPD_DC_PIN;   HAL_GPIO_Init(EPD_DC_PORT,  &g);

    g.Pin = EPD_CS_PIN;   HAL_GPIO_Init(EPD_CS_PORT,  &g);
    g.Pin = EPD_RST_PIN;  HAL_GPIO_Init(EPD_RST_PORT, &g);

    g.Pin = LED_B_PIN;    HAL_GPIO_Init(LED_B_PORT,   &g);

    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    g.Pin  = EPD_BSY_PIN; HAL_GPIO_Init(EPD_BSY_PORT, &g);

    g.Pin = GPIO_PIN_8;   HAL_GPIO_Init(GPIOB,        &g);

    HAL_GPIO_WritePin(EPD_CS_PORT,  EPD_CS_PIN,  GPIO_PIN_SET);   
    HAL_GPIO_WritePin(EPD_RST_PORT, EPD_RST_PIN, GPIO_PIN_SET);  
    HAL_GPIO_WritePin(EPD_CLK_PORT, EPD_CLK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_B_PORT,   LED_B_PIN,   GPIO_PIN_RESET); 
}

static void I2C1_Init(void)
{
    __HAL_RCC_I2C1_CLK_ENABLE();

    GPIO_InitTypeDef g = {0};
    g.Pin       = GPIO_PIN_6 | GPIO_PIN_7;
    g.Mode      = GPIO_MODE_AF_OD;
    g.Pull      = GPIO_NOPULL; 
    g.Speed     = GPIO_SPEED_FREQ_HIGH;
    g.Alternate = GPIO_AF1_I2C1;
    HAL_GPIO_Init(GPIOB, &g);

    hi2c1.Instance             = I2C1;
    hi2c1.Init.Timing          = 0x2000090E; 
    hi2c1.Init.OwnAddress1     = 0;
    hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) { while(1); }
}



static inline void epd_cs_low(void)
{ HAL_GPIO_WritePin(EPD_CS_PORT, EPD_CS_PIN, GPIO_PIN_RESET); }

static inline void epd_cs_high(void)
{ HAL_GPIO_WritePin(EPD_CS_PORT, EPD_CS_PIN, GPIO_PIN_SET); }

static void epd_spi_byte(uint8_t b)
{
  
    for (int8_t i = 7; i >= 0; i--) {
        HAL_GPIO_WritePin(EPD_CLK_PORT, EPD_CLK_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(EPD_DIN_PORT, EPD_DIN_PIN,
                          ((b >> i) & 1u) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(EPD_CLK_PORT, EPD_CLK_PIN, GPIO_PIN_SET);
    }
}

static void epd_cmd(uint8_t c)
{
    HAL_GPIO_WritePin(EPD_DC_PORT, EPD_DC_PIN, GPIO_PIN_RESET);
    epd_cs_low();
    epd_spi_byte(c);
    epd_cs_high();
}

static void epd_dat(uint8_t d)
{
    HAL_GPIO_WritePin(EPD_DC_PORT, EPD_DC_PIN, GPIO_PIN_SET);  
    epd_cs_low();
    epd_spi_byte(d);
    epd_cs_high();
}

static void epd_busy_wait(void)
{
    uint32_t t = HAL_GetTick();
    while (HAL_GPIO_ReadPin(EPD_BSY_PORT, EPD_BSY_PIN) == GPIO_PIN_SET) {
        if ((HAL_GetTick() - t) > 5000) break;  
        HAL_Delay(5);
    }
}

static void epd_hw_reset(void)
{
    HAL_GPIO_WritePin(EPD_RST_PORT, EPD_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(EPD_RST_PORT, EPD_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(EPD_RST_PORT, EPD_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(20);
}

static void epd_init(void)
{
    epd_hw_reset();

    epd_cmd(0x12);         
    HAL_Delay(10);
    epd_busy_wait();

    epd_cmd(0x01);
    epd_dat(0xF9);          
    epd_dat(0x00);      
    epd_dat(0x00);     

    epd_cmd(0x11);
    epd_dat(0x03);

    epd_cmd(0x44);
    epd_dat(0x00);
    epd_dat(EPD_BPR - 1); 

    epd_cmd(0x45);
    epd_dat(0x00); epd_dat(0x00);      
    epd_dat((EPD_W - 1) & 0xFF);      
    epd_dat((EPD_W - 1) >> 8);       

    epd_cmd(0x3C);
    epd_dat(0x05);

    epd_cmd(0x21);
    epd_dat(0x00);
    epd_dat(0x80);

    epd_cmd(0x18);
    epd_dat(0x80);

    epd_busy_wait();
}

static void epd_clear_fb(uint8_t colour)
{
    memset(g_fb, colour, EPD_BUFSZ);
}


static void epd_set_px(int x, int y)
{
    if ((unsigned)x >= EPD_W || (unsigned)y >= EPD_H) return;
    int idx = x * EPD_BPR + (y >> 3);
    int bit = 7 - (y & 7);
    g_fb[idx] &= ~(1u << bit);   
}

static void epd_draw_rect(int x, int y, int w, int h, int thick)
{
    for (int t = 0; t < thick; t++) {
        for (int i = x + t; i < x + w - t; i++) {
            epd_set_px(i, y + t);
            epd_set_px(i, y + h - 1 - t);
        }
        for (int j = y + t; j < y + h - t; j++) {
            epd_set_px(x + t,         j);
            epd_set_px(x + w - 1 - t, j);
        }
    }
}

static void epd_draw_glyph(int px, int py, int gi, int sc)
{
    for (int row = 0; row < 16; row++) {
        uint8_t bits = FONT[gi][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80u >> col)) {
                for (int sy = 0; sy < sc; sy++)
                    for (int sx = 0; sx < sc; sx++)
                        epd_set_px(px + col * sc + sx,
                                   py + row * sc + sy);
            }
        }
    }
}

static void epd_flush(void)
{
    epd_cmd(0x4E); epd_dat(0x00);
    epd_cmd(0x4F); epd_dat(0x00); epd_dat(0x00);
    epd_busy_wait();

    epd_cmd(0x24);
    for (uint32_t i = 0; i < EPD_BUFSZ; i++)
        epd_dat(g_fb[i]);
    epd_cmd(0x22);
    epd_dat(0xF7);
    epd_cmd(0x20);
    epd_busy_wait();
}

static void epd_sleep(void)
{
    epd_cmd(0x10);
    epd_dat(0x01);
    HAL_Delay(100);
}
/*  NFC  */
static void ntag_write_ndef_email(const char *email)
{
    const uint8_t elen        = (uint8_t)strlen(email);
    const uint8_t payload_len = 1u + elen;        
    const uint8_t ndef_len    = 4u + payload_len;  

    uint8_t msg[72];
    uint8_t pos = 0;

    msg[pos++] = 0x03;        
    msg[pos++] = ndef_len;      
    msg[pos++] = 0xD1;         
    msg[pos++] = 0x01;       
    msg[pos++] = payload_len;   
    msg[pos++] = 'U';           
    msg[pos++] = 0x06;          

    for (uint8_t i = 0; i < elen; i++)
        msg[pos++] = (uint8_t)email[i];

    msg[pos++] = 0xFE;      

    uint8_t blk = 0x01;
    uint8_t src = 0;

    while (src < pos) {
        uint8_t pkt[17];
        pkt[0] = blk;
        memset(&pkt[1], 0x00, 16);

        for (uint8_t i = 1; i <= 16 && src < pos; i++, src++)
            pkt[i] = msg[src];

        for (int retry = 0; retry < 3; retry++) {
            if (HAL_I2C_Master_Transmit(&hi2c1, NTAG_ADDR,
                                        pkt, 17, 50) == HAL_OK) break;
            HAL_Delay(2);
        }
        HAL_Delay(6);   
        blk++;
    }
}

static void led_success_sequence(void)
{
    for (int i = 0; i < 2; i++) {
        HAL_GPIO_WritePin(LED_B_PORT, LED_B_PIN, GPIO_PIN_SET);
        HAL_Delay(1000);
        HAL_GPIO_WritePin(LED_B_PORT, LED_B_PIN, GPIO_PIN_RESET);
        HAL_Delay(1000);
    }
    HAL_GPIO_WritePin(LED_B_PORT, LED_B_PIN, GPIO_PIN_SET);
}

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}
