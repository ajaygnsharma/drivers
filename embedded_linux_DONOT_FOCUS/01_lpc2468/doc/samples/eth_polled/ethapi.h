/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 *****************************************************************************/

#ifndef _ETHAPI_H_
#define _ETHAPI_H_


/*****************************************************************************
 *
 * Description:
 *    Initialize the ethernet driver
 *
 * Params:
 *    [in] pEthAddr - the Ethernet (MAC) address. Must be 6 bytes long.
 *
 ****************************************************************************/
void ethIf_init(tU8* pEthAddr);

/*****************************************************************************
 *
 * Description:
 *    Send an ethernet packet. 
 *
 * Params:
 *    [in] pData - the data to send
 *    [in] len   - length of the data to send
 *
 ****************************************************************************/
void ethIf_send(tU8* pData, 
                tU16 len);

/*****************************************************************************
 *
 * Description:
 *    Poll the driver for incoming packets. 
 *
 * Params:
 *    [in/out] pBuf - allocated buffer to hold the incoming packet
 *    [in]     len  - length of the buffer
 *
 * Returns:
 *    Length of the received packet.
 *
 ****************************************************************************/
tU16 ethIf_poll(tU8* pBuf, 
                tU16 len);


#endif
