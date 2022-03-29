/**
 * @file WaterPurifier_PacketMessage.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-03-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef WATERPURIFIER_PACKET_MESSAGE_H_
#define WATERPURIFIER_PACKET_MESSAGE_H_

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/


/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/
void water_purifier_packet_data_bool(unsigned char dpid, unsigned short value);
void water_purifier_packet_data_enum(unsigned char dpid, unsigned short value);
void water_purifier_packet_data_value(unsigned char dpid, const unsigned char value[]);
#endif /* WATERPURIFIER_PACKET_MESSAGE_H_ */
