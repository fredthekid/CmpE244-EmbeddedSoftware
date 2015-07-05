// Code contains files that are part of 5 separate files in my workspace.

//Start of "the file I made" .cpp and .hpp code
//UART2 HW Headers
void initUART2();
void uart2Baud(uint32_t baudRate);
bool uart2SendByte(char send);
char uart2GetByte(void);
void UART2_IRQHandler(void);
void uart2Homework(const char *);

//UART3 HW Headers
void initUART3();
void UART3_IRQHandler(void);
void uart3Baud(uint32_t baudRate);
char uart3GetByte(void);
bool uart3SendByte(char send);
void uart3Homework(const char *);

//UART2 HW
//Init UART2 (38400 Default Baud Rate)
void initUART2()
{
    //Powering the UART2
    uint32_t uart2Power = 1 << 24;
    LPC_SC->PCONP |= uart2Power;

    //Pin Select for UART2
    uint32_t uart2TX = 0b10 << 16;
    uint32_t uart2RX = 0b10 << 18;
    LPC_PINCON->PINSEL4 &= ~(0b1111 << 16); // clear uart2 pin select
    LPC_PINCON->PINSEL4 |= (uart2TX | uart2RX);

    //Setting up UART2 Clock
    uint32_t uart2Clock = 1 << 16;
    LPC_SC->PCLKSEL1 &= ~(0b11 << 16); // clear uart2 clock
    LPC_SC->PCLKSEL1 |= uart2Clock;

    //Set Baud Rate
    uart2Baud(38400); //default init baud rate

    //Set character bit send length
    LPC_UART2->LCR = 3;

    //Enable Interrupt
    NVIC_EnableIRQ(UART2_IRQn);
    LPC_UART2->IER |= 1 << 0;
}

//Set UART2 Baud Rate
void uart2Baud(uint32_t baudRate)
{
    //Turn on Divisor Latch Access Bit
    LPC_UART2->LCR |= (1 << 7);
    uint16_t config = sys_get_cpu_clock()/(baudRate * 16);

    LPC_UART2->DLL = config & 0xFF;
    LPC_UART2->DLM = config >> 8;

    //Turn off Divisor Latch Access Bit
    LPC_UART2->LCR &= ~(1 << 7);
}

//UART2 Send byte
bool uart2SendByte(char send)
{
    LPC_UART2->THR = send;
    while(!(LPC_UART2->LSR & (1 << 5)))
    {
        //printf("Finish Sending Data\n\r");
        // wait for data to finish sending
    }
    //printf("Sent\n\r");
    return true;
}

//uart2 get byte by polling
char uart2GetByte(void)
{
    while(!(LPC_UART2->LSR & 1))
    {
        //poll for data
    }
    char c = LPC_UART2->RBR;
    return c;
}

//uart2 rx isr
void UART2_IRQHandler(void)
{
    char c = LPC_UART2->RBR;
    printf("%c",c);
    NVIC->ICPR[0] &= ~(1 << 7);
}

//UART2 Alphabet Homework Code
void uart2Homework(const char * input)
{
    //Disable Interrupt
    NVIC_DisableIRQ(UART2_IRQn);

    const char first[] = "first"; //first to send. will send an 'A'
    char get = 0;
    if(strcmp(input,first) == 0)
    {
        uart2SendByte('A');
    }

    while(get != 'Z')
    {
        get = uart2GetByte();
        printf("%c\n\r", get);
        if(get == 'Z')
        {
            break;
        }
        get += 1;
        uart2SendByte(get);
    }

    printf("Finished!\n\r");

    //Enable Interrupt
    NVIC_EnableIRQ(UART2_IRQn);
}

//UART3
//Uart3 init
void initUART3()
{
    //Powering the UART3
    uint32_t uart3Power = 1 << 25;
    LPC_SC->PCONP |= uart3Power;

    //Pin Select for UART3
    uint32_t uart3TX = 0b11 << 24;
    uint32_t uart3RX = 0b11 << 26;
    LPC_PINCON->PINSEL9 &= ~(0b1111 << 24);
    LPC_PINCON->PINSEL9 |= (uart3TX | uart3RX);

    //Setting up UART3 Clock
    uint32_t uart3Clock = 1 << 18;
    LPC_SC->PCLKSEL1 &= ~(0b11 << 18); // clear uart3 clock
    LPC_SC->PCLKSEL1 |= uart3Clock;

    //Set Baud Rate
    uart3Baud(38400);
    LPC_UART3->LCR = 3;

    NVIC_EnableIRQ(UART3_IRQn);
    LPC_UART3->IER |= 1 << 0;
}

//uart3 rx isr
void UART3_IRQHandler(void)
{
    char c = LPC_UART3->RBR;
    printf("%c",c);
    NVIC->ICPR[0] &= ~(1 << 7);
}

