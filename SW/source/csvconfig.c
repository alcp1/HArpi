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
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>
#include <time.h>
#include <auxiliary.h>
#include <csvconfig.h>
#include <debug.h>
#include <harpi.h>

//----------------------------------------------------------------------------//
// INTERNAL DEFINITIONS
//----------------------------------------------------------------------------//
#define MAX_FILENAME_LEN 2048
#define MAX_LINE_LEN 4096
#define MAX_STRING_LEN 2048

//----------------------------------------------------------------------------//
// INTERNAL TYPES
//----------------------------------------------------------------------------//
// Error Type
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
// INTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//
static uint16_t g_n_csv_files;
static time_t* g_last_date_array;
static char** g_csv_filepath_array;

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
static bool areConfigFilesChanged(void);
static bool isCSVfile(const char *filename);
static csvconfig_file_section_t getCSVSection(char *str);


/**
 * Checks if there is a change on any of the csv files. If so, fills 
 * g_n_csv_files, g_last_date_array and g_csv_filepath_array
 * 
 * returns true if a change was detected
 **/
static bool areConfigFilesChanged(void)
{    
    DIR *d;
    struct dirent *dir;
    int n_csv_files = 0;
    char filepath[MAX_FILENAME_LEN];
    bool ret = false;
    int i;
    const char *filename;
    struct stat file_details;
    time_t filedate;
    //--------------------
    // Check number of csv files
    //--------------------
    d = opendir(CSV_CONFIG_FILES_PATH);
    if(d == NULL) 
    {
        #ifdef DEBUG_CVSCONFIG_ERRORS
        debug_print("cvsconfig - ERROR: Error opening directory!\n");
        #endif
    }
    else
    {
        // Read directory files
        while((dir = readdir(d)) != NULL) 
        {
            if(isCSVfile(dir->d_name))
            {
                n_csv_files++;
            }
        }
    }
    //--------------------
    // If number of files do not match, clear global variables
    //--------------------
    if(n_csv_files != g_n_csv_files)
    {
        #ifdef DEBUG_CVSCONFIG_EVENTS
        debug_print("cvsconfig - New number of CSV files in directory!\n");
        #endif
        // Files are changed
        ret = true;
        // Update g_last_date_array
        free(g_last_date_array);
        g_last_date_array = NULL;
        g_last_date_array = (time_t*)malloc(g_n_csv_files * sizeof(time_t));
        // Reset g_csv_filepath_array
        for (i = 0; i < g_n_csv_files; i++)
        {
            // Free the string and set it to NULL
            free(g_csv_filepath_array[i]);
            g_csv_filepath_array[i] = NULL;
            // Update file(s) date(s)
            g_last_date_array[i] = 0;
        }
        // Update g_n_csv_files after cleaning
        g_n_csv_files = n_csv_files;
        // Then, free the array of pointers itself and set it to NULL
        free(g_csv_filepath_array);
        g_csv_filepath_array = NULL;        
        // Allocate memory for the array of pointers
        g_csv_filepath_array = (char**)malloc(g_n_csv_files * sizeof(char*));
        for (i = 0; i < g_n_csv_files; i++)
        {
            // Set string to NULL
            g_csv_filepath_array[i] = NULL;
        }   
    }
    //--------------------
    // Check each file
    //--------------------
    i = 0;
    d = opendir(CSV_CONFIG_FILES_PATH);
    if(d == NULL) 
    {
        #ifdef DEBUG_CVSCONFIG_ERRORS
        debug_print("cvsconfig - ERROR: Error opening directory!\n");
        #endif
    }
    else
    {
        // Read directory files
        while((dir = readdir(d)) != NULL) 
        {
            filename = dir->d_name;
            if(isCSVfile(dir->d_name))
            {
                // Fill filepath
                snprintf(filepath, sizeof(filepath), "%s/%s", 
                    CSV_CONFIG_FILES_PATH, filename);
                //---------------------
                // Check file names
                //---------------------                
                if(g_csv_filepath_array[i] == NULL)
                {
                    #ifdef DEBUG_CVSCONFIG_EVENTS
                    debug_print("cvsconfig - New CSV file in directory: %s!\n", 
                        filename);
                    #endif
                    // New file - copy to the strings array
                    ret = true;
                    free(g_csv_filepath_array[i]);
                    g_csv_filepath_array[i] = NULL;
                    g_csv_filepath_array[i] = (char*)malloc(strlen(filepath)+1);
                    strncpy(g_csv_filepath_array[i], filepath, 
                        strlen(filepath)+1);
                }
                else
                {
                    if(strncmp(filepath, g_csv_filepath_array[i], 
                        sizeof(filepath)) != 0)
                    {
                        #ifdef DEBUG_CVSCONFIG_EVENTS
                        debug_print("cvsconfig - Different CSV file name in "
                            "directory: %s!\n", filename);
                        #endif
                        // New file / name - copy to the strings array
                        ret = true;
                        free(g_csv_filepath_array[i]);
                        g_csv_filepath_array[i] = NULL;
                        g_csv_filepath_array[i] = (char*)malloc(
                            strlen(filepath)+1);
                        strncpy(g_csv_filepath_array[i], filepath, 
                            strlen(filepath)+1);
                    }
                }
                //---------------------
                // Check file(s) date(s)
                //---------------------
                if( stat(filepath, &file_details) != -1)
                {
                    filedate = file_details.st_mtime;
                    if(filedate != g_last_date_array[i])
                    {
                        #ifdef DEBUG_CVSCONFIG_EVENTS
                        debug_print("cvsconfig - Different date for file in "
                            "directory: %s!\n", filename);
                        #endif
                        // New file date
                        ret = true;
                        g_last_date_array[i] = filedate;
                        free(g_csv_filepath_array[i]);
                        g_csv_filepath_array[i] = NULL;
                        g_csv_filepath_array[i] = (char*)malloc(
                            strlen(filepath)+1);
                        strncpy(g_csv_filepath_array[i], filepath, 
                            strlen(filepath)+1);
                    }
                }
                else
                {
                    #ifdef DEBUG_CVSCONFIG_ERRORS
                    debug_print("cvsconfig - Error: getting date for file in "
                        "directory: %s!\n", filepath);
                    #endif
                    // Stat error: consider new file
                    ret = true;
                    free(g_csv_filepath_array[i]);
                    g_csv_filepath_array[i] = NULL;
                    g_csv_filepath_array[i] = (char*)malloc(
                        strlen(filepath)+1);
                    strncpy(g_csv_filepath_array[i], filepath, 
                        strlen(filepath)+1);
                }
                // increment index to next string
                i++;
            }
        }
    }
    return ret;
}

