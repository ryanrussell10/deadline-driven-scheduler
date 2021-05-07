/* ECE 455 Project 2
 * Ryan Russell (V00873387)
 * April 15th, 2021
 */

#include <definitions.h>

static dd_list active_list;
static dd_list completed_list;
static dd_list overdue_list;

static QueueHandle_t scheduler_queue;
static QueueHandle_t monitor_queue;

/*-------------------------- DD Scheduler Code ------------------------------*/

void schedulerTask( void *pvParameters ) {

	message_t message;
    task_t t_handle = NULL;

    while(1) {

        if (xQueueReceive(scheduler_queue, (void*)&message, portMAX_DELAY ) == pdTRUE) {

        	// Clean up the active list and remove any overdue tasks
            taskListCleanup(&active_list, &overdue_list);
            while (overdue_list.length > 8) taskListRemove(NULL, &overdue_list, false, true);

			if (message.type == CREATE) {

				t_handle = (task_t)message.data;
				taskListInsert(t_handle, &active_list);
				uint16_t check = 1;

				// Create a software timer if the task is aperiodic
				if (t_handle->type == APERIODIC) {

					char name[20] = "Aperiodic Timer";
					TickType_t period = t_handle->abs_deadline - xTaskGetTickCount();
					t_handle->ap_timer = xTimerCreate(name, period, pdFALSE, (void*)t_handle, apTimerCallback);
					xTimerStart(t_handle->ap_timer, 0);
				}

				char name[32] = "";
				strcat(name, t_handle->name);

				// Inform the task queue of the updates
				if (name[14] == '1') {
					xQueueOverwrite(queue_t1, &check);
				} else if (name[14] == '2') {
					xQueueOverwrite(queue_t2, &check);
				} else if (name[14] == '3') {
					xQueueOverwrite(queue_t3, &check);
				} else {
					xQueueOverwrite(queue_ap, &check);
				}

			} else if (message.type == DELETE) {

				// Remove the task from the active list and add it to the completed list
				taskListInsert(message.sender, &completed_list);
				taskListRemove(message.sender, &active_list, false, false);
				uint16_t check = 1;

				char name[32] = "";
				strcat(name, t_handle->name);

				// Inform the task queue of the updates
				if (name[14] == '1') {
					xQueueOverwrite(queue_t1, &check);
				} else if (name[14] == '2') {
					xQueueOverwrite(queue_t2, &check);
				} else if (name[14] == '3') {
					xQueueOverwrite(queue_t3, &check);
				} else {
					xQueueOverwrite(queue_ap, &check);
				}

			} else if (message.type == ACTIVE) {

				// Store the active list from the message data
				message.data = (void*)taskListReturnMessages(&active_list);
				if (uxQueueSpacesAvailable(monitor_queue) == 0) xQueueReset(monitor_queue);

				// Send on the monitor queue so the monitor task can observe the list
				if (monitor_queue != NULL) {
					if (xQueueSend(monitor_queue, &message, (TickType_t) portMAX_DELAY ) != pdPASS) return;
				} else {
					printf("Error: Monitor Queue has not been created.\n");
					return;
				}

			} else if (message.type == COMPLETED) {

				// Store the overdue list from the message data
				message.data = (void*)taskListReturnMessages(&completed_list);
				if (uxQueueSpacesAvailable(monitor_queue) == 0) xQueueReset(monitor_queue);

				// Send on the monitor queue so the monitor task can observe the list
				if (monitor_queue != NULL) {
					if (xQueueSend(monitor_queue, &message, (TickType_t) portMAX_DELAY ) != pdPASS) return;
				} else {
					printf("Error: Monitor Queue has not been created.\n");
					return;
				}

			} else if (message.type == OVERDUE) {

				// Store the overdue list from the message data
				message.data = (void*)taskListReturnMessages(&overdue_list);
				if (uxQueueSpacesAvailable(monitor_queue) == 0) xQueueReset(monitor_queue);

				// Send on the monitor queue so the monitor task can observe the list
				if (monitor_queue != NULL) {
					if (xQueueSend(monitor_queue, &message, (TickType_t) portMAX_DELAY ) != pdPASS) return;
				} else {
					printf("Error: Monitor Queue has not been created.\n");
					return;
				}
			}
        }
    }
}

void initScheduler() {

    initTaskList(&active_list);
    initTaskList(&completed_list);
    initTaskList(&overdue_list);

    scheduler_queue = xQueueCreate(PRIORITY_RANGE, sizeof(message_t));
    monitor_queue = xQueueCreate(2, sizeof(message_t));

    vQueueAddToRegistry(scheduler_queue,"Scheduler Queue");
    vQueueAddToRegistry(monitor_queue,"monitorTask Queue");

    xTaskCreate(schedulerTask, "DD Scheduler Task", configMINIMAL_STACK_SIZE, NULL, SCHED_PRIORITY, NULL);
    xTaskCreate(monitorTask, "monitorTask Task", configMINIMAL_STACK_SIZE, NULL, MONITOR_PRIORITY, NULL);

}

