//contains only ADDITIONALLY ADDED code for the I2C assignment
//contains code added to i2c_base.hpp/cpp, i2c2.hpp/cpp, terminal.cpp, handlers.hpp/cpp, and "a file I created".hpp/cpp

//Start of functions and variables added to i2c_base.hpp
//Slave initialization function
bool i2c_BaseSlaveInit(uint8_t slaveAddress, uint8_t* buffer, uint16_t sizeOfBuffer);

//Private Variables used for Slave Information
typedef struct
{
	uint8_t* slaveBufferAddress;
	uint16_t slaveBufferSize;
	uint16_t slaveByteCount;
	uint16_t bufferIndex;
}slaveInfo_t;

//Initialize slaveInfo struct
slaveInfo_t slaveInfo;
//End of functions and variables added  to i2c_base.hpp

//Start of functions and variables added to i2c_base.cpp
//for better readability in code below
#define slaveIndexedLocation slaveInfo.slaveBufferAddress[slaveInfo.bufferIndex % slaveInfo.slaveBufferSize]

//slave initialization function
bool I2C_Base::i2c_BaseSlaveInit(uint8_t slaveAddress, uint8_t* buffer, uint16_t sizeOfBuffer)
{
    //power i2c2
    switch(mIRQ)
    {
        case I2C0_IRQn: lpc_pconp(pconp_i2c0, true);  break;
        case I2C1_IRQn: lpc_pconp(pconp_i2c1, true);  break;
        case I2C2_IRQn: lpc_pconp(pconp_i2c2, true);  break;
        default: return false;
    }

    //Clear Control Register
    mpI2CRegs->I2CONCLR = 0x7C;

    //Set Slave Address, only responds to 1 address
    mpI2CRegs->I2ADR0 = slaveAddress;
    mpI2CRegs->I2ADR1 = 0;
    mpI2CRegs->I2ADR2 = 0;
    mpI2CRegs->I2ADR3 = 0;

    slaveInfo.slaveBufferAddress = buffer;
    slaveInfo.slaveBufferSize = sizeOfBuffer;
    slaveInfo.slaveByteCount = 0;
    slaveInfo.bufferIndex = 0;

    //Set Control Register. set interrupt and ack bit
    mpI2CRegs->I2CONSET = 0x44;

    //enable i2c2 interrupt
    NVIC_EnableIRQ(mIRQ);
    return true;
}

//states added to i2cStateMachine enum
slaveAddressWriteReceived = 0x60,
dataReceived = 0x80,

slaveRepeatStartReceived = 0xA0,
slaveAddressReadReceived = 0xA8,
dataReadAckReceived = 0xB8,
dataReadNackReceived = 0xC0,

//added to i2cStateMachine switch statement
case slaveAddressWriteReceived: //0x60
	slaveInfo.slaveByteCount = 0;
	setAckFlag();
	clearSIFlag();
	break;

case dataReceived: //0x80
    	//printf("In dataReceived state\n\r");
	if(slaveInfo.slaveByteCount == 0) //first byte received
	{
	slaveInfo.bufferIndex = mpI2CRegs->I2DAT;
	slaveInfo.slaveByteCount++;
	}
	
	else
	{
	slaveIndexedLocation = mpI2CRegs->I2DAT;
	
	//turn on LED if a number is written to index 0. led off otherwise.
	if(slaveInfo.bufferIndex  % slaveInfo.slaveBufferSize == 0)
	{
	    if(slaveIndexedLocation >= '0' && slaveIndexedLocation <= '9')
	    {
	        //turn led on
	        LPC_GPIO0->FIOSET = 0b1 << 0; // TURN LED ON
	    }
	
	    else
	    {
	        //turn led off
	        LPC_GPIO0->FIOCLR = 0b1 << 0;
	    }
	}
	slaveInfo.bufferIndex++;
	slaveInfo.slaveByteCount++;
	}
	setAckFlag();
	clearSIFlag();
	break;

case slaveRepeatStartReceived: //0xA0
	setAckFlag();
	clearSIFlag();
	break;

case slaveAddressReadReceived: //0xA8
	mpI2CRegs->I2DAT = slaveIndexedLocation;
	slaveInfo.bufferIndex++;
	setAckFlag();
	clearSIFlag();
	break;

case dataReadAckReceived: //0xB8
	mpI2CRegs->I2DAT = slaveIndexedLocation;
	slaveInfo.bufferIndex++;
	setAckFlag();
	clearSIFlag();
	break;

case dataReadNackReceived: //0xC0
	setAckFlag();
	clearSIFlag();
	break;
//End of functions and variables added to i2c_base.cpp

//Start of functions and variables added to i2c2.hpp
//I2C2 slave init function header
bool i2c2SlaveInit(uint8_t slaveAddress, uint8_t* buffer, uint16_t sizeOfBuffer);
//End of functions and variables added to i2c2.hpp

