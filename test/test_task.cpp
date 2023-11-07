/*!
  \file
  \brief Модульные тесты для task.
  \authors Близнец Р.А.
  \version 0.0.0.1
  \date 05.05.2022
  \copyright (c) Copyright 2022, ООО "Глобал Ориент", Москва, Россия, http://www.glorient.ru/
*/

#include <limits.h>
#include <cstring>
#include "unity.h"
#include "CSoftwareTimer.h"
#include "CDelayTimer.h"
#include "CTrace.h"
#include "baseTaskTest.h"
#include "unity_test_utils_memory.h"

#define countof(x) (sizeof(x) / sizeof(x[0]))

/// Тест таймера FreeRTOS.
TEST_CASE("CSoftwareTimer", "[task]")
{
  unity_utils_record_free_mem();

  CSoftwareTimer *tm = nullptr;
  tm = new CSoftwareTimer(1);
  TEST_ASSERT_TRUE(tm != nullptr);

  STARTTIMESHOT();
  TEST_ASSERT_EQUAL_INT(0, tm->start(100));
  uint32_t flag = 0;
  TEST_ASSERT_TRUE(xTaskNotifyWait(0, (1 << 1), &flag, pdMS_TO_TICKS(500)));
  STOPTIMESHOT("100mSec time");

  STARTTIMESHOT();
  TEST_ASSERT_EQUAL_INT(0, tm->start(100, true));
  flag = 0;
  TEST_ASSERT_TRUE(xTaskNotifyWait(0, (1 << 1), &flag, pdMS_TO_TICKS(500)));
  flag = 0;
  TEST_ASSERT_TRUE(xTaskNotifyWait(0, (1 << 1), &flag, pdMS_TO_TICKS(500)));
  flag = 0;
  TEST_ASSERT_TRUE(xTaskNotifyWait(0, (1 << 1), &flag, pdMS_TO_TICKS(500)));
  STOPTIMESHOT("300mSec time");

  TEST_ASSERT_EQUAL_INT(0, tm->stop());
  delete tm;
  vTaskDelay(pdMS_TO_TICKS(10));

  unity_utils_evaluate_leaks_direct(0);
#ifdef CONFIG_HEAP_POISONING_COMPREHENSIVE
  heap_caps_check_integrity_all(true);
#endif
}

TEST_CASE("CDelayTimer", "[task]")
{
  unity_utils_record_free_mem();

  CDelayTimer *tm = new CDelayTimer();

  uint32_t flag = 0;
  STARTTIMESHOT();
  TEST_ASSERT_EQUAL_INT(0, tm->start(1, 250));
  TEST_ASSERT_TRUE(xTaskNotifyWait(0, (1 << 1), &flag, pdMS_TO_TICKS(10)));
  STOPTIMESHOT("250usec time");

  STARTTIMESHOT();
  TEST_ASSERT_EQUAL_INT(0, tm->start(1, 100 * 1000, true));
  flag = 0;
  TEST_ASSERT_TRUE(xTaskNotifyWait(0, (1 << 1), &flag, pdMS_TO_TICKS(500)));
  flag = 0;
  TEST_ASSERT_TRUE(xTaskNotifyWait(0, (1 << 1), &flag, pdMS_TO_TICKS(500)));
  flag = 0;
  TEST_ASSERT_TRUE(xTaskNotifyWait(0, (1 << 1), &flag, pdMS_TO_TICKS(500)));
  STOPTIMESHOT("300mSec time");

  TEST_ASSERT_EQUAL_INT(0, tm->stop());

  STARTTIMESHOT();
  TEST_ASSERT_EQUAL_INT(0, tm->wait(750));
  STOPTIMESHOT("750usec time");

  vTaskDelay(pdMS_TO_TICKS(10));

  delete tm;

  unity_utils_evaluate_leaks_direct(136);
#ifdef CONFIG_HEAP_POISONING_COMPREHENSIVE
  heap_caps_check_integrity_all(true);
#endif
}

void CBaseTaskTest::run()
{
  uint32_t flags;
  STaskMessage msg;
#ifdef CONFIG_DEBUG_CODE
  TRACE("Task start", 0, false);
#endif

  for (;;)
  {
    if (xTaskNotifyWait(0, 0xffffffff, &flags, portMAX_DELAY) == pdTRUE)
    {
      if ((flags & BASETASKTEST_QUEUE_FLAG) != 0)
      {
        while (getMessage(&msg))
        {
          switch (msg.msgID)
          {
          case MSG_ECHO:
            delete[] msg.msgBody;
            mFlag = true;
            break;
          default:
            // TRACE("Terminate",0,false);
            return;
          }
        }
      }
    }
  }
}

/// Тест CBaseTask.
TEST_CASE("CBaseTask", "[task]")
{
  uint32_t mem1 = esp_get_free_heap_size();

  CBaseTaskTest *tsk = new CBaseTaskTest();
  tsk->init("base", 4096, 3, 10, 0);
  vTaskDelay(pdMS_TO_TICKS(100));
  TEST_ASSERT_TRUE(tsk->isRun());
  delete tsk;
  vTaskDelay(pdMS_TO_TICKS(10));

  STaskMessage msg;
  tsk = new CBaseTaskTest();
  tsk->init("base", 4096, 3, 10, 1);
  TEST_ASSERT_NOT_NULL(tsk->allocNewMsg(&msg, MSG_ECHO, 512));
  TEST_ASSERT_TRUE(tsk->sendMessage(&msg, 10, true));
  vTaskDelay(pdMS_TO_TICKS(10));
  TEST_ASSERT_TRUE(tsk->mFlag);
  msg.msgID = MSG_TERMINATE;
  TEST_ASSERT_TRUE(tsk->sendMessage(&msg, 10));
  vTaskDelay(pdMS_TO_TICKS(10));
  delete tsk;
  vTaskDelay(pdMS_TO_TICKS(10));

  uint32_t mem2 = esp_get_free_heap_size();
  if (mem1 != mem2)
  {
    TRACE("memory leak", mem1 - mem2, false);
    TRACE("start", mem1, false);
    TRACE("stop", mem2, false);
    TEST_FAIL_MESSAGE("memory leak");
  }
}
