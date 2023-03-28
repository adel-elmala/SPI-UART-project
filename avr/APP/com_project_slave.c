#include "com_project_slave.h"
#include "../MCAL/DIO/DIO_Interface.h"
#include "../MCAL/DIO/DIO_REG.h"
#include "../MCAL/SPI/SPI_Interface.h"
// #include "../MCAL/TWI/TWI_Interface.h"
#include "../HAL/LCD/LCD_Interface.h"

#include "../LIB/Queue/Queue.h"

static SPI_Config_t spi_conf_slave = {
    true,           // bool enable_interrupt;
    true,           // bool enable_spi;
    false,          // bool is_master;
    false,          // bool double_speed;
    CLK_HIGH_IDLE,  // uint8 clk_polarity;
    SETUP_ON_FIRST, // uint8 clk_phase;
    CLK_DIV_128,    // uint8 clk_divisor;
    DORD_LSB_FIRST  // uint8 data_order;
};

void project_slave_init(void)
{
    lcd_init();

    SPI_Init(&spi_conf_slave);
}
void project_slave_app(void)
{
    // uint8 spi_buff[32] = {0};
    uint8 counter = 0;
    bool at_line_2 = false;
    while (1)
    {
        uint8 spi_data = SPI_Receive_Async();
        if (spi_data != QUEUE_ERROR_EMPTY)
        {
            if ((counter > 16))
            {
                if (at_line_2)
                {
                    lcd_clearAndHome();
                    counter = 0;
                }
                else
                {
                    lcd_goto_line2();
                    counter = 0;
                    at_line_2 = true;
                }
            }
            lcd_sendData(spi_data);
            counter++;
        }
    }
}