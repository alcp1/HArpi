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
#include <limits.h>
#include <time.h>
#include <auxiliary.h>
#include <csvconfig.h>
#include <debug.h>
#include <harpi.h>
#include <harpiactions.h>

//----------------------------------------------------------------------------//
// INTERNAL DEFINITIONS
//----------------------------------------------------------------------------//
#define MAX_FILENAME_LEN 2048
#define MAX_LINE_LEN 4096
#define MAX_STRING_LEN 2048

//----------------------------------------------------------------------------//
// INTERNAL TYPES
//----------------------------------------------------------------------------//
// CSV File Data
typedef struct  
{
    uint16_t last_maxStateMachineID;
    uint16_t last_maxActionSetID;
    uint16_t last_maxEventSetID;
    uint16_t new_maxStateMachineID;
    uint16_t new_maxActionSetID;
    uint16_t new_maxEventSetID;
} csvconfigFileData;

// Field type
typedef enum  
{
    CSV_FIELD_TYPE_INT16 = 0,
    CSV_FIELD_TYPE_LOAD,
    CSV_FIELD_TYPE_UINT8_HEX,
    CSV_FIELD_TYPE_UINT8_DEC,
    CSV_FIELD_TYPE_CHAR,
} csvconfig_field_type_t;

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
static csvconfig_file_section_t processLine(char *line, 
    csvconfigFileData* fileData);
static bool processSMLoads(char *line, csvconfigFileData* fileData, 
    harpiSMLoadsData* data);
static bool processSMEvents(char *line, csvconfigFileData* fileData, 
    harpiSMEventsData* data);
static bool processActionSets(char *line, csvconfigFileData* fileData,
    harpiActionSetsData* data);
static bool processStatesActions(char *line, csvconfigFileData* fileData, 
    harpiStateActionsData* data);
static bool processStatesTransitions(char *line, csvconfigFileData* fileData, 
    harpiStateTransitionsData* data);
static bool processEventSets(char *line, csvconfigFileData* fileData, 
    harpiEventSetsData* data);
static bool processField(char* field, csvconfig_field_type_t fieldtype, 
    int16_t* value);

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

/**
 * Function to get the load type for a given string
 **/
static harpiLoadType_t getLoadType(char *str)
{
    harpiLoadType_t type;
    // Find out the section
    if (strcmp(str, "Relay") == 0)
    {
        type = HARPI_LOAD_TYPE_RELAY;
    }    
    else
    {
        type = HARPI_LOAD_TYPE_OTHER;
    }
    return type;
}

/**
 * Function to process a given line of the config file. 
 * Returns the section processed (CSV_SECTION_OTHER in case of error)
 * and fills fileData for the fields found.
 **/
static csvconfig_file_section_t processLine(char *line, 
    csvconfigFileData* fileData)
{
    char *token;
    harpiLinkedList element;
    csvconfig_file_section_t section = CSV_SECTION_OTHER;
    // Remove newline character if present
    line[strcspn(line, "\n")] = 0;
    // Parse the line.
    token = strtok_r(line, ",", &line);
    if (token != NULL) 
    {
        section = getCSVSection(token);
        switch(section)
        {
            case CSV_SECTION_STATE_MACHINES_AND_LOADS:
                if(!processSMLoads(line, fileData, &element.smLoadsData))
                {
                    section = CSV_SECTION_OTHER;
                }
                break;
            case CSV_SECTION_STATE_MACHINES_AND_EVENTS:
                if(!processSMEvents(line, fileData, &element.smEventsData))
                {
                    section = CSV_SECTION_OTHER;
                }
                break;
            case CSV_SECTION_ACTION_SETS:
                if(!processActionSets(line, fileData, &element.actionSetsData))
                {
                    section = CSV_SECTION_OTHER;
                }
                break;
            case CSV_SECTION_STATES_AND_ACTIONS:
                if(!processStatesActions(line, fileData, 
                    &element.stateActionsData))
                {
                    section = CSV_SECTION_OTHER;
                }
                break;
            case CSV_SECTION_STATE_TRANSITIONS:
                if(!processStatesTransitions(line, fileData, 
                    &element.stateTransitionsData))
                {
                    section = CSV_SECTION_OTHER;
                }
                break;
            case CSV_SECTION_EVENT_SETS:
                if(!processEventSets(line, fileData, &element.eventSetsData))
                {
                    section = CSV_SECTION_OTHER;
                }
                break;
            default:
                break;
        }
        if(section != CSV_SECTION_OTHER)
        {
            element.section = section;
            harpi_addElementToList(&element);
        }
    }
    return section;
}