/**
 * Function to check if a file has a .csv extension
 **/
static bool isCSVfile(const char *filename)
{
    bool ret;
    const char *dot = strrchr(filename, '.');
    ret = dot != NULL;
    ret = ret && (strcmp(dot, ".csv") == 0);
    return ret;
}

/**
 * Function to get the section for a given string
 **/
static csvconfig_file_section_t getCSVSection(char *str)
{
    csvconfig_file_section_t section;
    // Find out the section
    if (strcmp(str, "State Machines and Loads") == 0)
    {
        section = CSV_SECTION_STATE_MACHINES_AND_LOADS;
    }
    else if (strcmp(str, "State Machines and Events") == 0)
    {
        section = CSV_SECTION_STATE_MACHINES_AND_EVENTS;
    }
    else if (strcmp(str, "Action Sets") == 0)
    {
        section = CSV_SECTION_ACTION_SETS;
    }
    else if (strcmp(str, "States and Actions") == 0)
    {
        section = CSV_SECTION_STATES_AND_ACTIONS;
    }
    else if (strcmp(str, "State Transitions") == 0)
    {
        section = CSV_SECTION_STATE_TRANSITIONS;
    }
    else if (strcmp(str, "Event Sets") == 0)
    {
        section = CSV_SECTION_EVENT_SETS;
    }
    else
    {
        section = CSV_SECTION_OTHER;
    }
    return section;
}

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
/**
 * Initial setup (only called during startup)
 **/
