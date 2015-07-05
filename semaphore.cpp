//Note: code works but semaphore shouldve have been implemented better

SemaphoreHandle_t buttonSemaphore = NULL;

void port2Pin6Init()
{
    uint32_t pin6 = 1 << 6;
    LPC_GPIO2->FIODIR &= ~(pin6);
}

void port6ButtonPressInterrupt()
{
    LPC_GPIOINT->IO2IntClr = 0xFFFFFFFF;
    long yield = 0;
    xSemaphoreGiveFromISR(buttonSemaphore, &yield);
    portYIELD_FROM_ISR(yield);
}

void port6ButtonPressTask(void* p)
{
    while(1)
    {
        if(xSemaphoreTake(buttonSemaphore, portMAX_DELAY))
        {
            //execute interrupt
        }
    }
}

void beggingForSemaphore(void* p)
{
    SoftTimer myTimer(50);
    while(1)
    {
        if(xSemaphoreTake(buttonSemaphore, portMAX_DELAY))
        {
            uart0_puts("TASK HAS SEMAPHORE!");
            xSemaphoreGive(buttonSemaphore);
            myTimer.reset();
            while(!myTimer.expired())
            {

            }
        }
    }
}

int main(void)
{
    buttonSemaphore = xSemaphoreCreateBinary();
    port2Pin6Init();
    eint3_enable_port2(6, eint_falling_edge, port6ButtonPressInterrupt);
    xTaskCreate(port6ButtonPressTask, "Button Press", STACK_BYTES(2048), 0, PRIORITY_HIGH, 0);
    xTaskCreate(beggingForSemaphore, "I NEED SEMAPHOREZ", STACK_BYTES(2048), 0, PRIORITY_HIGH, 0);
    vTaskStartScheduler();
    return -1;
}
