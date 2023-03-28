#include "TWI_REG.h"
#include "TWI_Interface.h"
#include "TWI_Private.h"
#include "../../LIB/Calcbit.h"
#include "../GIE/GIE_Interface.h"
#include <math.h>

// typedef struct TWI_Config
// {
//     /* data */
//     uint32 scl_freq;     // shouldn't exceed twi_max_scl supported
//     uint8 scl_prescaler; // shouldn't exceed twi_max_scl supported
//     bool enable_ack;
//     bool enable_twi;
//     bool enable_interrupt;
//     uint8 own_slave_address;
//     bool enable_general_call;
// } TWI_Config_t;

void TWI_Init(TWI_Config_t conf)
{
    /* set prescaler */
    TWSR |= (conf.scl_prescaler) & 0x03;

    uint32 scl = (conf.scl_freq > TWI_MAX_SCL) ? TWI_MAX_SCL : conf.scl_freq;
    TWBR = ((F_CPU / scl) - 16) / (2 * pow(4, conf.scl_prescaler));
    if (conf.enable_ack) // check if this bit shouldn't be set at the init.
        setbit(TWCR, TWEA);
    if (conf.enable_twi)
        setbit(TWCR, TWEN);
    if (conf.enable_interrupt)
    {
        sei();
        setbit(TWCR, TWIE);
    }
    uint8 addr = (conf.own_slave_address) << 1;
    if (conf.enable_general_call)
        addr += 1;
    TWAR = addr;
}

// bool TWI_Master_Transmit_Byte(uint8 byte, uint8 slave_adrr)
// {
//     {
//         TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
//         while (!(TWCR & (1 << TWINT)))
//             ;
//         if ((TWSR & 0xF8) != START)
//             return MT_START_ERROR;
//     }

//     {
//         TWDR = (slave_adrr);
//         TWCR = (1 << TWINT) | (1 << TWEN);
//         while (!(TWCR & (1 << TWINT)))
//             ;
//         if ((TWSR & 0xF8) != MT_SLA_ACK)
//             return MT_SLA_ERROR;
//     }
//     {
//         TWDR = byte;
//         TWCR = (1 << TWINT) | (1 << TWEN);
//         while (!(TWCR & (1 << TWINT)))
//             ;

//         if ((TWSR & 0xF8) != MT_DATA_ACK)
//             return MT_DATA_ERROR;
//     }
//     {
//         TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
//     }
//     return TWI_SUCESS;
// }

uint8 TWI_Start()
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)))
        ;
    if ((TWSR & 0xF8) != START)
        return MT_START_ERROR;
    return TWI_SUCESS;
}
uint8 TWI_SlaveAdrr(uint8 slave_adrr, uint8 read_write)
{
    TWDR = slave_adrr + read_write;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)))
        ;
    uint8 status = (TWSR & 0xF8);
    if (read_write == TWI_WRITE)
    {
        if ((status != MT_SLA_ACK) || (status != MT_SLA_NACK))
            return MT_SLA_ERROR;
    }
    else
    {
        if ((status != MR_SLA_ACK) || (status != MR_SLA_NACK))
            return MR_SLA_ERROR;
    }
}
bool TWI_Master_Transmit_Byte(uint8 byte, uint8 slave_adrr)
{
    if (TWI_Start() == MT_START_ERROR)
        return false;

    TWI_SlaveAdrr(slave_adrr, TWI_WRITE);

    /* send data */
    TWDR = byte;
    setbit(TWCR, TWINT); // clear flag

    /* wait for data  to be sent*/
    while (getbit(TWCR, TWINT) == 0)
        ;

    /*Check value of TWI Status Register*/
    if ((TWSR & 0xF8) != MT_DATA_ACK)
        return MT_DATA_ERROR;

    /* send stop cond */
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    // setbit(TWCR, TWSTO);

    return TWI_SUCESS;
}

bool TWI_Master_Transmit_Buffer(uint8 *buff, uint32 buff_size, uint8 slave_adrr)
{
    if (TWI_Start() == MT_START_ERROR)
        return false;

    TWI_SlaveAdrr(slave_adrr, TWI_WRITE);

    for (uint32 i = 0; i < buff_size; ++i)
    {
        /* send data */
        TWDR = buff[i];
        setbit(TWCR, TWINT); // clear flag

        /* wait for data  to be sent*/
        while (getbit(TWCR, TWINT) == 0)
            ;

        /*Check value of TWI Status Register*/
        if ((TWSR & 0xF8) != MT_DATA_ACK)
            return MT_DATA_ERROR;

        // // repeat start
        // TWI_Start();
        // TWI_SlaveAdrr(slave_adrr, TWI_WRITE);
    }

    /* send stop cond */
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    // setbit(TWCR, TWSTO);

    return TWI_SUCESS;
}

uint8 TWI_Master_Receive_Byte(uint8 slave_adrr)
{
    uint8 byte;
    if (TWI_Start() == MT_START_ERROR)
        return false;

    TWI_SlaveAdrr(slave_adrr, TWI_READ);

    /* wait for data  to be recieved*/
    while (getbit(TWCR, TWINT) == 0)
        ;

    /* rec data */
    TWCR = (1 << TWINT) | (1 << TWEN); // send nack

    /* wait for nack  to be recieved*/
    while (getbit(TWCR, TWINT) == 0)
        ;
    byte = TWDR;
    /*Check value of TWI Status Register*/
    if ((TWSR & 0xF8) != MR_DATA_NACK)
        return MR_DATA_ERROR;

    /* send stop cond */
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    // setbit(TWCR, TWSTO);

    return byte;
}

uint8 TWI_Slave_Receive_Byte(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    /* wait for data  to be recieved*/
    while (getbit(TWCR, TWINT) == 0)
        ;
    uint8 status = TWSR & 0xF8;                           /* Read TWI status register */
    if ((status != SR_SLA_ACK) || (status != SR_SLA_ACK)) /* Own SLA+W received &ack returned */
        return SR_SLA_ERROR;

    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)))
        ;                                                     /* Wait until TWI finish its current job */
    status = TWSR & 0xF8;                                     /* Read TWI status register */
    if (status == SR_DATA_ACK || status == SR_GCALL_DATA_ACK) /* Check for data received &ack returned */
        return TWDR;                                          /* If yes then return received data */
}
bool TWI_Slave_Receive_Buffer(char *buffer, uint32 buff_size)
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    /* wait for data  to be recieved*/
    while (getbit(TWCR, TWINT) == 0)
        ;
    uint8 status = TWSR & 0xF8;                           /* Read TWI status register */
    if ((status != SR_SLA_ACK) || (status != SR_SLA_ACK)) /* Own SLA+W received &ack returned */
        return SR_SLA_ERROR;

    for (uint32 i = 0; i < buff_size; ++i)
    {
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
        while (!(TWCR & (1 << TWINT)))
            ;                                                     /* Wait until TWI finish its current job */
        status = TWSR & 0xF8;                                     /* Read TWI status register */
        if (status == SR_DATA_ACK || status == SR_GCALL_DATA_ACK) /* Check for data received &ack returned */
            buffer[i] = TWDR;
    }
}