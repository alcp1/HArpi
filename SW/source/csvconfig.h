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
#define CSV_CONFIG_FILES_PATH  "./"

//----------------------------------------------------------------------------//
// EXTERNAL TYPES
//----------------------------------------------------------------------------//
    
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

