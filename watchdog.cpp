/* Add this line to /L5_Application/Source/terminal.cpp
 * cp.addHandler(taskStartSuspend, "task", "Watchdog HW");
 *
 * Add this line to /L5_Application/handlers.hpp
 * CMD_HANDLER_FUNC(taskStartSuspend);
 *
 * Add this function to L2_Drivers/src/rtc.c
 * const char* myTimeFunction(void)
 * {
 *    time_t rawtime;
 *    struct tm* timeinfo;
 *    char returnTimeString[80];
 *    time(&rawtime);
 *    timeinfo=localtime(&rawtime);
 *    strftime(returnTimeString,80,"%H:%M:%S",timeinfo);
 *    return returnTimeString;
 * }
 *
 * Add this line to /L2_Drivers/rtc.h
 * const char* myTimeFunction(void);
 */
 
#include "tasks.hpp"
#include "examples/examples.hpp"
#include "stdio.h"
#include "utilities.h"
#include "i2c2.hpp"
#include "light_sensor.hpp"
#include "uart0_min.h"
#include "eint.h"
#include "storage.hpp"
#include "event_groups.h"
#include <string.h>

#define PRO_BIT (1 << 0)
#define CON_BIT (1 << 1)

TaskHandle_t producerHandle = NULL;
TaskHandle_t consumerHandle = NULL;
QueueHandle_t sensorQ = 0;
EventGroupHandle_t sensorGroup = 0;
EventBits_t sensorBit = 0;

void sensorProducerTask(void* p)
{
    Light_Sensor& LS = Light_Sensor::getInstance();
    SoftTimer sensorTimer(1);
    uint16_t lightSensor = 0;
    uint16_t lightSensorSum = 0;
    while(1)
    {
        for(int i = 0; i < 100; i++)
        {
            sensorTimer.reset();
            while(!sensorTimer.expired()){
                //wait till timer has expired. expired means 1 ms has passed.
            }
            lightSensor = LS.getRawValue();
            lightSensorSum += lightSensor;
            if(i == 99)
            {
                uint16_t average = lightSensorSum/100;
                xQueueSend(sensorQ, &average, portMAX_DELAY);
                lightSensorSum = 0;
            }
            sensorBit =  xEventGroupSetBits(sensorGroup, PRO_BIT);
        }
    }
}

void sensorConsumerTask(void* p)
{
    uint16_t sensorGetAverage = 0;
    char buffer[50] = {};
    uint16_t sizeOfData = 0;
    FILE* sensorFile = fopen("0:sensor.txt","w+");

    if(sensorFile){
        printf("File sensor.txt created\n");
    }

    else{
        printf("creating sensor.txt failed\n");
    }
    while(1)
    {
        if(xQueueReceive(sensorQ, &sensorGetAverage, portMAX_DELAY))
        {
            sizeOfData = sprintf(buffer, "%s %u\n",myTimeFunction(), sensorGetAverage);
        }

        if(Storage::append("0:sensor.txt", &buffer,sizeOfData,0) == FR_OK){

        }
        else{
            printf("writing to sensor.txt failed.\n");
        }

        sensorBit =  xEventGroupSetBits(sensorGroup, CON_BIT);
    }
}

//took this from preets info task
void writeCPUUsage(const char* fileName)
{
    char buffer[1024] = {};
    uint16_t sizeOfData = 0;
    const char * const taskStatusTbl[] = { "RUN", "RDY", "BLK", "SUS", "DEL" };
    // Limit the tasks to avoid heap allocation.
    const unsigned portBASE_TYPE maxTasks = 16;
    TaskStatus_t status[maxTasks];
    uint32_t totalRunTime = 0;
    uint32_t tasksRunTime = 0;
    const unsigned portBASE_TYPE uxArraySize =
    uxTaskGetSystemState(&status[0], maxTasks, &totalRunTime);

    sizeOfData = sprintf(buffer,"%10s Sta Pr Stack CPU%%          Time\n", "Name");
    Storage::append(fileName, &buffer,sizeOfData,0);
    for(unsigned priorityNum = 0; priorityNum < configMAX_PRIORITIES; priorityNum++)
    {
        /* Print in sorted priority order */
        for (unsigned i = 0; i < uxArraySize; i++) {
            TaskStatus_t *e = &status[i];
            if (e->uxBasePriority == priorityNum) {
                tasksRunTime += e->ulRunTimeCounter;

                const uint32_t cpuPercent = (0 == totalRunTime) ? 0 : e->ulRunTimeCounter / (totalRunTime/100);
                const uint32_t timeUs = e->ulRunTimeCounter;
                const uint32_t stackInBytes = (4 * e->usStackHighWaterMark);

                sizeOfData = sprintf(buffer,"%10s %s %2u %5u %4u %10u us\n",
                              e->pcTaskName, taskStatusTbl[e->eCurrentState], e->uxBasePriority,
                              stackInBytes, cpuPercent, timeUs);
                Storage::append(fileName, &buffer,sizeOfData,0);
            }
        }
    }
}

