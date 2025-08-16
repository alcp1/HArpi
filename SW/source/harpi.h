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
#include <csvconfig.h>
    
//----------------------------------------------------------------------------//
// EXTERNAL DEFINITIONS
//----------------------------------------------------------------------------//
    
//----------------------------------------------------------------------------//
// EXTERNAL TYPES
//----------------------------------------------------------------------------//
// Load Type
typedef enum
{
  HARPI_LOAD_TYPE_RELAY = 0, // "Relay"
  HARPI_LOAD_TYPE_OTHER
}harpiLoadType_t;

// Event Type
typedef enum
{
  HARPI_EVENT_CAN = 0,
  HARPI_EVENT_OTHER
}harpiEventType_t;

// Event (for event processing)
typedef struct  
{
    harpiEventType_t type;
    int16_t eventSetID;
} harpiEvent_t;

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
    int16_t eventSetID;
    uint8_t fiterCondition[HAPCAN_FULL_FRAME_LEN];
    uint8_t fiter[HAPCAN_FULL_FRAME_LEN];
} harpiEventSetsData;

// States and Actions
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

// Linked List Data
typedef struct harpiLinkedList
{
    csvconfig_file_section_t section;
    harpiSMLoadsData smLoadsData;
    harpiSMEventsData smEventsData;
    harpiActionSetsData actionSetsData;
    harpiEventSetsData eventSetsData;
    harpiStateActionsData stateActionsData;
    harpiStateTransitionsData stateTransitionsData;
    struct harpiLinkedList* next;
} harpiLinkedList;

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
/**
 * Init buffers
 * 
 * \return  EXIT_SUCCESS
 *          EXIT_FAILURE (Close and Reinit Buffers)
 **/
int harpi_initBuffers(void);

/**
 * Init Linked Lists data:
 * - empty the list and if list is available, free used memory
 * 
 * * \param   init_modules  (INPUT) flag to init submodules (true to init all
 *                              submodules)
 * 
 **/
void harpi_initList(bool init_modules);

/**
 * Add an element to the linked list from CSV file
 * 
 * \param   element   (INPUT) element to be added to the linked list
 * 
 **/
void harpi_addElementToList(harpiLinkedList* element);

/**
 * Get the number of elements in the linked list for a given section
 * 
 * \param   section   (INPUT) section to be matched with linked list elements
 * 
 **/
int16_t harpi_getLinkedListNElements(csvconfig_file_section_t section);

/**
 * Init list and memory after all elements are added with the 
 * harpi_AddElementToList function.
 * 
 **/
void harpi_load(void);


#ifdef __cplusplus
}
#endif

#endif /* HARPI_H */
