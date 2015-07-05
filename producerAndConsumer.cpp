typedef enum
{
    invalid,
    up, // z is positive
    down, //z is negative
    left, //x is negative
    right, // x is positive
    front, //y is positive
    back, //y is negative

} orientation_t;

orientation_t calcOrient(int16_t xInput, int16_t yInput, int16_t zInput);
const char* printOrient(orientation_t orientationInput);
void orientConsumer(void* p);
void orientProducer(void* p);
uint8_t LOL(orientation_t currentOrient, uint8_t count);

QueueHandle_t q = 0;

int main(void)
{
    q = xQueueCreate(1, sizeof(orientation_t));
    
    //PRIORITY CAN BE SWITCHED HERE TO FOLLOW ASSIGNMENT
    xTaskCreate(orientConsumer, "Consumer", STACK_BYTES(2048), 0, PRIORITY_LOW, 0);
    xTaskCreate(orientProducer, "Producer", STACK_BYTES(2048), 0, PRIORITY_HIGH, 0);
    
    vTaskStartScheduler();
    return -1;
}

orientation_t calcOrient(int16_t xInput, int16_t yInput, int16_t zInput)
{
    int16_t xMag = xInput;
    int16_t yMag = yInput;
    int16_t zMag = zInput;

    bool xNeg = false;
    bool yNeg = false;
    bool zNeg = false;

    if(xMag < 0)
    {
        xNeg = true;
        xMag *= -1;
    }

    if(yMag < 0)
    {
        yNeg = true;
        yMag *= -1;
    }

    if(zMag < 0)
    {
        zNeg = true;
        zMag *= -1;
    }

    if(xMag > yMag) // x is bigger than y
    {
        if(xMag > zMag) //x is the biggest
        {
            if(xNeg == true)
            {
                return left;
            }

            else
            {
                return right;
            }
        }

        else //z is the biggest
        {
            if(zNeg == true)
            {
                return down;
            }

            else
            {
                return up;
            }
        }
    }
    else //y is bigger than x
    {
        if(yMag > zMag) //y is the biggest
        {
            if(yNeg == true)
            {
                return back;
            }

            else
            {
                return front;
            }
        }

        else // z is the biggest
        {
            if(zNeg == true)
            {
                return down;
            }

            else
            {
                return up;
            }
        }
    }
}

const char* printOrient(orientation_t orientationInput)
{
    switch(orientationInput)
    {
        case invalid:
            return "Invalid";
            break;
        case up:
            return "Up";
            break;
        case down:
            return "Down";
            break;
        case left:
            return "Left";
            break;
        case right:
            return "Right";
            break;
        case front:
            return "Front";
            break;
        case back:
            return "Back";
            break;
        default:
            break;
    }

    return "Invalid";
}

void orientConsumer(void* p)
{
    orientation_t orientGet = invalid;

    while(1)
    {
        xQueueReceive(q, &orientGet, portMAX_DELAY);
        uart0_puts(printOrient(orientGet));
    }
}

void orientProducer(void* p)
{
    orientation_t orient = invalid;
    Acceleration_Sensor& accel = Acceleration_Sensor::getInstance();
    accel.init();
    int16_t xAccel = 0;
    int16_t yAccel = 0;
    int16_t zAccel = 0;

    uint8_t cheatCode = 0;
    while(1)
    {
        xAccel = accel.getX();
        yAccel = accel.getY();
        zAccel = accel.getZ();

        orient = calcOrient(xAccel, yAccel, zAccel);

        cheatCode = LOL(orient, cheatCode);
        if(cheatCode == 5)
        {
            cheatCode = 0;
            dancingLights(); //in another file
        }

        uart0_puts("Sending");
        xQueueSend(q, &orient, portMAX_DELAY);
        uart0_puts("Sent");
        vTaskDelay(1000);
    }
}

uint8_t LOL(orientation_t currentOrient, uint8_t count)
{
    //BOARD ORIENTATION COMBINATION TO SEE LIGHTS
    //UP,DOWN,LEFT,RIGHT,UP
    switch(count)
    {
        case 0:
            if(currentOrient == up) return 1;
            else return 0;

        case 1:
            if(currentOrient == down) return 2;
            else return 0;

        case 2:
            if(currentOrient == left) return 3;
            else return 0;

        case 3:
            if(currentOrient == right) return 4;
            else return 0;

        case 4:
            if(currentOrient == up) return 5;
            else return 0;

        default:
            return 0;
    }
}