void watchdogTask(void* p)
{
    SoftTimer writeCPUTimer(60000); //1 minute
    FILE* cpuFile = fopen("0:cpu.txt","w+");
    FILE* stuckFile = fopen("0:stuck.txt","w+");
    char producerStuckString[] = "Producer Task is Stuck\n";
    char consumerStuckString[] = "Consumer Task is Stuck\n";
    char newLineForAppendFunction[1] = {'\n'};
    uint16_t producerStuckStringLength = strlen(producerStuckString);
    uint16_t consumerStuckStringLength = strlen(consumerStuckString);

    //Following if/else is just to check if the files were created successfully
    if(!cpuFile){
        printf("creating cpu.txt failed\n");
    }

    else{
        //file was created successfully
    }

    if(!stuckFile){
        printf("creating stuck.txt failed\n");
    }

    else{
        //file was created successfully
    }

    writeCPUTimer.reset();
    while(1)
    {
        //get group bits
        sensorBit = xEventGroupWaitBits(sensorGroup, (PRO_BIT | CON_BIT), pdTRUE, pdTRUE, 1000);

        //Check if sensor producer bit is set, if not set something went wrong.
        if((sensorBit & PRO_BIT) == 0)
        {
            if(Storage::append("0:stuck.txt", &producerStuckString, producerStuckStringLength,0) == FR_OK)
            {
               //Success
            }

            else
            {
                printf("producer is not stuck, but failed to write to file.\n");
            }
        }
        else{
            //producer is stuck
        }

        //Check if sensor consumer bit is set, if not set something went wrong.
        if((sensorBit & CON_BIT) == 0)
        {
            if(Storage::append("0:stuck.txt", &consumerStuckString, consumerStuckStringLength,0) == FR_OK)
            {
               //Success
            }

            else
            {
                printf("consumer is not stuck, but failed to write to file.\n");
            }
        }

        else{
            //consumer is stuck
        }

        //Check timer to see if we need to write CPU info to file
        if(writeCPUTimer.expired())
        {
            writeCPUUsage("0:cpu.txt");
            Storage::append("0:cpu.txt", &newLineForAppendFunction,1,0);
            writeCPUTimer.reset();
        }
    }
}

CMD_HANDLER_FUNC(taskStartSuspend)
{
    char taskName[256];

    //get Task Name
    if(1 != cmdParams.scanf("%*s %s", &taskName))
    {
        //did not enter correct amount of paramters
        return false;
    }

    if(cmdParams.beginsWith("suspend"))
    {
        if(strcmp(taskName,"pro") == 0)
        {
            //suspend producer
            vTaskSuspend(producerHandle);
            printf("Producer Task was suspended\n");
            return true;
        }

        else if(strcmp(taskName,"con") == 0)
        {
            //suspend consumer
            vTaskSuspend(consumerHandle);
            printf("Consumer Task was suspended\n");
            return true;
        }

        else
        {
            //error
            return false;
        }
    }

    else if(cmdParams.beginsWith("resume"))
    {
        if(strcmp(taskName,"pro") == 0)
        {
            //resume producer
            vTaskResume(producerHandle);
            printf("Producer Task is resumed\n");
            return true;
        }

        else if(strcmp(taskName,"con") == 0)
        {
            //resume consumer
            vTaskResume(consumerHandle);
            printf("Consumer Task is resumed\n");
            return true;
        }

        else
        {
            //error
            return false;
        }
    }
    else
    {
        return false;
    }
}

int main(void)
{
    sensorQ = xQueueCreate(1, sizeof(uint16_t));
    sensorGroup = xEventGroupCreate();
    xTaskCreate(sensorProducerTask, "sensorproducer", STACK_BYTES(2048), 0,PRIORITY_MEDIUM,&producerHandle);
    xTaskCreate(sensorConsumerTask, "sensorconsumer", STACK_BYTES(2048), 0,PRIORITY_MEDIUM,&consumerHandle);
    xTaskCreate(watchdogTask, "watchdog", STACK_BYTES(2048*2),0,PRIORITY_HIGH,0);
    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
    scheduler_start();
    return -1;
}
