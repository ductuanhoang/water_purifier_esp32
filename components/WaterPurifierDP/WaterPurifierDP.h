/**
 * @file WaterPurifierDP.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-03-26
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef WATERPURIFIERDP_H_
#define WATERPURIFIERDP_H_
#ifdef __cplusplus
extern "C"
{
#endif
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* Data point define */
#define DPID_TDS_OUT 1
#define DPID_PAC_FILTERTIM 4
#define DPID_RO_FILTERTIME 5
#define DPID_CF_FILTERTIME 6
#define DPID_CURRENT_TEMP 8
#define DPID_TOTAL_WATER 9
#define DPID_RESET_WATER 10
#define DPID_SWITCH_FLOW 12
#define DPID_CHILD_LOCK 15
#define DPID_STATUS 16
#define DPID_RO_RESET 17
#define DPID_PAC_RESET 18
#define DPID_CF_RESET 20
#define DPID_SWITCH 21
#define DPID_TEMP 22
#define DPID_FAULT 23
#define DPID_SET_FLOW 25
#define DPID_PAC_DAYLEFT 27
#define DPID_RO_DAYLEFT 28
#define DPID_CF_DAYLEFT 29
#define DPID_AUTO_CLEAN 30
    /****************************************************************************/
    /***        Type Definitions                                              ***/
    /****************************************************************************/

    /****************************************************************************/
    /***         Exported global functions                                     ***/
    /****************************************************************************/
    extern "C"
    {
        void WaterPurifier_init(void);
        void WaterPurifier_Process(void);
    }
#ifdef __cplusplus
}
#endif
#endif /* WATERPURIFIERDP_H_ */
