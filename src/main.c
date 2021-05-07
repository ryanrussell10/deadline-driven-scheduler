/* ECE 455 Project 2
 * Ryan Russell (V00873387)
 * April 15th, 2021
 */

#include <definitions.h>

/*-------------------------- Main Function ----------------------------------*/

int main(void) {

	initQueues();
    initScheduler();

    xTaskCreate(taskGenerator1, "Task Generator 1", configMINIMAL_STACK_SIZE, NULL, GEN_PRIORITY, &t1_handle);
    xTaskCreate(taskGenerator2, "Task Generator 2", configMINIMAL_STACK_SIZE, NULL, GEN_PRIORITY, &t2_handle);
    xTaskCreate(taskGenerator3, "Task Generator 3", configMINIMAL_STACK_SIZE, NULL, GEN_PRIORITY, &t3_handle);
    //xTaskCreate(aperiodicTaskGenerator, "AperiodicCreator", configMINIMAL_STACK_SIZE, NULL, GEN_PRIORITY, &ap_handle);

    vTaskStartScheduler();

    return 0;
}

/*-------------------------- Task List Code ---------------------------------*/

void initTaskList(tasklist list) {

	if (list == NULL)
		return;

    list->length = 0;
    list->head = NULL;
    list->tail = NULL;
}

task_t createTask() {

	task_t new_task = (task_t)pvPortMalloc(sizeof(dd_task));

	// Initialize task with default values
	new_task->handle = NULL;
	new_task->task_func = NULL;
	new_task->type = NoType;
	new_task->task_id = 0;
    new_task->name = "";
    new_task->release_time = 0;
    new_task->abs_deadline = 0;
    new_task->ap_timer = NULL;
    new_task->next = NULL;
    new_task->prev = NULL;

    return new_task;
}

bool deleteTask(task_t del_task) {

	if (del_task == NULL) {
		printf("deleteTask: task passed in was NULL.\n");
		return false;
	}

	if (del_task->next != NULL || del_task->prev != NULL) return false;

	// Reset to all default values before deletion
	del_task->handle = NULL;
	del_task->task_func = NULL;
	del_task->type = NoType;
	del_task->task_id = 0;
	del_task->name = "";
	del_task->release_time = 0;
	del_task->abs_deadline = 0;
	del_task->ap_timer = NULL;
	del_task->next = NULL;
	del_task->prev = NULL;

    vPortFree((void*)del_task);
    return true;
}

char* taskListReturnMessages(tasklist list) {

    uint32_t size = list->length;
	char* out_buf = (char*)pvPortMalloc(((configMAX_TASK_NAME_LEN + 50) * (size + 1)));
	out_buf[0] = '\0';

	// Return either a string stating that the list is empty or the requested task list
    if (size == 0) {
    	char empty_string[21] = ("List is empty.\n");
    	strcat(out_buf, empty_string);
    } else {
    	task_t cur_task = list->head;

		while (cur_task != NULL) {

			// Store each task in a buffer to be returned and output later
			char cur_buf[60];
			sprintf(cur_buf, "Task: %s. Deadline: %u \n", cur_task->name, (unsigned int) cur_task->abs_deadline);
			strcat(out_buf, cur_buf);
			cur_task = cur_task->next;
		}
    }
    return out_buf;
}

void taskListInsert(task_t task , tasklist list) {

	if (list == NULL || task == NULL) {
		printf("taskListInsert: task or list passed in was NULL.\n");
		return;
	}

	// If the list is empty we can simply add the new task and return
    if (list->length == 0) {
        list->head = task;
        list->tail = task;
        list->length = 1;
        vTaskPrioritySet(task->handle, BASE_DD_PRIORITY);
        return;
    }

	// Set the current task to the head of the list and store its priority (current highest priority)
    task_t cur_task = list->head;
    uint32_t cur_priority = uxTaskPriorityGet(cur_task->handle);
    cur_priority += 1;

    if(cur_priority == GEN_PRIORITY) return;

    // Search for the proper location to add the new task based on deadlines
    while (cur_task != NULL) {

        if (task->abs_deadline < cur_task->abs_deadline) {

            if (cur_task == list->head) list->head = task;

            task->next = cur_task;
            task->prev = cur_task->prev;
            cur_task->prev = task;

            list->length += 1;
            vTaskPrioritySet(task->handle, cur_priority);
            return;

        } else {

        	// If we have reached the end of the list, insert the new task here
            if (cur_task->next == NULL) {

                list->tail = task;
                task->prev = cur_task;
                task->next = NULL;
                cur_task->next = task;

                list->length += 1;
                vTaskPrioritySet(cur_task->handle, cur_priority);
                vTaskPrioritySet(task->handle, BASE_DD_PRIORITY);
                return;
            }

            vTaskPrioritySet(cur_task->handle, cur_priority);
            cur_task = cur_task->next;
            cur_priority -= 1;
        }
    }
}

