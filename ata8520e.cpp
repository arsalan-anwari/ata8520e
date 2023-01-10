#include "ata8520e.h"


static Ata8520eInterface* _interfaceAta8520e;

void printStatusAta8520e(uint8_t statusMode){

    spiAquire( _interfaceAta8520e->spiDev, _interfaceAta8520e->spiClk, SPI_MODE_0, SPI_ORDER_MSB );

   _singleTransferAta8520e(ATA8520E_GET_STATUS);
   _singleTransferAta8520e(0);
   _singleTransferAta8520e(0); 
    uint8_t mask = _singleTransferAta8520e(0);

    spiRelease(_interfaceAta8520e->spiDev);

    switch (statusMode)
    {
    case SIGFOX_STATUS_MODE_CHIP:
        _printChipStatusAta8520e(mask);
        break;
    case SIGFOX_STATUS_MODE_SYSTEM:
        uint8_t mask = _singleTransferAta8520e(0);
        _printSystemStatusAta8520e(mask);
        break;
    case SIGFOX_STATUS_MODE_ALL:
        _printChipStatusAta8520e(mask);
        uint8_t mask = _singleTransferAta8520e(0);
        _printSystemStatusAta8520e(mask);
        break;
    default:
        print("Error invalid status mode");
        break;
    }
}

void sendCmdAta8520e(uint8_t cmd){
    spiAquire( _interfaceAta8520e->spiDev, _interfaceAta8520e->spiClk, SPI_MODE_0, SPI_ORDER_MSB );
    _singleTransferAta8520e(cmd);
    spiRelease(_interfaceAta8520e->spiDev);
}

void resetAta8520e(uint8_t resetMode){
    switch (resetMode)
    {
    case SIGFOX_RESET_MODE_SYSTEM:
        _resetSystemAta8520e();
        break;
    case SIGFOX_RESET_MODE_CHIP:
        _resetChipAta8520e();
        break;
    default:
        print("Error invalid reset mode");
        break;
    }
}

void setModeAta8520e(uint8_t deviceMode){
    switch (deviceMode)
    {
    case SIGFOX_MODE_ON:
        gpioSet( _interfaceAta8520e->power, HIGH );
        _resetChipAta8520e();
        break;
    case SIGFOX_MODE_OFF:
        printStatusAta8520e(SIGFOX_STATUS_MODE_ALL);
        gpioSet( _interfaceAta8520e->power, LOW );
        sendCmdAta8520e(ATA8520E_OFF_MODE);
        break;
    default:
        print("Error invalid device mode");
        break;
    }
}


void readAta8520e(uint8_t* buff, uint8_t size){
    setModeAta8520e(SIGFOX_MODE_ON);

    spiAquire( _interfaceAta8520e->spiDev, _interfaceAta8520e->spiClk, SPI_MODE_0, SPI_ORDER_MSB );
    _singleTransferAta8520e(ATA8520E_GET_PAC);
    _singleTransferAta8520e(0);
    _multiTransferAta8520e(nullptr, buff, size );
    spiRelease(_interfaceAta8520e->spiDev);

    setModeAta8520e(SIGFOX_MODE_ON);
}

void sendAta8520e(uint8_t* buff, uint8_t size){
    _prepareAta8520e(buff, size);
    sendCmdAta8520e(ATA8520E_SEND_FRAME);

    // wait untill message is send or timeout of 8s is reached.
    sleepMs(TX_TIMEOUT * 1000);
    setModeAta8520e(SIGFOX_MODE_OFF);
}

bool CreateAta8520e(SigfoxDevice* self, Ata8520eInterface* interface){
    _interfaceAta8520e = interface;

    self->printStatus = &printStatusAta8520e;
    self->sendCmd = &sendCmdAta8520e;
    self->reset = &resetAta8520e;
    self->setMode = &setModeAta8520e;
    self->read = &readAta8520e;
    self->send = &sendAta8520e;

    bool error = _initAta8520e();
    if (error) { 
        print("Unable to initialize SigfoxDevice[Ata8520e]! \n")
        return false; 
    }
    print("Initialized SigfoxDevice[Ata8520e] sucessfully! \n")
    return true;
}

/////

bool _initAta8520e(void){
    gpioInit(_interfaceAta8520e->int_, GPIO_IN);
    gpioInit(_interfaceAta8520e->power, GPIO_OUT);
    gpioInit(_interfaceAta8520e->reset, GPIO_OUT);
    gpioInit(_interfaceAta8520e->nss, GPIO_OUT);

    setModeAta8520e(SIGFOX_MODE_ON);
    if ( spiInit(_interfaceAta8520e->spiDev) == -1){ return false; }

    sleepMs(100);
    printStatusAta8520e(SIGFOX_STATUS_MODE_ALL);
    setModeAta8520e(SIGFOX_MODE_OFF);

    return true;
}

