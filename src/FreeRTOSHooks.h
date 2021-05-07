/*
 * FreeRTOSHooks.h
 *
 *  Created on: Mar 23, 2019
 *      Author: amcote
 */

#ifndef FREERTOSHOOKS_H_
#define FREERTOSHOOKS_H_

#include <definitions.h>


void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName );
void vApplicationIdleHook( void );


#endif /* FREERTOSHOOKS_H_ */