void taskListRemove(TaskHandle_t task, tasklist list, bool move_lists, bool del_overdue) {

	if (task == NULL && !del_overdue) {
		printf("taskListRemove: task passed in was NULL.\n");
		return;
	}

	if (list == NULL || list->length == 0) {
		printf("taskListRemove: list is already empty or does not exist.\n");
		return;
	}

	// Set the current task to the head of the list and store its priority (current highest priority)
	task_t cur_task = list->head;
    uint32_t cur_priority = uxTaskPriorityGet(cur_task->handle);

    // Case where the task to remove is the lone task in the list
    if (list->length == 1 && (cur_task->handle == task || del_overdue)) {

		list->length = 0;
    	list->head = NULL;
		list->tail = NULL;
		if(!move_lists) deleteTask(cur_task);
		return;
	}

    // Case where we are removing excess tasks from the overdue list
    if (del_overdue) {

    	list->head = cur_task->next;
    	cur_task->next->prev = NULL;
    	(list->length)--;
    	cur_task->prev = NULL;
    	cur_task->next = NULL;
    	deleteTask(cur_task);
    	return;
    }

    // General case
    while (cur_task != NULL) {

		// Found the task we want to delete
        if (cur_task->handle == task) {

			// If the task is aperiodic we need to stop the timer first
            if ((cur_task->type == APERIODIC) && (cur_task->ap_timer != NULL)) {
                xTimerStop( cur_task->ap_timer, 0 );
                xTimerDelete( cur_task->ap_timer, 0 );
                cur_task->ap_timer = NULL;
            }

            if (task == list->tail->handle) {
            	// Remove the tail of the list
                list->head = cur_task->prev;
                cur_task->prev->next = NULL;
            } else if (task == list->head->handle) {
            	// Remove the head of the list
            	list->head = cur_task->next;
            	cur_task->next->prev = NULL;
            } else {
                cur_task->prev->next = cur_task->next;
                cur_task->next->prev = cur_task->prev;
            }

            (list->length)--;
            if (!move_lists) deleteTask(cur_task);

            return;

        } else {

        	// Iterate to the next task in the list
			cur_priority--;
			vTaskPrioritySet(cur_task->handle, cur_priority);
			cur_task = cur_task->next;
        }
    }

    // Reset all priorities if task not in list
    cur_task = list->tail;
    cur_priority = BASE_DD_PRIORITY;

    while (cur_task != NULL) {
        vTaskPrioritySet(cur_task->handle, cur_priority);
        cur_task = cur_task->prev;
        cur_priority++;
    }
}

void taskListCleanup(tasklist active_list, tasklist overdue_list) {

	if (active_list == NULL || overdue_list == NULL) {
		printf("taskListCleanup: at least one of the lists passed in was NULL.\n");
		return;
	}

	task_t cur_task = active_list->head;
    TickType_t cur_time = xTaskGetTickCount();

    // Iterate through the list and remove overdue tasks from the active list and add to the overdue list
    while (cur_task != NULL) {

        if (cur_task->abs_deadline < cur_time) {

        	taskListRemove(cur_task->handle, active_list, true, false);

        	if (overdue_list->length != 0) {

        		// Add the overdue task to the end of the overdue list
        		task_t temp = overdue_list->tail;
				overdue_list->tail = cur_task;
				temp->next = cur_task;
				cur_task->prev = temp;
				overdue_list->length += 1;

			} else {

				// Add the overdue task as the only element in the overdue list
				overdue_list->length = 1;
				overdue_list->head = cur_task;
				overdue_list->tail = cur_task;
			}

			if (cur_task->type == PERIODIC) {
				vTaskSuspend(cur_task->handle);
				vTaskDelete(cur_task->handle);
			}
        }
        cur_task = cur_task->next;
    }
}

void initQueues(void) {

	queue_t1 = xQueueCreate(1, sizeof(uint16_t));
	queue_t2 = xQueueCreate(1, sizeof(uint16_t));
	queue_t3 = xQueueCreate(1, sizeof(uint16_t));

	if (queue_t1 == NULL) return;
	if (queue_t2 == NULL) return;
	if (queue_t3 == NULL) return;

	vQueueAddToRegistry(queue_t1, "queue_t1");
	vQueueAddToRegistry(queue_t2, "queue_t2");
	vQueueAddToRegistry(queue_t1, "queue_t3");

	return;
}