void createDDTask(task_t task) {

	// Create the task and then suspend it until it has been scheduled
    if (task == NULL) return;
    xTaskCreate(task->task_func, task->name, configMINIMAL_STACK_SIZE, (void*)task, MIN_DD_PRIORITY, &(task->handle));
    if (task->handle == NULL) return;
    vTaskSuspend(task->handle);

    message_t create_msg = {CREATE, xTaskGetCurrentTaskHandle(), task};
    uint16_t check = 0;

    // Send the creation message to the scheduler so the new task can be scheduled
    if (scheduler_queue != NULL) {
    	if (xQueueSend(scheduler_queue, &create_msg, portMAX_DELAY) != pdPASS) return;
    } else {
    	printf("Error: Scheduler Queue has not been created.\n");
		return;
    }

	char name[32] = "";
	strcat(name, task->name);

	// Inform the task queue of the updates
	if (name[14] == '1') {
		xQueueReceive(queue_t1, &check, portMAX_DELAY);
	} else if (name[14] == '2') {
		xQueueReceive(queue_t2, &check, portMAX_DELAY);
	} else if (name[14] == '3') {
		xQueueReceive(queue_t3, &check, portMAX_DELAY);
	} else {
		xQueueReceive(queue_ap, &check, portMAX_DELAY);
	}

	// Resume the task's execution
    vTaskResume(task->handle);
    return;
}

void deleteDDTask(TaskHandle_t task, char task_val) {

    if (task == NULL) return;

    message_t del_msg = {DELETE, task, NULL};
    uint16_t check = 0;

    // Send the deletion message to the scheduler
	if (scheduler_queue != NULL) {
		if (xQueueSend(scheduler_queue, &del_msg, portMAX_DELAY) != pdPASS) return;
	} else {
		printf("Error: Scheduler Queue has not been created.\n");
		return;
	}

	// Inform the task queue of the updates
	if (task_val == '1') {
		xQueueReceive(queue_t1, &check, portMAX_DELAY);
	} else if (task_val == '2') {
		xQueueReceive(queue_t2, &check, portMAX_DELAY);
	} else if (task_val == '3') {
		xQueueReceive(queue_t3, &check, portMAX_DELAY);
	} else {
		xQueueReceive(queue_ap, &check, portMAX_DELAY);
	}

	// Delete the actual F-Task
    vTaskDelete(task);
    return;
}

void getActiveDDTaskList(void) {

	message_t active_msg = {ACTIVE, NULL, NULL};

	if (scheduler_queue != NULL) {
		if (xQueueSend(scheduler_queue, &active_msg, portMAX_DELAY) != pdPASS) return;
	} else {
		printf("Error: Scheduler Queue has not been created.\n");
		return;
	}

	// Retrieve the list from the monitor queue and print it out
	if (monitor_queue != NULL) {
		if (xQueueReceive(monitor_queue, &active_msg, (TickType_t) portMAX_DELAY) == pdTRUE) {
			printf("Active Task List: \n%s\n", (char*)(active_msg.data));
			vPortFree(active_msg.data);
			active_msg.data = NULL;
		}

	} else {
		printf("Error: Monitor Queue has not been created.\n");
	}

	return;
}

void getCompletedDDTaskList(void) {

	message_t completed_msg = {COMPLETED, NULL, NULL};

	if (scheduler_queue != NULL) {
		if (xQueueSend(scheduler_queue, &completed_msg, portMAX_DELAY) != pdPASS) return;
	} else {
		printf("Error: Scheduler Queue has not been created.\n");
		return;
	}

	// Retrieve the list from the monitor queue and print it out
	if (monitor_queue != NULL) {
		if (xQueueReceive(monitor_queue, &completed_msg, (TickType_t) portMAX_DELAY) == pdTRUE) {
			printf("Completed Task List: \n%s\n", (char*)(completed_msg.data));
			vPortFree(completed_msg.data);
			completed_msg.data = NULL;
		}

	} else {
		printf("Error: Monitor Queue has not been created.\n");
	}

	return;
}

void getOverdueDDTaskList(void) {

	message_t overdue_msg = {OVERDUE, NULL, NULL};

	if (scheduler_queue != NULL) {
		if (xQueueSend(scheduler_queue, &overdue_msg, portMAX_DELAY) != pdPASS) return;
	} else {
		printf("Error: Scheduler Queue has not been created.\n");
		return;
	}

    // Retrieve the list from the monitor queue and print it out
    if( monitor_queue != NULL) {
		if(xQueueReceive(monitor_queue, &overdue_msg, (TickType_t) portMAX_DELAY) == pdTRUE) {
			printf("Overdue Task List: \n%s\n", (char*)(overdue_msg.data));
			vPortFree(overdue_msg.data);
			overdue_msg.data = NULL;
		}

    } else {
		printf("Error: Monitor Queue has not been created.\n");
	}

    return;
}

static void apTimerCallback(xTimerHandle ap_timer) {

	task_t task = (task_t)pvTimerGetTimerID(ap_timer);
    xTimerDelete(task->ap_timer, 0);
    task->ap_timer = NULL;
    TaskHandle_t handle = task->handle;
    vTaskSuspend(handle);
    vTaskDelete(handle);
}

/*-------------------------- Monitor Task Code ------------------------------*/

void monitorTask(void *pvParameters) {

	vTaskDelay(1300);

    while (1) {

    	getActiveDDTaskList();
    	getCompletedDDTaskList();
        getOverdueDDTaskList();
        vTaskDelay(100);
    }
}
