/* ECE 455 Project 2
 * Ryan Russell (V00873387)
 * April 15th, 2021
 */

#include <definitions.h>

/*-------------------------- DD Generator Code ------------------------------*/

void taskGenerator1 (void *pvParameters) {

    while (1) {

        TickType_t deadline = period_t1;
        TickType_t cur_time = xTaskGetTickCount();
        task_t new_task = createTask();

        // Initialize new task with task 1 details
        new_task->task_func = task1;
        new_task->type = PERIODIC;
        new_task->name = "Periodic Task 1";
        new_task->task_id = 1;
        new_task->release_time = cur_time;
        new_task->abs_deadline = deadline + cur_time;

        createDDTask(new_task);
        vTaskDelay(period_t1);
    }
}

void taskGenerator2(void *pvParameters) {

    while (1) {

        TickType_t deadline = period_t2;
        TickType_t cur_time = xTaskGetTickCount();
        task_t new_task = createTask();

        // Initialize new task with task 2 details
        new_task->task_func = task2;
        new_task->type = PERIODIC;
        new_task->name = "Periodic Task 2";
        new_task->task_id = 2;
        new_task->release_time = cur_time;
        new_task->abs_deadline = deadline + cur_time;

        createDDTask(new_task);
        vTaskDelay(period_t2);
    }
}

void taskGenerator3(void *pvParameters) {

    while (1) {

        TickType_t deadline = period_t3;
        TickType_t cur_time = xTaskGetTickCount();
        task_t new_task = createTask();

        // Initialize new task with task 3 details
        new_task->task_func = task3;
        new_task->type = PERIODIC;
        new_task->name = "Periodic Task 3";
        new_task->task_id = 3;
        new_task->release_time = cur_time;
        new_task->abs_deadline = deadline + cur_time;

        createDDTask(new_task);
        vTaskDelay(period_t3);
    }
}

void aperiodicTaskGenerator( void *pvParameters ) {

    TickType_t deadline = deadline_ap;

    while (1) {

    	uint16_t check = 0;
    	xQueueReceive(queue_ap, &check, portMAX_DELAY);
        task_t new_task = createTask();

        // Initialize new task with aperiodic task details
        new_task->task_func = aperiodicTask;
        new_task->type = APERIODIC;
        new_task->name = "Aperiodic Task";

        TickType_t cur_time = xTaskGetTickCount();
        new_task->release_time = cur_time;
        new_task->abs_deadline = deadline + cur_time;

        createDDTask(new_task);
    }
}

/*-------------------------- User-Defined Task Code -------------------------*/

void task1(void *pvParameters) {

	task_t cur_task = (task_t)pvParameters;
	TickType_t exec_time = exec_t1 / portTICK_PERIOD_MS;
	TickType_t cur_time = 0;
	TickType_t prev_time = 0;
	TickType_t rem_time = 0;

    while (1) {

    	// Begin task execution
    	cur_time = xTaskGetTickCount();
    	prev_time = cur_time;
    	printf("\nTask 1: starting execution at %u ms. Priority: %u\n", (unsigned int)cur_time, (unsigned int)uxTaskPriorityGet(NULL));

    	// Ensure that the task executes for the specified duration
        for (int i = 0; i < exec_time; i++) {

        	cur_time = xTaskGetTickCount();
			if (cur_time == prev_time) i--;
			prev_time = cur_time;
        }

        // Complete execution and wait for the deadline
        cur_time = xTaskGetTickCount();
		printf("\nTask 1: completed execution at %u ms.\n", (unsigned int)cur_time);
        rem_time = cur_task->abs_deadline - cur_time;
        if (rem_time != 0) vTaskDelayUntil(&cur_time, rem_time);
        deleteDDTask(xTaskGetCurrentTaskHandle(), '1');
    }
}

void task2(void *pvParameters) {

	task_t cur_task = (task_t)pvParameters;
	TickType_t exec_time = exec_t2 / portTICK_PERIOD_MS;
	TickType_t cur_time = 0;
	TickType_t prev_time = 0;
	TickType_t rem_time = 0;

    while (1) {

    	// Begin task execution
    	cur_time = xTaskGetTickCount();
    	prev_time = cur_time;
    	printf("\nTask 2: starting execution at %u ms. Priority: %u\n", (unsigned int)cur_time, (unsigned int)uxTaskPriorityGet(NULL));

    	// Ensure that the task executes for the specified duration
        for (int i = 0; i < exec_time; i++) {

        	cur_time = xTaskGetTickCount();
			if (cur_time == prev_time) i--;
			prev_time = cur_time;
        }

        // Complete execution and wait for the deadline
        cur_time = xTaskGetTickCount();
        printf("\nTask 2: completed execution at %u ms.\n", (unsigned int)cur_time);
        rem_time = cur_task->abs_deadline - cur_time;
        if(rem_time != 0) vTaskDelayUntil(&cur_time, rem_time);
        deleteDDTask(xTaskGetCurrentTaskHandle(), '2');
    }
}

void task3(void *pvParameters) {

	task_t cur_task = (task_t)pvParameters;
	TickType_t exec_time = exec_t3 / portTICK_PERIOD_MS;
	TickType_t cur_time = 0;
	TickType_t prev_time = 0;
	TickType_t rem_time = 0;

    while (1) {

    	// Begin task execution
    	cur_time = xTaskGetTickCount();
    	prev_time = cur_time;
    	printf("\nTask 3: starting execution at %u ms. Priority: %u\n", (unsigned int)cur_time, (unsigned int)uxTaskPriorityGet(NULL));

    	// Ensure that the task executes for the specified duration
        for (int i = 0; i < exec_time; i++) {

        	cur_time = xTaskGetTickCount();
			if (cur_time == prev_time) i--;
			prev_time = cur_time;
        }

        // Complete execution and wait for the deadline
        cur_time = xTaskGetTickCount();
        printf("\nTask 3: completed execution at %u ms.\n", (unsigned int)cur_time);
        rem_time = cur_task->abs_deadline - cur_time;
        if(rem_time != 0) vTaskDelayUntil(&cur_time, rem_time);
        deleteDDTask(xTaskGetCurrentTaskHandle(), '3');
    }
}


void aperiodicTask (void *pvParameters) {

	task_t cur_task = (task_t)pvParameters;
	TickType_t exec_time = exec_ap / portTICK_PERIOD_MS;
	TickType_t cur_time = 0;
	TickType_t prev_time = 0;
	TickType_t rem_time = 0;

    while (1) {

    	// Begin task execution
    	cur_time = xTaskGetTickCount();
    	prev_time = cur_time;
    	printf("\nAperiodic Task: starting execution at %u ms. Priority: %u\n", (unsigned int)cur_time, (unsigned int)uxTaskPriorityGet( NULL ) );

    	// Ensure that the task executes for the specified duration
        for (int i = 0; i < exec_time; i++) {

			cur_time = xTaskGetTickCount();
			if (cur_time == prev_time) i--;
			prev_time = cur_time;
		}

        // Complete execution and wait for the deadline
        rem_time = cur_task->abs_deadline - cur_time;
        vTaskDelayUntil(&cur_time, rem_time);
        deleteDDTask(xTaskGetCurrentTaskHandle(), 'A');
    }
}