//Start of functions and variables added to i2c2.cpp
bool I2C2::i2c2SlaveInit(uint8_t slaveAddress, uint8_t* buffer, uint16_t sizeOfBuffer)
{
    //Stole this section from Preet
    
    const uint32_t i2c_pin_mask = ( (1<<10) | (1<<11) );
    const bool i2c_wires_are_pulled_high = (i2c_pin_mask == (LPC_GPIO0->FIOPIN & i2c_pin_mask) );

    LPC_PINCON->PINMODE0 &= ~(0xF << 20); // Both pins with Pull-Up Enabled
    LPC_PINCON->PINMODE0 |=  (0xA << 20); // Disable both pull-up and pull-down

    // Enable Open-drain for I2C2 on pins P0.10 and P0.11
    LPC_PINCON->PINMODE_OD0 |= i2c_pin_mask;
    
    //End of stealing

    //peripheral clock selection. clear bits first, then set. clk is set up with no divisor
    LPC_SC->PCLKSEL1 &= ~(0b11 << 20);
    LPC_SC->PCLKSEL1 |= 0b1 << 20;

    //changing pins to SCL2 and SDA2
    //clear bits then set. sda2 is selected
    LPC_PINCON->PINSEL0 &= ~(0b11 << 20);
    LPC_PINCON->PINSEL0 |= 0b10 << 20;
    //clear bits then set. scl2 is selected
    LPC_PINCON->PINSEL0 &= ~(0b11 << 22);
    LPC_PINCON->PINSEL0 |= 0b10 << 22;

    //Stealing again
    if (i2c_wires_are_pulled_high) {
        return I2C_Base::i2c_BaseSlaveInit(slaveAddress, buffer, sizeOfBuffer);
    }
   
    else {
        disableOperation();
        return false;
    }
}
//End of functions and variables added to i2c2.cpp

//Start of functions and variables added to handlers.hpp
CMD_HANDLER_FUNC(i2c2HwHandler);
//End of functions and variables added to handlers.hpp

//Start of functions and variables added to handlers.cpp
CMD_HANDLER_FUNC(i2c2HwHandler)
{
    //to check if the uart2 driver was initialized
    static bool masterInit = false;
    static bool slaveInit = false;
    static const uint8_t slaveDeviceAddress = 0x70;

    static uint8_t slaveBuffer[256] = {'X','Y','Z','F'};
    static uint8_t masterReadBuffer[256] = {'A', 'B', 'C', 'D'};

    I2C2& i2cInstance = I2C2::getInstance();
    if(cmdParams.beginsWith("masterinit"))
    {
        if(slaveInit == true || masterInit == true)
        {
            printf("I2C2 has already been initialized.\n\r");
            return true;
        }
        masterInit = true;
        i2cInstance.init(10);
        printf("I2C2 is initialized as a Master Device.\n\r");
        return true;
    }

    else if(cmdParams.beginsWith("slaveinit"))
    {
        if(slaveInit == true || masterInit == true)
        {
            printf("I2C2 has already been initialized.\n\r");
            return true;
        }
        slaveInit = true;
        i2cInstance.i2c2SlaveInit(slaveDeviceAddress,slaveBuffer,sizeof(slaveBuffer));
        slaveMemoryLEDInit();
        printf("I2C2 is initialized as a Slave Device.\n\r");
        return true;
    }

    else if(cmdParams.beginsWith("-rs"))
    {
        if(slaveInit == true) // just read directly from buffer
        {
            uint16_t count = 0;
            for(int i = 0; i < 16; i++)
            {
                printf("%03X   ",count); //formatting base address
                for(int j = 0; j < 16; j++)
                {
                    printf("%c ", slaveBuffer[count]);
                    count++;
                }
                printf("\n\r");
            }
            return true;
        }

        else // need to call from slave device
        {
            i2cInstance.readRegisters(slaveDeviceAddress, 0, masterReadBuffer, 256);
            uint16_t count = 0;
            for(int i = 0; i < 16; i++)
            {
                printf("%03X   ",count); //formatting base address
                for(int j = 0; j < 16; j++)
                {
                    printf("%c ", masterReadBuffer[count]);
                    count++;
                }
                printf("\n\r");
            }
            return true;
        }
    }

    else if(cmdParams.beginsWith("-ws"))
    {
        if(masterInit == false)
        {
            printf("I2C2 must be initialized as master\n\r");
            return false;
        }

        uint16_t ind;
        uint32_t length;
        uint8_t masterWriteBuffer[256];

        if(2 != cmdParams.scanf("%*s %u %s", &ind, &masterWriteBuffer))
        {
            printf("Invalid Parameters.\n\r");
            return true;
        }

        length = uint8Length(masterWriteBuffer); //because the cstring library sucks and doesnt know how to adjust to casting.

        i2cInstance.writeRegisters(slaveDeviceAddress, ind, masterWriteBuffer, length);
        printf("Finished Command.\n\r");
        return true;
    }

    else
    {
        printf("you probably entered something invalid bro.\n\n\r");
        printf("OPTIONS:\n\r");
        printf("masterinit                 Initialize as master\n\r");
        printf("slaveinit                  Initialize as slave\n\r");
        printf("MASTER OPTIONS:\n\r");
        printf("-rs                        read slave memory.\n\r");
        printf("-ws {index} {character}    write slave memory.\n\r");
        printf("Slave OPTIONS:\n\r");
        printf("-rs                        read slave memory.\n\r");
        return true;
    }
}
//End of functions and variables added to handlers.cpp

//Start of functions and variables added to "a file I created".hpp
void slaveMemoryLEDInit();
uint32_t uint8Length(const uint8_t * cstring);
//End of functions and variables added to "a file I created".hpp

//Start of functions and variables added to "a file I created".cpp
void slaveMemoryLEDInit()
{
    uint32_t led = 0b1 << 0;
    LPC_GPIO0->FIODIR |= led;
}

uint32_t uint8Length(const uint8_t * cstring)
{
    int count = 0;
    for(count = 0; cstring[count] != 0; count++)
    {
	//keep countttinnnn.
    }
    return count;
}
//End of functions and variables added to "a file I created".cpp
