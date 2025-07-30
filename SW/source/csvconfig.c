//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 30/Jul/2025 |                               | ALCP             //
// - First version                                                            //
//----------------------------------------------------------------------------//

/*
* Includes
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <dirent.h>
#include <stdint.h>
#include <time.h>
#include <auxiliary.h>
#include <csvconfig.h>
#include <debug.h>

//----------------------------------------------------------------------------//
// INTERNAL DEFINITIONS
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL TYPES
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//
static pthread_mutex_t g_csvfiles_mutex = PTHREAD_MUTEX_INITIALIZER;
static time_t g_last_date;

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
static bool getConfigFileModifiedDate(time_t *date);
static bool isFileChanged(void);
static void updateConfigFromFile(void);

/**
 * Check if configuration file was changed
 * \param   date    date to be filled
 * \return   If check was OK
 */
static bool getConfigFileModifiedDate(time_t *date) 
{
    /*
    bool ret;
    struct stat file_details;
    if( stat(CSV_CONFIG_FILES_PATH, &file_details) != -1)
    {
        ret = true;
        *date = file_details.st_mtime;
    }
    else
    {
        ret = false;
    }
    return ret;
    */
    return true;
}

/**
 * Check if configuration file was changed
 * \return   TRUE or FALSE
 */
static bool isFileChanged(void) 
{
    time_t date;
    bool ret;
    
    if( getConfigFileModifiedDate(&date) )
    {
        ret = (difftime(date, g_last_date) != 0);
    }
    else
    {
        ret = false;
    }
    return ret;
}

/**
 * Update the local data structure with the data from the file.
 * In case of errors, default values are used.
 **/
static void updateConfigFromFile(void)
{    
    DIR *d;
    struct dirent *dir;
    d = opendir(CSV_CONFIG_FILES_PATH);
    if (d == NULL) 
    {
        perror("Error opening directory");
        return 1;
    }
    // Update file date
    if( !getConfigFileModifiedDate(&g_last_date) )
    {
        g_last_date = 0;
    }
}

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
/**
 * Initial setup (only called during startup)
 **/
void csvconfig_init(void)
{
    updateConfigFromFile();
}

/**
 * Inform if a new configuraton file is available
 * 
 * \return  true    new file available
 *          false   no new file available
 **/
bool csvconfig_isNewConfigAvailable(void)
{

}

/**
 * Fill the entire configuration file with data from the available files
 * 
 **/
void csvconfig_reload(void)
{

}
