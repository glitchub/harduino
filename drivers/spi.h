// SPI driver
void init_spi(void);
bool xfer_spi(uint8_t *txdata, uint8_t txcount, uint8_t rxskip, uint8_t *rxdata, uint8_t rxcount);
