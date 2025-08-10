//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 30/Jul/2025 |                               | ALCP             //
// - First Version                                                            //
//----------------------------------------------------------------------------//


#ifndef CSVCONFIG_H
#define CSVCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif
    
/*
* Includes
*/
    
//----------------------------------------------------------------------------//
// EXTERNAL DEFINITIONS
//----------------------------------------------------------------------------//
#define CSV_CONFIG_FILES_PATH  "."

//----------------------------------------------------------------------------//
// EXTERNAL TYPES
//----------------------------------------------------------------------------//
// Section
typedef enum  
{
    CSV_SECTION_STATE_MACHINES_AND_LOADS = 0,
    CSV_SECTION_STATE_MACHINES_AND_EVENTS,
    CSV_SECTION_ACTION_SETS,
    CSV_SECTION_STATES_AND_ACTIONS,
    CSV_SECTION_STATE_TRANSITIONS,
    CSV_SECTION_EVENT_SETS,
    CSV_SECTION_OTHER,
} csvconfig_file_section_t;
    
//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
/**
 * Initial setup (only called during startup)
 **/
void csvconfig_init(void);

/**
 * Inform if a new configuraton file is available
 * 
 * \return  true    new file available
 *          false   no new file available
 **/
bool csvconfig_isNewConfigAvailable(void);

/**
 * Fill the entire configuration file with data from the available files
 * 
 **/
void csvconfig_reload(void);

#ifdef __cplusplus
}
#endif

#endif /* CSVCONFIG_H */

