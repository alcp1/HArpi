//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 17/Aug/2025 |                               | ALCP             //
// - First version                                                            //
//----------------------------------------------------------------------------//

#ifndef TIMER_H
#define TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <harpi.h>

//----------------------------------------------------------------------------//
// EXTERNAL DEFINITIONS
//----------------------------------------------------------------------------//    
    
//----------------------------------------------------------------------------//
// EXTERNAL TYPES
//----------------------------------------------------------------------------//
    
//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
/**
 * Create timers
 * 
 * \param       ntimers: number of timers to be created
 * \param       smIDArray: an array with the stateMachineID of each timer
 * 
 **/
void timer_createTimers(int16_t ntimers, int16_t* smIDArray);

/**
 * set timer with specific value
 * \param       stateMachineID: number of timers to be created
 * \param       value: value in 100ms base (ex: 3 = 300ms)
 * 
 **/
void timer_setTimer(int16_t stateMachineID, int16_t value);

/**
 * Periodic update of timers - To be called every 100ms
 **/
void timer_periodic(void);

/**
 * get the timer status of a given stateMachineID
 * \param       stateMachineID: number of timers to be created
 * 
 **/
harpiTimerStatus_t timer_getTimerStatus(int16_t stateMachineID);

#ifdef __cplusplus
}
#endif

#endif

