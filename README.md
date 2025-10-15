# Classes for Simplifying Real-Time Process Management on ESP32
To add to a project in the components folder from the command line, run:    

    git submodule add https://github.com/rbliznets/esp32-task task

## CBaseTask
A FreeRTOS task wrapper with a data exchange template via queue and/or notification. Аn abstract base class for creating FreeRTOS tasks on ESP32. It provides:
- **Message Passing**: A built-in queue (QueueHandle_t) and a *STaskMessage* structure for sending data between tasks. It supports sending messages to the front/back of the queue and from ISRs.
- **Task Lifecycle**: Methods for initialization (init) and cleanup (destructor), including pinning tasks to specific CPU cores.
- **Notifications**: Optional integration with FreeRTOS task notifications (mNotify).
- **Memory Management**: Helper function (allocNewMsg) for allocating message body memory, potentially from PSRAM.
- **Core Logic**: Requires derived classes to implement the *run()* method, which contains the task's main loop.
- **Utilities**: Helper methods like *isRun()* and *getTask()*.
Essentially, it's a framework for managing ESP-IDF tasks with integrated messaging and optional notifications.

Example using only a queue:  

    bool CTestTask::sendMessage(STaskMessage* msg,TickType_t xTicksToWait=0, bool free=false)
	{
		return CBaseTask::sendMessage(msg, 0, xTicksToWait, free);
	};

    void CTestTask::run()
    {
        STaskMessage msg;
        while(getMessage(&msg,portMAX_DELAY))
        {
            switch(msg.msgID)
            {
                case ...:
                    ....
                break;
                default:
                    ESP_LOGW("CTestTask", "unknown message %d", msg.msgID);
                    break;
            }
        }
    }
Example using both queue and notification:  

    #define TESTTASK_QUEUE_BIT 			(31)	
    
    bool CTestTask::sendMessage(STaskMessage* msg,TickType_t xTicksToWait=0, bool free=false)
	{
		return CBaseTask::sendMessage(msg, BIT(TESTTASK_QUEUE_BIT),xTicksToWait,free);
	};

    void CTestTask::run()
    {
	    uint32_t flags;
        STaskMessage msg;
        if(xTaskNotifyWait(0,0xffffffff,&flags,portMAX_DELAY) == pdTRUE)
        {
            if((flags & BIT(TESTTASK_QUEUE_BIT)) != 0)
            {
                while(getMessage(&msg,0))
                {
                    switch(msg.msgID)
                    {
                        case ...:
                            ....
                        break;
                        default:
                            ESP_LOGW("CTestTask", "unknown message %d", msg.msgID);
                            break;
                    }
                }
            }
        }
    }

## Timer Events
- ***CSoftwareTimer*** - A FreeRTOS timer wrapper.
- ***CDelayTimer*** - A microsecond timer.
## Debug Messages
Used when ESP_LOG is insufficient:
-  Need to accurately measure time between events, including from interrupts
-  Messages from multiple CPU cores
-  Multiple output interfaces or interfaces different from standard ones

The interface for outputting messages is in *ITraceLog.h*, and the functions for outputting are in CTrace.h. Connection is done via ***ADDLOG***.

Output settings are configured via sdkconfig. Initial initialization: ***INIT_TRACE()***.

