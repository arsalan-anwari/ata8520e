#ifndef ATA8520E_HPP
#define ATA8520E_HPP

#include "../../hal/hal.h"
#include "../../tools/data_types/byte_types.h"    
#include "../sigfox_device.h"

#include "ata8520e_regs.h"
#include "ata8520e_settings.h"

typedef struct Ata8520eInterface {
    spi_t spiDev;
    spi_clk_t spiClk;
    gpio_t nss;
    gpio_t int_;
    gpio_t power;
    gpio_t reset;
} Ata8520eInterface;

void printStatusAta8520e(uint8_t statusMode);
void sendCmdAta8520e(uint8_t cmd);
void resetAta8520e(uint8_t resetMode);
void setModeAta8520e(uint8_t deviceMode);
void readAta8520e(uint8_t* buff, uint8_t size);
void sendAta8520e(uint8_t* buff, uint8_t size);

bool CreateAta8520e(SigfoxDevice* self, Ata8520eInterface* interface);

/////////////

bool _initAta8520e(void);
void _printChipStatusAta8520e(uint8_t mask);
void _printSystemStatusAta8520e(uint8_t mask);

uint8_t _singleTransferAta8520e(uint8_t value);
void _multiTransferAta8520e(uint8_t* output, uint8_t* input, uint8_t size);

void _resetSystemAta8520e(void);
void _resetChipAta8520e(void);

void _prepareAta8520e(uint8_t* msg, uint8_t size);


#endif /* ATA8520E_HPP */
