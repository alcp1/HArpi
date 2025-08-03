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
// CSV File Data
typedef struct  
{
    uint16_t n_LinesStateLoads;
    uint16_t n_LinesStateEvents;
    uint16_t n_LinesActionSets;
    uint16_t n_LinesEventSets;
    uint16_t n_LinesStateActions;
    uint16_t n_LinesStateTransitions;
    uint16_t maxStateMachineID;
    uint16_t maxActionSetID;
    uint16_t maxEventSetID;
} csvconfigFileData;
    
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

