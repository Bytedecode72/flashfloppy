/*
 * led.c
 * 
 * Drive 4-digit display via TM1651 driver IC.
 * I2C-style serial protocol: DIO=PB10, CLK=PB11
 * 
 * Written & released by Keir Fraser <keir.xen@gmail.com>
 * 
 * This is free and unencumbered software released into the public domain.
 * See the file COPYING for more details, or visit <http://unlicense.org>.
 */

#include "stm32f10x.h"
#include "intrinsics.h"

#define DAT 10 /* PB10 */
#define CLK 11 /* PB11 */
#define write_clk(val) gpio_write_pin(gpiob, CLK, val)
#define write_dat(val) gpio_write_pin(gpiob, DAT, val)

static const uint8_t digits[] = {
    0x7e, 0x0c, 0xb6, 0x9e, 0xcc, 0xda, 0xfa, 0x0e, /* 0-7 */
    0xfe, 0xde, 0xee, 0xf8, 0x72, 0xbc, 0xf2, 0xe2, /* 8-f */
};

static void write(uint8_t x)
{
    unsigned int i;

    /* Data, LSB first */
    for (i = 0; i < 8; i++) {
        write_clk(0);
        write_dat(x&1);
        x >>= 1;
        delay_us(2);
        write_clk(1);
        delay_us(2);
    }

    /* Wait for ACK from TM1651 */
    write_clk(0);
    write_dat(1);
    write_clk(1);
    gpio_configure_pin(gpiob, DAT, GPI_pulled);
    for (i = 0; (i < 5) && gpio_read_pin(gpiob, DAT); i++)
        delay_us(1);

    gpio_configure_pin(gpiob, DAT, GPO_pushpull);
}

static void start(void)
{
    write_clk(1);
    write_dat(1);
    delay_us(2);
    write_dat(0);
    write_clk(0);
} 

static void stop(void)
{
    write_clk(0);
    write_dat(0);
    write_clk(1);
    write_dat(1);
}

void leds_write_hex(unsigned int x)
{
    start();
    write(0xc0);              /* set addr 0 */
    write(digits[(x>>8)&15]); /* dat0 */
    write(digits[(x>>4)&15]); /* dat1 */
    write(digits[(x>>0)&15]); /* dat2 */
    write(0x00);              /* dat3 */
    stop();
}

void leds_init(void)
{
    gpio_configure_pin(gpiob, DAT, GPO_pushpull);
    gpio_configure_pin(gpiob, CLK, GPO_pushpull);

    start();
    write(0x40); /* data cmd: write, auto-incr addr */
    stop();

    start();
    write(0x93); /* display ctrl: low brightness */
    stop();
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