//function used to configure baud rate
void uart3Baud(uint32_t baudRate)
{
    //Turn on Divisor Latch Access Bit
    LPC_UART3->LCR |= (1 << 7);
    uint16_t config = sys_get_cpu_clock()/(baudRate * 16);

    LPC_UART3->DLL = config & 0xFF;
    LPC_UART3->DLM = config >> 8;

    //Turn off Divisor Latch Access Bit
    LPC_UART3->LCR &= ~(1 << 7);
}

//for sending a byte
bool uart3SendByte(char send)
{
    LPC_UART3->THR = send;
    while(!(LPC_UART3->LSR & (1 << 5)))
    {
        // wait for data to finish sending
    }
    return true;
}

//receive byte using polling
char uart3GetByte(void)
{
    while(!(LPC_UART3->LSR & 1))
    {
        delay_ms(1);
    }
    char c = LPC_UART3->RBR;
    return c;
}

//UART3 Alphabet Homework Code
void uart3Homework(const char * input)
{
    //Disable Interrupt
    NVIC_DisableIRQ(UART3_IRQn);

    const char first[] = "first"; //first to send. will send an 'A'
    char get = 0;
    if(strcmp(input,first) == 0)
    {
        uart3SendByte('A');
    }

    while(get != 'Z')
    {
        get = uart3GetByte();
        printf("%c\n\r", get);
        if(get == 'Z')
        {
            break;
        }
        get += 1;
        uart3SendByte(get);
    }

    printf("Finished!\n\r");

    //Enable Interrupt
    NVIC_EnableIRQ(UART3_IRQn);
}
//End of "the file I made" .cpp and .hpp code

//Start of "handlers.cpp" and "handlers.hpp" code

//Function Headers
CMD_HANDLER_FUNC(uart2HwHandler);
CMD_HANDLER_FUNC(uart3HwHandler);

//Uart2 Handler function
CMD_HANDLER_FUNC(uart2HwHandler)
{
    //to check if the uart2 driver was initialized
    static bool uart2Init = false;

    //initialize uart2 driver, baud rate = 38400, RX interrupt on
    if(cmdParams == "init")
    {
        initUART2();
        isr_register(UART2_IRQn, UART2_IRQHandler);
        uart2Init = true;
        printf("GPIO UART2 Initialized!\n\r");
        return true;
    }

    //changing the baud rate
    else if(cmdParams.beginsWith("-b")) //configure baud rate
    {
        if(uart2Init == false)
        {
            printf("UART2 needs to be initialized first.\n\r");
            return true;
        }
        unsigned int baudRate = 0;
        if(1 != cmdParams.scanf("%*s %u", &baudRate))
        {
            return false;
        }

        if(baudRate > 500000)
        {
            printf("Baud Rate needs to be less than 500000");
            return true;
        }

        else
        {
            uart2Baud(baudRate);
            printf("Baud Rate was set to %u", baudRate);
            return true;
        }
    }

    //Enable/Disable Interrupt
    else if(cmdParams.beginsWith("-i"))
    {
        if(uart2Init == false)
        {
            printf("UART2 needs to be initialized first.\n\r");
            return true;
        }
        const char u2on[] = "on";
        const char u2off[] = "off";
        char onOff[3];
        if(1 != cmdParams.scanf("%*s %s", &onOff))
        {
            printf("Invalid. -i on or -i off\n\r");
            return true;
        }

        if(strcmp(onOff,u2on) == 0)
        {
            //turn interrupt on
            NVIC_EnableIRQ(UART2_IRQn);
            printf("UART2 Interrupt Enabled!\n\r");
            return true;
        }

        else if(strcmp(onOff,u2off) == 0)
        {
            //turn interrupt off
            NVIC_DisableIRQ(UART2_IRQn);
            printf("UART2 Interrupt Disabled!\n\r");
            return true;
        }

        else
        {
            //invalid
            printf("Invalid. -i on or -i off\n\r");
            return true;
        }

    }

    //used for sending bytes
    else if(cmdParams.beginsWith("-s"))
    {
        if(uart2Init == false)
        {
            printf("UART2 needs to be initialized first.\n\r");
            return true;
        }
        char sendString[256];
        if(1 != cmdParams.scanf("%*s %s", &sendString))
        {
            return false;
        }

        if(strlen(sendString) <= 0)
        {
            printf("no string was entered\n\r");
            return true;
        }

        printf("String Entered Was: %s\n\r", sendString);

        for(unsigned int i = 0; i < strlen(sendString); i++)
        {
            uart2SendByte(sendString[i]);
        }
        uart2SendByte('\n');
        uart2SendByte('\r');
        printf("Sent.\n\r");
        return true;
    }

    //uart2 homework, sends alphabets from A to Z back and forth
    else if(cmdParams.beginsWith("-h"))
    {
        if(uart2Init == false)
        {
            printf("UART2 needs to be initialized first.\n\r");
            return true;
        }
        const char uart2First[] = "first"; //first to send. will send an 'A'
        const char uart2Second[] = "second";//second to send. will receive 'A', will send A+1
        char firstOrSecond[6];
        if(1 != cmdParams.scanf("%*s %s", &firstOrSecond))
        {
            return false;
        }

        if(strcmp(uart2First,firstOrSecond) == 0 || strcmp(uart2Second,firstOrSecond) == 0)
        {
            uart2Homework(firstOrSecond);
            return true;
        }

        else
        {
            printf("Invalid parameters\n\r");
            return true;
        }

    }
    
    else
    {
        printf("Invalid parameters\n\r");
        return true;
    }
}

