/*
 * NRF24L01
 * auth:H jammoul
 * 27nov2021
 */

#include "stm32f7xx_hal.h"
#include "NRF24L01.h"
#include <stdio.h>
#include <inttypes.h>

extern SPI_HandleTypeDef hspi5;
#define NRF24_SPI &hspi5

#define NRF24_CE_PORT   GPIOB
#define NRF24_CE_PIN    GPIO_PIN_8

#define NRF24_CSN_PORT   GPIOB
#define NRF24_CSN_PIN    GPIO_PIN_9

void CS_Select (void)
{
	HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_RESET);
}

void CS_UnSelect (void)
{
	HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_SET);
}


void CE_Enable (void)
{
	HAL_GPIO_WritePin(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_PIN_SET);
}

void CE_Disable (void)
{
	HAL_GPIO_WritePin(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_PIN_RESET);
}

// write a single byte to the particular register
void nrf24_WriteReg (uint8_t Reg, uint8_t Data)
{
	uint8_t buf[2];
	buf[0] = Reg|1<<5;
	buf[1] = Data;

	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_SPI_Transmit(NRF24_SPI, buf, 2, 1000);
	while(HAL_SPI_GetState(&hspi5) != HAL_SPI_STATE_READY);
	// Pull the CS HIGH to release the device
	CS_UnSelect();
}

//write multiple bytes starting from a particular register
void nrf24_WriteRegMulti (uint8_t Reg, uint8_t *data, int size)
{
	uint8_t buf[2];
	buf[0] = Reg|1<<5;

	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_SPI_Transmit(NRF24_SPI, buf, 1, 100);
	HAL_SPI_Transmit(NRF24_SPI, data, size, 1000);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
}

// send the command to the NRF standalone command mainly flush for tx and rx
void nrfsendCmd (uint8_t cmd)
{
	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_SPI_Transmit(NRF24_SPI, &cmd, 1, 100);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
}

uint8_t nrf24_ReadReg(uint8_t Reg)
{
	uint16_t data=0;
  //PULL CS pin low to selesct device
	CS_Select();
	HAL_SPI_Transmit(NRF24_SPI, &Reg, 1,1000);
	while(HAL_SPI_GetState(&hspi5) != HAL_SPI_STATE_READY);
	HAL_SPI_Receive(NRF24_SPI, &data,1,1000);
		CS_UnSelect();
	return data;

}

void nrf24_reset(uint8_t REG)
{
	if (REG == STATUS)
	{
		nrf24_WriteReg(STATUS, 0x00);
	}

	else if (REG == FIFO_STATUS)
	{
		nrf24_WriteReg(FIFO_STATUS, 0x11);
	}

	else {
	nrf24_WriteReg(CONFIG, 0x08);
	nrf24_WriteReg(EN_AA, 0x3F);
	nrf24_WriteReg(EN_RXADDR, 0x03);
	nrf24_WriteReg(SETUP_AW, 0x03);
	nrf24_WriteReg(SETUP_RETR, 0x03);
	nrf24_WriteReg(RF_CH, 0x02);
	nrf24_WriteReg(RF_SETUP, 0x0E);
	nrf24_WriteReg(STATUS, 0x00);
	nrf24_WriteReg(OBSERVE_TX, 0x00);
	nrf24_WriteReg(CD, 0x00);
	uint8_t rx_addr_p0_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
	nrf24_WriteRegMulti(RX_ADDR_P0, rx_addr_p0_def, 5);
	uint8_t rx_addr_p1_def[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
	nrf24_WriteRegMulti(RX_ADDR_P1, rx_addr_p1_def, 5);
	nrf24_WriteReg(RX_ADDR_P2, 0xC3);
	nrf24_WriteReg(RX_ADDR_P3, 0xC4);
	nrf24_WriteReg(RX_ADDR_P4, 0xC5);
	nrf24_WriteReg(RX_ADDR_P5, 0xC6);
	uint8_t tx_addr_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
	nrf24_WriteRegMulti(TX_ADDR, tx_addr_def, 5);
	nrf24_WriteReg(RX_PW_P0, 0);
	nrf24_WriteReg(RX_PW_P1, 0);
	nrf24_WriteReg(RX_PW_P2, 0);
	nrf24_WriteReg(RX_PW_P3, 0);
	nrf24_WriteReg(RX_PW_P4, 0);
	nrf24_WriteReg(RX_PW_P5, 0);
	nrf24_WriteReg(FIFO_STATUS, 0x11);
	nrf24_WriteReg(DYNPD, 0);
	nrf24_WriteReg(FEATURE, 0);
	}
}




void NRF24_Init (void)
{
	// disable the chip before configuring the device
	CE_Disable();


	// reset everything
	nrf24_reset (0);

	nrf24_WriteReg(CONFIG, 0);  // will be configured later

	nrf24_WriteReg(EN_AA, 0);  // No Auto ACK

	nrf24_WriteReg (EN_RXADDR, 0);  // Not Enabling any data pipe right now

	nrf24_WriteReg (SETUP_AW, 0x03);  // 5 Bytes for the TX/RX address

	nrf24_WriteReg (SETUP_RETR, 0);   // No retransmission

	nrf24_WriteReg (RF_CH, 0);  // will be setup during Tx or RX

	nrf24_WriteReg (RF_SETUP, 0x0E);   // Power= 0db, data rate = 2Mbps

	uint8_t config = nrf24_ReadReg(CONFIG);
	printf("%" PRIu16 "\n",config);
	// Enable the chip after configuring the device
	CE_Enable();

}


// set up the Tx mode

void NRF24_TxMode (uint8_t *Address, uint8_t channel)
{
	// disable the chip before configuring the device
	CE_Disable();

	nrf24_WriteReg (RF_CH, channel);  // select the channel

	nrf24_WriteRegMulti(TX_ADDR, Address, 5);  // Write the TX address


	// power up the device
	uint8_t config = nrf24_ReadReg(CONFIG);
	config = config | (1<<1);   // write 1 in the PWR_UP bit

	config = config & (0xF2);    // write 0 in the PRIM_RX, and 1 in the PWR_UP, and all other bits are masked
	nrf24_WriteReg (CONFIG, config);

	// Enable the chip after configuring the device
	CE_Enable();
}


// transmit the data

uint8_t NRF24_Transmit (uint8_t *data)
{
	uint8_t cmdtosend = 0;

	// select the device
	CS_Select();

	// payload command
	cmdtosend = W_TX_PAYLOAD;
	HAL_SPI_Transmit(NRF24_SPI, &cmdtosend, 1, 100);
	while(HAL_SPI_GetState(&hspi5) != HAL_SPI_STATE_READY);
	// send the payload
	HAL_SPI_Transmit(NRF24_SPI, data, 32, 1000);
	while(HAL_SPI_GetState(&hspi5) != HAL_SPI_STATE_READY);
	// Unselect the device
	CS_UnSelect();

	HAL_Delay(1);

	uint8_t fifostatus = nrf24_ReadReg(FIFO_STATUS);
	printf("%" PRIu16 "\n",fifostatus);
	// check the fourth bit of FIFO_STATUS to know if the TX fifo is empty
	if ((!(fifostatus&(1<<3))))
	{
		cmdtosend = FLUSH_TX;
		nrfsendCmd(cmdtosend);

		// reset FIFO_STATUS
		nrf24_reset (FIFO_STATUS);

		return 1;
	}

	return 0;
}

