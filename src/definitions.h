/* ECE 455 Project 2
 * Ryan Russell (V00873387)
 * April 15th, 2021
 */

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4xx.h"
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"

#include "stm32f4_discovery.h"
#include "FreeRTOSHooks.h"

/*--------------------------- Task Priorities --------------------------------*/

# define MIN_DD_PRIORITY    (1)
# define MONITOR_PRIORITY   (2)
//# define MONITOR_PRIORITY   (4)
# define BASE_DD_PRIORITY 	(3)
# define GEN_PRIORITY   	(configMAX_PRIORITIES - 3)
# define SCHED_PRIORITY	   	(configMAX_PRIORITIES - 1)
# define PRIORITY_RANGE 	(SCHED_PRIORITY - 4)

/*--------------------------- Task Periods and Exec Times --------------------*/

// Benchmark 1
//#define period_t1 		(500)
//#define exec_t1   		(95)
//#define period_t2 		(500)
//#define exec_t2   		(150)
//#define period_t3 		(750)
//#define exec_t3   		(250)

// Benchmark 2
#define period_t1 	(250)
#define exec_t1   	(95)
#define period_t2 	(500)
#define exec_t2   	(150)
#define period_t3 	(750)
#define exec_t3   	(250)

// Benchmark 3
//#define period_t1 	(500)
//#define exec_t1   	(100)
//#define period_t2 	(500)
//#define exec_t2   	(200)
//#define period_t3 	(500)
//#define exec_t3   	(200)

#define exec_ap 		(500)
#define deadline_ap 	(900)

/*--------------------------- Task Structs -----------------------------------*/

typedef enum task_type {
    APERIODIC,
	PERIODIC,
	NoType
} task_type;

typedef struct dd_task {
	TaskHandle_t      	handle;
	TaskFunction_t    	task_func;
	task_type    	  	type;
    uint32_t			task_id;
    const char*      	name;
    TickType_t        	release_time;
    TickType_t        	abs_deadline;
    xTimerHandle      	ap_timer;
    struct dd_task* 	next;
    struct dd_task* 	prev;
} dd_task;

typedef dd_task* task_t;

typedef struct dd_list {
    uint32_t    length;
    task_t 		head;
    task_t 		tail;
} dd_list;

typedef dd_list* tasklist;

typedef enum msg_type {
    CREATE,
    DELETE,
    ACTIVE,
	COMPLETED,
    OVERDUE
} msg_type;

typedef struct message_t {
	msg_type 		type;
    TaskHandle_t    sender;
    void*           data;
} message_t;


/*--------------------------- Task List Function Definitions  ----------------------------*/

void initTaskList(tasklist list);
task_t createTask();
bool deleteTask(task_t task);
char* taskListReturnMessages(tasklist list);
void taskListInsert(task_t task , tasklist list);
void taskListRemove(TaskHandle_t task, tasklist list, bool move_lists, bool del_overdue);
void taskListCleanup(tasklist activeList, tasklist overdueList);
void initQueues(void);

/*--------------------------- DD Scheduler Function Definitions  -------------------------*/

void initScheduler( void );
void schedulerTask( void *pvParameters );
void createDDTask(task_t task);
void deleteDDTask(TaskHandle_t task, char task_val);
void getActiveDDTaskList(void);
void getCompletedDDTaskList(void);
void getOverdueDDTaskList(void);
static void apTimerCallback(xTimerHandle xTimer);

/*--------------------------- Monitor Task Definitions -----------------------------------*/

void monitorTask(void *pvParameters);

/*--------------------------- DD Generator Function Definitions  -------------------------*/

void taskGenerator1(void *pvParameters);
void taskGenerator2(void *pvParameters);
void taskGenerator3(void *pvParameters);
void task1(void *pvParameters);
void task2(void *pvParameters);
void task3(void *pvParameters);

void aperiodicTaskGenerator(void *pvParameters);
void aperiodicTask(void *pvParameters);

/*--------------------------- DD Generator Handles  --------------------------------------*/

TaskHandle_t t1_handle;
TaskHandle_t t2_handle;
TaskHandle_t t3_handle;
TaskHandle_t ap_handle;

/*--------------------------- Queue Handles -----------------------------------*/

QueueHandle_t queue_t1;
QueueHandle_t queue_t2;
QueueHandle_t queue_t3;
QueueHandle_t queue_ap;


#endif