void csvconfig_init(void)
{
    bool update;
    g_n_csv_files = 0;
    g_last_date_array = NULL;
    g_csv_filepath_array = NULL;
    update = areConfigFilesChanged();
    if(update)
    {
        csvconfig_reload();
    }
}

/**
 * Inform if a new configuraton file is available
 * 
 * \return  true    new file available
 *          false   no new file available
 **/
bool csvconfig_isNewConfigAvailable(void)
{
    bool ret;
    ret = areConfigFilesChanged();
    return ret;
}

/**
 * Fill the entire configuration file with data from the available files
 * 
 **/
void csvconfig_reload(void)
{
    int i;
    FILE *file;
    bool isOK;
    char line[MAX_LINE_LEN];
    csvconfigFileData* a_csvconfigFileData;
    char *token;
    csvconfig_file_section_t section = CSV_SECTION_OTHER;
    //----------------------------------
    // Init CSV File(s) data
    //----------------------------------
    a_csvconfigFileData = (csvconfigFileData*)malloc(g_n_csv_files * 
        sizeof(csvconfigFileData));
    for (i = 0; i < g_n_csv_files; i++)
    {
        a_csvconfigFileData[i].n_LinesActionSets = 0;
        a_csvconfigFileData[i].n_LinesStateEvents = 0;
        a_csvconfigFileData[i].n_LinesActionSets = 0;
        a_csvconfigFileData[i].n_LinesEventSets = 0;
        a_csvconfigFileData[i].n_LinesStateActions = 0;
        a_csvconfigFileData[i].n_LinesStateTransitions = 0;
        a_csvconfigFileData[i].maxStateMachineID = 0;
        a_csvconfigFileData[i].maxActionSetID = 0;
        a_csvconfigFileData[i].maxEventSetID = 0;
    }    
    //----------------------------------
    // Get the CSV File Data for each file
    //----------------------------------
    isOK = true;
    for (i = 0; i < g_n_csv_files; i++)
    {
        file = fopen(g_csv_filepath_array[i], "r");
        if (file == NULL)
        {
            #ifdef DEBUG_CVSCONFIG_ERRORS
            debug_print("cvsconfig - ERROR: opening file %s!\n", 
                g_csv_filepath_array[i]);
            #endif
            // Leave processing - unexpected error
            isOK = false;
            break;
        }
        // Read header line (if any)
        if (fgets(line, sizeof(line), file) == NULL) 
        {
            fclose(file);
            // Empty file or error reading header
            #ifdef DEBUG_CVSCONFIG_ERRORS
            debug_print("cvsconfig - ERROR: reading file %s!\n", 
                g_csv_filepath_array[i]);
            #endif
            // Leave processing - unexpected error
            isOK = false;
            break;
        }
        while(fgets(line, sizeof(line), file) != NULL)
        {
            // Remove newline character if present
            line[strcspn(line, "\n")] = 0;
            // Parse the line.
            token = strtok(line, ",");
            if (token != NULL) 
            {
                section = getCSVSection(token);
                if(section == CSV_SECTION_OTHER)
                {
                    // Leave processing - unexpected error
                    isOK = false;
                    break;
                }
            } 
            else 
            {
                // Empty file or error reading header
                #ifdef DEBUG_CVSCONFIG_ERRORS
                debug_print("cvsconfig - ERROR: Malformed line in %s: %s\n",
                    g_csv_filepath_array[i], line);
                #endif
                // Leave processing - unexpected error
                isOK = false;
                break;
            }
        }
    }
    if(!isOK)
    {
        free(a_csvconfigFileData);
        a_csvconfigFileData = NULL;
        return;
    }
}
