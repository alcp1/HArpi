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
// State Machines and Loads 
typedef struct  
{
    char* stateMachineName;
    uint16_t stateMachineID;
    char* bus;
    char* load;
    char* type;
    uint8_t node;
    uint8_t group;
    uint8_t channel;
} harpiSMLoadsData;

// State Machines and Events
typedef struct  
{
    char* stateMachineName;
    char* eventName;
    uint16_t stateMachineID;
    uint16_t eventSetID;
} harpiSMEventsData;

// Action Sets
typedef struct  
{
    uint16_t actionsSetID;
    char* bus;
    char* load;
    char* eventName;
    hapcanCANData frame;
} harpiActionSetsData;

// Event Sets
typedef struct  
{
    uint16_t eventsSetID;
    char* bus;
    char* load;
    char* eventName;
    uint8_t fiterCondition[12];
    uint8_t fiter[12];
} harpiEventSetsData;

//States and Actions
typedef struct  
{
    char* stateMachineName;
    char* eventName;
    uint16_t stateMachineID;
    uint16_t currentStateID;
    uint16_t eventSetID;
    uint16_t actionsSetID;
} harpiStateActionsData;

// State Transitions
typedef struct  
{
    char* stateMachineName;
    char* currentStateName;
    char* eventName;
    char* newStateName;
    uint16_t stateMachineID;
    uint16_t currentStateID;
    uint16_t eventSetID;
    uint16_t newStateID;
} harpiStateTransitionsData;

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//

#ifdef __cplusplus
}
#endif

#endif /* HARPI_H */