/**
 * Process lines for "State Machines and Loads"
 **/
static bool processSMLoads(char *line, csvconfigFileData* fileData, 
    harpiSMLoadsData* data)
{
    char *token;
    char *rest_of_line; // Context pointer for strtok_r
    bool ret = false;
    int16_t val;
    // initial position - line was processed with strtok_r a first time
    rest_of_line = line;
    //-------------------------
    // Get fields
    //-------------------------
    // int16_t stateMachineID
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_INT16, &val);
    if(!ret)
    {
        return ret;
    }
    data->stateMachineID = val + fileData->last_maxStateMachineID;
    if(data->stateMachineID > fileData->new_maxStateMachineID)
    {
        fileData->new_maxStateMachineID = data->stateMachineID;
    }
    // harpiLoadType_t type
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_LOAD, &val);
    if(!ret)
    {
        return ret;
    }
    data->type = (harpiLoadType_t)(val);
    // uint8_t node
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_UINT8_HEX, &val);
    if(!ret)
    {
        return ret;
    }
    data->node = (uint8_t)(val);
    // uint8_t group
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_UINT8_HEX, &val);
    if(!ret)
    {
        return ret;
    }
    data->group = (uint8_t)(val);
    // uint8_t channel
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_UINT8_DEC, &val);
    if(!ret)
    {
        return ret;
    }
    data->channel = (uint8_t)(val);
    return ret;
}

/**
 * Process lines for "State Machines and Events"
 **/
static bool processSMEvents(char *line, csvconfigFileData* fileData, 
    harpiSMEventsData* data)
{
    char *token;
    char *rest_of_line; // Context pointer for strtok_r
    bool ret = false;
    int16_t val;
    // initial position - line was processed with strtok_r a first time
    rest_of_line = line;
    //-------------------------
    // Get fields
    //-------------------------
    // int16_t stateMachineID
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_INT16, &val);
    if(!ret)
    {
        return ret;
    }
    data->stateMachineID = val + fileData->last_maxStateMachineID;
    if(data->stateMachineID > fileData->new_maxStateMachineID)
    {
        fileData->new_maxStateMachineID = data->stateMachineID;
    }
    // int16_t eventSetID
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_INT16, &val);
    if(!ret)
    {
        return ret;
    }
    data->eventSetID = val + fileData->last_maxEventSetID;
    if(data->eventSetID > fileData->new_maxEventSetID)
    {
        fileData->new_maxEventSetID = data->eventSetID;
    }
    return ret;
}

/**
 * Process lines for "Action Sets"
 **/
static bool processActionSets(char *line, csvconfigFileData* fileData,
    harpiActionSetsData* data)
{
    char *token;
    char *rest_of_line; // Context pointer for strtok_r
    bool ret = false;
    int16_t val;
    int16_t i;
    uint8_t bytesFrame[HAPCAN_FULL_FRAME_LEN];    
    // initial position - line was processed with strtok_r a first time
    rest_of_line = line;
    //-------------------------
    // Get fields
    //-------------------------
    // int16_t actionsSetID
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_INT16, &val);
    if(!ret)
    {
        return ret;
    }
    data->actionsSetID = val + fileData->last_maxActionSetID;
    if(data->actionsSetID > fileData->new_maxActionSetID)
    {
        fileData->new_maxActionSetID = data->actionsSetID;
    }
    // hapcanCANData frame
    for(i = 0; i < HAPCAN_FULL_FRAME_LEN; i++)
    {
        token = strtok_r(rest_of_line, ",", &rest_of_line);
        ret = processField(token, CSV_FIELD_TYPE_UINT8_HEX, &val);
        if(!ret)
        {
            return ret;
        }
        bytesFrame[i] = (uint8_t)(val);
    }
    // convert byte array to hapcan frame
    aux_clearHAPCANFrame(&(data->frame));
    aux_getHAPCANFromBytes(bytesFrame, &(data->frame));
    return ret;
}

/**
 * Process lines for "States and Actions"
 **/
