/*
 * ot_logging.c — OpenThread logging platform for Zephyr
 *
 * otPlatLog 將訊息格式化後放入 message queue，
 * 由獨立的 log task 從 queue 取出後輸出，避免阻塞呼叫端。
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <zephyr/kernel.h>

#include <openthread/config.h>
#include <openthread/platform/logging.h>

#define LOG_MSG_SIZE   256
#define LOG_QUEUE_DEPTH 8

K_MSGQ_DEFINE(ot_log_msgq, LOG_MSG_SIZE, LOG_QUEUE_DEPTH, 4);

static K_THREAD_STACK_DEFINE(ot_log_stack, 512);
static struct k_thread ot_log_thread;

static void ot_log_task(void *a, void *b, void *c)
{
    char buf[LOG_MSG_SIZE];

    while (1) {
        k_msgq_get(&ot_log_msgq, buf, K_FOREVER);
        printk("%s\r\n", buf);
    }
}

static int ot_log_init(void)
{
    k_thread_create(&ot_log_thread, ot_log_stack,
                    K_THREAD_STACK_SIZEOF(ot_log_stack),
                    ot_log_task, NULL, NULL, NULL,
                    K_LOWEST_APPLICATION_THREAD_PRIO, 0, K_NO_WAIT);
    k_thread_name_set(&ot_log_thread, "ot_log");
    return 0;
}
SYS_INIT(ot_log_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion,
               const char *aFormat, ...)
{
    (void)aLogLevel;
    (void)aLogRegion;

    char buf[LOG_MSG_SIZE];
    va_list args;

    va_start(args, aFormat);
    vsnprintf(buf, sizeof(buf), aFormat, args);
    va_end(args);

    /* queue 滿時丟棄最舊的一條，確保不阻塞呼叫端 */
    if (k_msgq_put(&ot_log_msgq, buf, K_NO_WAIT) != 0) {
        k_msgq_purge(&ot_log_msgq);
        k_msgq_put(&ot_log_msgq, buf, K_NO_WAIT);
    }
}