void _printChipStatusAta8520e(uint8_t mask){
    
    if (mask & ATA8520E_ATMEL_PA_MASK) {
       print("PA ON\n");
    }else {
       print("PA OFF\n");
    }
    
    if ((mask >> 1) & ATA8520E_ATMEL_SYSTEM_READY_MASK) {
       print("System ready to operate\n");
        return;
    }
    
    if ((mask >> 1) & ATA8520E_ATMEL_FRAME_SENT_MASK) {
        print("Frame sent\n");
        return;
    }

    switch ((mask >> 1) & 0x0F) {
        case ATA8520E_ATMEL_OK:
           print("System is ready\n");
            break;
        case ATA8520E_ATMEL_COMMAND_ERROR:
           print("Command error / not supported\n");
            break;
        case ATA8520E_ATMEL_GENERIC_ERROR:
           print("Generic error\n");
            break;
        case ATA8520E_ATMEL_FREQUENCY_ERROR:
           print("Frequency error\n");
            break;
        case ATA8520E_ATMEL_USAGE_ERROR:
           print("Usage error\n");
            break;
        case ATA8520E_ATMEL_OPENING_ERROR:
           print("Opening error\n");
            break;
        case ATA8520E_ATMEL_CLOSING_ERROR:
           print("Closing error\n");
            break;
        case ATA8520E_ATMEL_SEND_ERROR:
           print("Send error\n");
            break;
        default:
           print("Unknown mask code\n");
            break;
    }

}


void _printSystemStatusAta8520e(uint8_t mask){
    switch (mask) {
        case ATA8520E_SIGFOX_NO_ERROR:
           print("OK\n");
            break;
        case ATA8520E_SIGFOX_TX_LEN_TOO_LONG:
           print("TX data length exceeds 12 bytes\n");
            break;
        case ATA8520E_SIGFOX_RX_TIMEOUT:
           print("Timeout for downlink message\n");
            break;
        case ATA8520E_SIGFOX_RX_BIT_TIMEOUT:
           print("Timeout for bit downlink\n");
            break;
        case ATA8520E_SIGFOX2_INIT_ERROR:
           print("Initialization error\n");
            break;
        case ATA8520E_SIGFOX2_TX_ERROR:
           print("Error during send\n");
            break;
        case ATA8520E_SIGFOX2_RF_ERROR:
           print("Error in RF frequency\n");
            break;
        case ATA8520E_SIGFOX2_DF_WAIT_ERROR:
           print("Error during wait for data frame\n");
            break;
        default:
           print("Internal error\n");
            break;
    }
}


uint8_t _singleTransferAta8520e(uint8_t value){
    gpioSet( _interfaceAta8520e->nss, LOW );
    sleepMs(1);
    uint8_t response = spiTransfer(_interfaceAta8520e->spiDev, value);
    sleepMs(1);
    gpioSet( _interfaceAta8520e->nss, HIGH );
    return response;
}

void _multiTransferAta8520e(uint8_t* output, uint8_t* input, uint8_t size){
    gpioSet( _interfaceAta8520e->nss, LOW );
    sleepMs(1);
    spiTransferBytes(_interfaceAta8520e->spiDev, input, output, size);
    sleepMs(1);
    gpioSet( _interfaceAta8520e->nss, HIGH );
}

void _resetSystemAta8520e(void){
    sendCmdAta8520e(ATA8520E_SYSTEM_RESET);
}

void _resetChipAta8520e(void){
    gpioSet( _interfaceAta8520e->reset, HIGH );
    sleepMs(10);
    gpioSet( _interfaceAta8520e->reset, LOW );
    sleepMs(10);
    gpioSet( _interfaceAta8520e->reset, HIGH );
}

void _prepareAta8520e(uint8_t* msg, uint8_t size){
    setModeAta8520e(SIGFOX_MODE_ON);
    sleepMs(5);
    printStatusAta8520e(SIGFOX_STATUS_MODE_ALL);

    if (size > 12U){ 
        print("Message is too long to send! So it will be shortend to 12 characters\n");
        size = 12; 
    }

    print("Writing to RX buffer...\n");
    spiAquire( _interfaceAta8520e->spiDev, _interfaceAta8520e->spiClk, SPI_MODE_0, SPI_ORDER_MSB );
    _singleTransferAta8520e(ATA8520E_WRITE_TX_BUFFER);
    _singleTransferAta8520e(size);
    _multiTransferAta8520e(msg, nullptr, size);
    spiRelease(_interfaceAta8520e->spiDev);
}