//start of uart3 handlers function
CMD_HANDLER_FUNC(uart3HwHandler)
{
    //to check if the uart2 driver was initialized
    static bool uart3Init = false;
    if(cmdParams == "init")
    {
        initUART3();
        isr_register(UART3_IRQn, UART3_IRQHandler);
        uart3Init = true;
        printf("GPIO UART3 Initialized!\n\r");
        return true;
    }

    //Enable/Disable Interrupt
    else if(cmdParams.beginsWith("-b")) //configure baud rate
    {
        if(uart3Init == false)
        {
            printf("UART3 needs to be initialized first.\n\r");
            return true;
        }
        unsigned int baudRate = 0;

        if(1 != cmdParams.scanf("%*s %u", &baudRate))
        {
            return false;
        }

        if(baudRate > 500000)
        {
            printf("Baud Rate needs to be less than 500000");
            return true;
        }

        else
        {
            uart3Baud(baudRate);
            printf("Baud Rate was set to %u", baudRate);
            return true;
        }
    }

    //Enable/Disable Interrupt
    else if(cmdParams.beginsWith("-i"))
    {
        if(uart3Init == false)
        {
            printf("UART3 needs to be initialized first.\n\r");
            return true;
        }

        const char u3on[] = "on";
        const char u3off[] = "off";
        char onOff[3];
        if(1 != cmdParams.scanf("%*s %s", &onOff))
        {
            printf("Invalid. -i on or -i off\n\r");
            return true;
        }

        if(strcmp(onOff,u3on) == 0)
        {
            //turn interrupt on
            NVIC_EnableIRQ(UART3_IRQn);
            printf("UART3 Interrupt Enabled!\n\r");
            return true;
        }

        else if(strcmp(onOff,u3off) == 0)
        {
            //turn interrupt off
            NVIC_DisableIRQ(UART3_IRQn);
            printf("UART3 Interrupt Disabled!\n\r");
            return true;
        }

        else
        {
            //invalid
            printf("Invalid. -i on or -i off\n\r");
            return true;
        }

    }

    //used for sending bytes
    else if(cmdParams.beginsWith("-s"))
    {
        if(uart3Init == false)
        {
            printf("UART3 needs to be initialized first.\n\r");
            return true;
        }

        char sendString[256];
        if(1 != cmdParams.scanf("%*s %s", &sendString))
        {
            return false;
        }

        if(strlen(sendString) <= 0)
        {
            printf("no string was entered\n\r");
            return true;
        }

        printf("String Entered Was: %s\n\r", sendString);

        for(unsigned int i = 0; i < strlen(sendString); i++)
        {
            uart3SendByte(sendString[i]);
        }
        uart3SendByte('\n');
        uart3SendByte('\r');
        printf("Sent.\n\r");
        return true;
    }

    //uart3 homework, sends alphabets from A to Z back and forth
    else if(cmdParams.beginsWith("-h"))
    {
        if(uart3Init == false)
        {
            printf("UART3 needs to be initialized first.\n\r");
            return true;
        }

        const char uart3First[] = "first"; //first to send. will send an 'A'
        const char uart3Second[] = "second";//second to send. will receive 'A', will send A+1
        char firstOrSecond[6];
        if(1 != cmdParams.scanf("%*s %s", &firstOrSecond))
        {
            return false;
        }

        if(strcmp(uart3First,firstOrSecond) == 0 || strcmp(uart3Second,firstOrSecond) == 0)
        {
            uart3Homework(firstOrSecond);
            return true;
        }

        else
        {
            printf("Invalid parameters\n\r");
            return true;
        }

    }

    else
    {
        printf("Invalid parameters\n\r");
        return true;
    }
}
//End of "handlers.cpp" and "handlers.hpp" code

//Start of "terminal.cpp" code
cp.addHandler(uart2HwHandler, "uart2", "UART2 Driver Homework");
cp.addHandler(uart3HwHandler, "uart3", "UART3 Driver Homework");
//End of "terminal.cpp" code