static bool processStatesActions(char *line, csvconfigFileData* fileData, 
    harpiStateActionsData* data)
{
    char *token;
    char *rest_of_line; // Context pointer for strtok_r
    bool ret = false;
    int16_t val;
    // initial position - line was processed with strtok_r a first time
    rest_of_line = line;
    //-------------------------
    // Get fields
    //-------------------------
    // int16_t stateMachineID
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_INT16, &val);
    if(!ret)
    {
        return ret;
    }
    data->stateMachineID = val + fileData->last_maxStateMachineID;
    if(data->stateMachineID > fileData->new_maxStateMachineID)
    {
        fileData->new_maxStateMachineID = data->stateMachineID;
    }
    // int16_t currentStateID
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_INT16, &val);
    if(!ret)
    {
        return ret;
    }
    data->currentStateID = val;
    // int16_t eventSetID
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_INT16, &val);
    if(!ret)
    {
        return ret;
    }
    data->eventSetID = val + fileData->last_maxEventSetID;
    if(data->eventSetID > fileData->new_maxEventSetID)
    {
        fileData->new_maxEventSetID = data->eventSetID;
    }
    // int16_t actionsSetID
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_INT16, &val);
    if(!ret)
    {
        return ret;
    }
    data->actionsSetID = val + fileData->last_maxActionSetID;
    if(data->actionsSetID > fileData->new_maxActionSetID)
    {
        fileData->new_maxActionSetID = data->actionsSetID;
    }
    return ret;
}

/**
 * Process lines for "State Transitions"
 **/
static bool processStatesTransitions(char *line, csvconfigFileData* fileData, 
    harpiStateTransitionsData* data)
{
    char *token;
    char *rest_of_line; // Context pointer for strtok_r
    bool ret = false;
    int16_t val;
    // initial position - line was processed with strtok_r a first time
    rest_of_line = line;
    //-------------------------
    // Get fields
    //-------------------------
    // int16_t stateMachineID
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_INT16, &val);
    if(!ret)
    {
        return ret;
    }
    data->stateMachineID = val + fileData->last_maxStateMachineID;
    if(data->stateMachineID > fileData->new_maxStateMachineID)
    {
        fileData->new_maxStateMachineID = data->stateMachineID;
    }
    // int16_t currentStateID
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_INT16, &val);
    if(!ret)
    {
        return ret;
    }
    data->currentStateID = val;
    // int16_t eventSetID
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_INT16, &val);
    if(!ret)
    {
        return ret;
    }
    data->eventSetID = val + fileData->last_maxEventSetID;
    if(data->eventSetID > fileData->new_maxEventSetID)
    {
        fileData->new_maxEventSetID = data->eventSetID;
    }
    // int16_t newStateID
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_INT16, &val);
    if(!ret)
    {
        return ret;
    }
    data->newStateID = val;
    return ret;
}

/**
 * Process lines for "Event Sets".
 **/
static bool processEventSets(char *line, csvconfigFileData* fileData, 
    harpiEventSetsData* data)
{
    char *token;
    char *rest_of_line; // Context pointer for strtok_r
    bool ret = false;
    int16_t val;
    int16_t i; 
    // initial position - line was processed with strtok_r a first time
    rest_of_line = line;
    //-------------------------
    // Get fields
    //-------------------------
    // int16_t eventSetID
    token = strtok_r(rest_of_line, ",", &rest_of_line);
    ret = processField(token, CSV_FIELD_TYPE_INT16, &val);
    if(!ret)
    {
        return ret;
    }
    data->eventSetID = val + fileData->last_maxEventSetID;
    if(data->eventSetID > fileData->new_maxEventSetID)
    {
        fileData->new_maxEventSetID = data->eventSetID;
    }
    // uint8_t fiterCondition[HAPCAN_FULL_FRAME_LEN]
    for(i = 0; i < HAPCAN_FULL_FRAME_LEN; i++)
    {
        token = strtok_r(rest_of_line, ",", &rest_of_line);
        ret = processField(token, CSV_FIELD_TYPE_CHAR, &val);
        if(!ret)
        {
            return ret;
        }
        data->fiterCondition[i] = (uint8_t)(val);
    }
    // uint8_t fiter[HAPCAN_FULL_FRAME_LEN]
    for(i = 0; i < HAPCAN_FULL_FRAME_LEN; i++)
    {
        token = strtok_r(rest_of_line, ",", &rest_of_line);
        ret = processField(token, CSV_FIELD_TYPE_UINT8_HEX, &val);
        if(!ret)
        {
            return ret;
        }
        data->fiter[i] = (uint8_t)(val);
    }
    return ret;
}

