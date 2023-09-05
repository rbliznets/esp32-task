# Классы для упрощения работы с процессами реального времени под esp32
Для добавления в проект в папке компонентов из командной строки запустить:    

    git submodule add https://github.com/rbliznets/esp32-task task
## CBaseTask
Обертка задачи FreeRTOS с шаблоном обмена данными через очередь и/или notification. 
Основная задача определяется в ***run()*** 
Пример только с очередью:  

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
Пример с очередью и notification:  

    #define TESTTASK_QUEUE_BIT 			(31)	///< Номер бита уведомления о сообщении в очереди.
    
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
## События по таймеру
- ***CSoftwareTimer*** - обертка таймера FreeRTOS. Событие через notification.
- ***CDelayTimer*** - микросекундный таймер. Событие через notification.
## Отладочные сообщения
Применяется если ESP_LOG недостаточно:
-  нужно точно замерять время между событиями, в том числе и из прерываний
-  сообщения из нескольких ядер CPU
-  несколько интерфейсов для вывода или они отличаются от стандартных

Интерфейс для вывода сообщений в *ITraceLog.h*, а функции для вывода в *CTrace.h*. Подключение через ***ADDLOG***.

Настройки вывода через sdkconfig. Начальная инициализация: ***INIT_TRACE()***.

