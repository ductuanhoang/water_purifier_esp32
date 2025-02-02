/**
 * @file json_bluetooth.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-03-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef JSON_BLUETOOTH_
#define JSON_BLUETOOTH_

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <stdbool.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/


/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/
/**
 * @brief 
 * 
 * @param token 
 * @param message_packet 
 */
void json_bluetooth_packet(char *message_packet, const char *token, bool state, const char *clientid);

#endif /* JSON_BLUETOOTH_ */