static bool processField(char* field, csvconfig_field_type_t fieldtype, 
    int16_t* value)
{
    bool ret;
    char *endptr;
    ret = false;
    long val;
    uint8_t byte;
    int test;
    switch(fieldtype)
    {
        case CSV_FIELD_TYPE_INT16:
            // Convert string to unsigned long - Base 10
            val = strtol(field, &endptr, 10);
            // Check for conversion errors
            if( (endptr == field) || (*endptr != '\0') || (val > USHRT_MAX) || 
                (val < 0) )
            {
                ret = false;
            }
            else
            {
                *value = (int16_t)val;
                ret = true;
            }
            break;
        case CSV_FIELD_TYPE_LOAD:
            val = (long)(getLoadType(field));
            if(val == HARPI_LOAD_TYPE_OTHER)
            {
                ret = false;
            }
            else
            {
                *value = (int16_t)val;
                ret = true;
            }
            break;
        case CSV_FIELD_TYPE_UINT8_HEX:
            // Use %hhx to read a hexadecimal value into a uint8_t
            // The '2' in %2hhx ensures that exactly two characters are read
            test = sscanf(field, "%2hhx", &byte);
            if( (test != 1) || (byte > UCHAR_MAX) || (byte < 0) )
            {
                ret = false;
            }
            else
            {
                *value = (int16_t)byte;
                ret = true;
            }

            break;
        case CSV_FIELD_TYPE_UINT8_DEC:
            // Convert string to unsigned long - Base 10
            val = strtol(field, &endptr, 10);
            // Check for conversion errors
            if( (endptr == field) || (*endptr != '\0') || (val > UCHAR_MAX) || 
                (val < 0) )
            {
                ret = false;
            }
            else
            {
                *value = (int16_t)val;
                ret = true;
            }
            break;
        case CSV_FIELD_TYPE_CHAR:
            if(strlen(field) != 1)
            {
                ret = false;
            }
            else
            {
                *value = (int16_t)(field[0]);
                ret = true;
            }
            break;
        default:    
            break;
    }
    return ret;
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
    csvconfigFileData fileData;
    //----------------------------------
    // Init Gateways
    //----------------------------------
    harpi_initList(true);
    //----------------------------------
    // Init CSV File(s) data
    //----------------------------------
    a_csvconfigFileData = (csvconfigFileData*)malloc(g_n_csv_files * 
        sizeof(csvconfigFileData));
    for (i = 0; i < g_n_csv_files; i++)
    {
        a_csvconfigFileData[i].last_maxStateMachineID = 0;
        a_csvconfigFileData[i].last_maxActionSetID = 0;
        a_csvconfigFileData[i].last_maxEventSetID = 0;
        a_csvconfigFileData[i].new_maxStateMachineID = 0;
        a_csvconfigFileData[i].new_maxActionSetID = 0;
        a_csvconfigFileData[i].new_maxEventSetID = 0;
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
        // Get the offsets
        fileData = a_csvconfigFileData[i];
        while(fgets(line, sizeof(line), file) != NULL)
        {
            // Process each line
            if (processLine(line, &fileData) == CSV_SECTION_OTHER)
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
        // Set the offsets antil the last file is read
        if(i < (g_n_csv_files - 1))
        {
            // Fill offsets - last max StateMachineID
            a_csvconfigFileData[i + 1].last_maxStateMachineID = 
                a_csvconfigFileData[i].last_maxStateMachineID + 1;
            // Fill offsets - last max ActionSetID
            a_csvconfigFileData[i + 1].last_maxActionSetID = 
                a_csvconfigFileData[i].last_maxActionSetID + 1;
            // Fill offsets - last max EventSetID
            a_csvconfigFileData[i + 1].last_maxEventSetID = 
                a_csvconfigFileData[i].last_maxEventSetID + 1;
            // Fill new maximum values
            a_csvconfigFileData[i + 1].new_maxStateMachineID = 0;         
            a_csvconfigFileData[i + 1].new_maxActionSetID = 0;
            a_csvconfigFileData[i + 1].new_maxEventSetID = 0;
        }
        // close file
        fclose(file);
    }
    if(!isOK)
    {
        // Init all modules and clear linked list
        harpi_initList(true);
    }
    else
    {
        // Load all modules and clear linked list
        harpi_load();
    }
    // Free used data
    free(a_csvconfigFileData);
    a_csvconfigFileData = NULL;
}
