//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 30/Jul/2025 |                               | ALCP             //
// - First Version                                                            //
//----------------------------------------------------------------------------//


#ifndef HARPI_H
#define HARPI_H

#ifdef __cplusplus
extern "C" {
#endif
    
/*
* Includes
*/
#include <hapcan.h>
    
//----------------------------------------------------------------------------//
// EXTERNAL DEFINITIONS
//----------------------------------------------------------------------------//
    
//----------------------------------------------------------------------------//
// EXTERNAL TYPES
//----------------------------------------------------------------------------//
typedef enum
{
  HARPI_LOAD_TYPE_RELAY = 0, // "Relay"
  HARPI_LOAD_TYPE_OTHER
}harpiLoadType_t;

// State Machines and Loads 
typedef struct  
{
    int16_t stateMachineID;
    harpiLoadType_t type;
    uint8_t node;
    uint8_t group;
    uint8_t channel;
} harpiSMLoadsData;

// State Machines and Events
typedef struct  
{
    int16_t stateMachineID;
    int16_t eventSetID;
} harpiSMEventsData;

// Action Sets
typedef struct  
{
    int16_t actionsSetID;
    hapcanCANData frame;
} harpiActionSetsData;

// Event Sets
typedef struct  
{
    int16_t eventsSetID;
    uint8_t fiterCondition[HAPCAN_FULL_FRAME_LEN];
    uint8_t fiter[HAPCAN_FULL_FRAME_LEN];
} harpiEventSetsData;

//States and Actions
typedef struct  
{
    int16_t stateMachineID;
    int16_t currentStateID;
    int16_t eventSetID;
    int16_t actionsSetID;
} harpiStateActionsData;

// State Transitions
typedef struct  
{
    int16_t stateMachineID;
    int16_t currentStateID;
    int16_t eventSetID;
    int16_t newStateID;
} harpiStateTransitionsData;

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//

#ifdef __cplusplus
}
#endif

#endif /* HARPI_H */
