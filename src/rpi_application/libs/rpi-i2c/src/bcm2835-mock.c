/**
 * \file bcm2835-mock.c
 * \author Martin Kloesch (martin.kloesch-ee@infineon.com)
 * \brief Dummy implementation of bcm2835 driver for test purposes
 */
#include "bcm2835.h"

/**
 * \brief No-op implementation of \ref bcm2835_init(void) library function
 */
int bcm2835_init(void) { return 1; }

/**
 * \brief No-op implementation of \ref bcm2835_spi_begin(void) library function
 */
int bcm2835_spi_begin(void) { return 1; }

/**
 * \brief No-op implementation of \ref bcm2835_close(void) library function
 */
int bcm2835_close(void) { return 1; }

/**
 * \brief No-op implementation of \ref bcm2835_spi_end(void) library function
 */
void bcm2835_spi_end(void) {}

/**
 * \brief No-op implementation of \ref bcm2835_spi_setDataMode(uint8_t) library function
 */
void bcm2835_spi_setDataMode(uint8_t mode) {}

/**
 * \brief No-op implementation of \ref bcm2835_spi_set_speed_hz(uint32_t) library function
 */
void bcm2835_spi_set_speed_hz(uint32_t speed_hz) {}

/**
 * \brief No-op implementation of \ref bcm2835_spi_chipSelect(uint8_t) library function
 */
void bcm2835_spi_chipSelect(uint8_t cs) {}

/**
 * \brief No-op implementation of \ref bcm2835_spi_writenb(const char*, uint32_t) library function
 */
void bcm2835_spi_writenb(const char *buf, uint32_t len) {}

/**
 * \brief No-op implementation of \ref bcm2835_spi_transfern(char*, uint32_t) library function
 */
void bcm2835_spi_transfern(char *buf, uint32_t len) {}