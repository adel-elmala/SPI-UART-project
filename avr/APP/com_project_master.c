#include "com_project_master.h"
#include "../MCAL/DIO/DIO_Interface.h"
#include "../MCAL/DIO/DIO_REG.h"
#include "../MCAL/EXT_INT/External_INT_Interface.h"
#include "../MCAL/UART/UART_Interface.h"
#include "../MCAL/SPI/SPI_Interface.h"
// #include "../MCAL/TWI/TWI_Interface.h"

#include "../LIB/Queue/Queue.h"

UART_Config_t uart_conf_master = {

    9600,                                           //     uint16 baud_rate;
    UART_DISABLE_DOUBLE_TRANSMISSION_SPEED,         //     uint8 enable_double_transmission_speed;
    UART_DISABLE_MULTIPROCESSOR_COMMUNICATION_MODE, //     uint8 enable_multiprocessor_mode;
    UART_ENABLE_RECEIVE_COMPLETE_INTERRUPT,         //     uint8 enable_recevie_complete_interrupt;
    UART_ENABLE_TRANSMIT_COMPLETE_INTERRUPT,        //     uint8 enable_transmit_complete_interrupt;
    UART_DISABLE_DATA_REG_EMPTY_COMPLETE_INTERRUPT, //     uint8 enable_data_reg_empty_complete_interrupt;
    UART_ENABLE_RECEIVER,                           //     uint8 enable_receiver;
    UART_ENABLE_TRANSMITER,                         //     uint8 enable_transmiter;
    UART_8BIT_CHAR_SIZE,                            //     uint8 frame_data_bits_size;
    UART_DISABLE_PARITY,                            //     uint8 parity_mode;
    UART_1_BIT_STOP                                 //     uint8 stop_bits_mode;

};

EXT_Int_Conf ext_conf_master = {
    ENABLE_INT0,      //     uint8 Enable_INT0_Interrupt;
    ENABLE_INT1,      //     uint8 Enable_INT1_Interrupt;
    DISABLE_INT,      //     uint8 Enable_INT2_Interrupt;
    RISING_EDGE_MODE, //     uint8 INT0_Triggering_Mode;
    RISING_EDGE_MODE, //     uint8 INT1_Triggering_Mode;
    RISING_EDGE_MODE  //     uint8 INT2_Triggering_Mode;
};
SPI_Config_t spi_conf_master = {
    true,           // bool enable_interrupt;
    true,           // bool enable_spi;
    true,           // bool is_master;
    false,          // bool double_speed;
    CLK_HIGH_IDLE,  // uint8 clk_polarity;
    SETUP_ON_FIRST, // uint8 clk_phase;
    CLK_DIV_128,    // uint8 clk_divisor;
    DORD_LSB_FIRST  // uint8 data_order;
};

#define MAX_BUFFER_SIZE 256

uint8 uart_buffer[MAX_BUFFER_SIZE] = {0};
uint32 uart_buffer_wt_at = 0;

void delete_last_char(void);
void send_buffer_to_slave_mcu(void);

void project_master_init(void)
{
    /*set the ext0,ext1 pin direction to be input*/
    DIO_vSetPinDirection(PORTD, Pin_Int0, INPUT);
    DIO_vSetPinDirection(PORTD, Pin_Int1, INPUT);

    EXT_INT_SET_CONFIG(ext_conf_master);
    register_ext_int_callbacks(delete_last_char, send_buffer_to_slave_mcu, NULL);

    UART_vInit(uart_conf_master);
    SPI_Init(&spi_conf_master);
}
void project_master_app(void)
{
    while (1)
    {
        uint16 uart_data = UART_u16Receive_no_poll();
        if (uart_data != QUEUE_ERROR_EMPTY)
        {
            // buffer recieved data
            uart_buffer[uart_buffer_wt_at++] = (uint8)uart_data;
        }
    }
}
void delete_last_char(void)
{
    /*effectevly ignores the last buffer byte */

    uart_buffer[--uart_buffer_wt_at] = 0;
}
void send_buffer_to_slave_mcu(void)
{
    SPI_Transmit_Buffer_Async(uart_buffer, uart_buffer_wt_at, NULL);
}