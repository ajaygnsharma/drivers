/******************************************************************************
 * Includes
 *****************************************************************************/
#include "pca9532.h"
#include "i2c.h"

/******************************************************************************
 * Implementation of public functions
 *****************************************************************************/

tS8
pca9532(tU8* pBuf, tU16 len, tU8* pBuf2, tU16 len2) 
{
  tS8  retCode = 0;
  tU8  status  = 0;
  tU16 i       = 0;

  do
  {
    /* generate Start condition */
    retCode = i2cStart();
    if(retCode != I2C_CODE_OK)
      break;


    /* write pca9532 address */
    retCode = i2cWriteWithWait(0xc0);
    if(retCode != I2C_CODE_OK)
      break;

    /* write data */
    for(i = 0; i <len; i++)
    {
      retCode = i2cWriteWithWait(*pBuf);
      if(retCode != I2C_CODE_OK)
        break;

      pBuf++;
    }

  } while(0);

  if (len2 > 0)
  {
    /* Generate Start condition */
    retCode = i2cRepeatStart();

    /* Transmit device address */
    if( retCode == I2C_CODE_OK)
    {
      /* Write SLA+R */
      retCode = i2cPutChar( 0xc0 + 0x01 );
      while( retCode == I2C_CODE_BUSY )
      {
        retCode = i2cPutChar( 0xc0 + 0x01 );
      }
    }

    /* Wait until SLA+R transmitted */
    while(1)
    {
      /* Get new status */
      status = i2cCheckStatus();

      if(status == 0x40)
      {
        /* Data transmitted and ACK received */
        break;
      }
      else if(status != 0xf8)
      {
        /* error */
        retCode = I2C_CODE_ERROR;
        break;
      }
    }

    if( retCode == I2C_CODE_OK )
    {
      /* wait until address transmitted and receive data */
      for(i = 1; i <= len2; i++ )
      {
        /* wait until data transmitted */
        while(1)
        {
          /* Get new status */
          status = i2cCheckStatus();

          if(( status == 0x40 ) || ( status == 0x48 ) || ( status == 0x50 ))
          {
            /* Data received */

            if(i == len2 )
            {
              /* Set generate NACK */
              retCode = i2cGetChar( I2C_MODE_ACK1, pBuf2 );
            }
            else
            {
              retCode = i2cGetChar( I2C_MODE_ACK0, pBuf2 );
            }

            /* Read data */
            retCode = i2cGetChar( I2C_MODE_READ, pBuf2 );
            while( retCode == I2C_CODE_EMPTY )
            {
              retCode = i2cGetChar( I2C_MODE_READ, pBuf2 );
            }
            pBuf2++;

            break;
          }
          else if( status != 0xf8 )
          {
            /* ERROR */
            i = len2;
            retCode = I2C_CODE_ERROR;
            break;
          }
        }
      }
    }
  }

  /* Generate Stop condition */
  i2cStop();

  return retCode;
}
