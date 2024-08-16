
/******************************************************************************
 * Includes
 *****************************************************************************/
#include <linux/kernel.h>
/* #include <asm/arch/hardware.h> */
//#include "lpc2468_registers.h"
//#include "irqVec.h"
#include "mci.h"
#include "dma.h"
#include "lpc24xx.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

#if 0
  #define DPRINT(args...) printk(args)
#else
  #define DPRINT(args...)
#endif


int hw_size;        /* in kbytes */
int hw_nr_sects;
int hw_sect_size;   /* in bytes */

extern volatile unsigned long MCI_CardType;

/* Below buffer allocation could be simplier, if the buffer used for DMA and non-DMA 
are shared. Future clear up will be needed. For now, leave as it's. */
#if MCI_DMA_ENABLED
unsigned long *src_addr; 
unsigned long *dest_addr;
#else
volatile unsigned char WriteBlock[BLOCK_LENGTH], ReadBlock[BLOCK_LENGTH];
volatile unsigned long TXBlockCounter, RXBlockCounter;
#endif


/******************************************************************************
 * Public functions
 *****************************************************************************/

int mmc_hw_init(void)
{
  int i      = 0;
  int result = -1;

  DPRINT("mmc_hw_init\n");

  /* on DMA channel 0, source is memory, destination is MCI FIFO. */
  /* On DMA channel 1, source is MCI FIFO, destination is memory. */
  (*(volatile unsigned long *)(PCONP)) |= (1 << 29);   /* Enable GPDMA clock */


  do
  {
    if ( MCI_Init() != TRUE )
    {
      printk("mmc_hw_init:  Failed to initialize MCI\n");
      break;
    }

    if ( (MCI_CardType = MCI_CardInit()) == CARD_UNKNOWN )
    {
      printk("  Error: Memory card could not be found\n");
      break;
    }


    if ( MCI_Check_CID() != TRUE )
    {
      printk("mmc_hw_init: Failed to check CID\n");
      break;
    }

    if ( MCI_Set_Address() != TRUE )
    {
      printk("mmc_hw_init: Failed to set address\n");
      break;
    }

    if ( MCI_Send_CSD() != TRUE )
    {
      printk("mmc_hw_init: Failed to send CSD\n");
      break;
    }

    if ( MCI_Select_Card() != TRUE )
    {
      printk("mmc_hw_init Failed to select card\n");
      break;
    }


    if ( MCI_CardType == SD_CARD )
    {
      DPRINT("mmc_hw_init: Memory card is of SD-type\n");

	    MCI_Set_MCIClock( NORMAL_RATE );
//      MCI_CLOCK |= (1 << 11);       /* Use wide bus for SD */
//      for ( i = 0; i < 0x20; i++ );
//      if ( MCI_Send_ACMD_Bus_Width( BUS_WIDTH_4BITS ) == FALSE )
      if (SD_Set_BusWidth( SD_4_BIT ) != TRUE )
      {
        printk("mmc_hw_init: ERROR from MCI_Send_ACMD_Bus_Width()\n");
        break;
      }
    }

    if ( MCI_Set_BlockLen( BLOCK_LENGTH ) != TRUE )
    {
      printk("mmc_hw_init Failed to set block length\n");
      break;
    }


    result = 0; // Ok
    DPRINT("mmc_hw_init: Initialization OK\n");

  } while (FALSE);


  return result;
}

unsigned char 
mmc_read_csd(unsigned char *pBuffer)
{
  return MCI_Get_CSD(pBuffer);
}

unsigned char 
mmc_read_sector (unsigned long addr,
                 unsigned char *pBuffer)
{
#if MCI_DMA_ENABLED
  unsigned long *p32 = (unsigned long*)pBuffer;
#endif
  unsigned long i = 0;
//  unsigned char* p = 0;
  
  if ( MCI_Read_Block( addr ) != TRUE )
  {
    printk("\nmmc_read_sector: ERROR when calling MCI_Read_Block\n");
    return( -1 );     /* Fatal error */
  }

  /* 
   * When RX_ACTIVE is not set, it indicates RX is done from
   * Interrupt_Read_Block, now, validation of RX and TX buffer can start. 
   */
  while ( MCI_STATUS & MCI_RX_ACTIVE );


  /* Note RXEnable() is called in the Interrupt_Read_Block(). */
  MCI_RXDisable();

#if MCI_DMA_ENABLED
  dest_addr = (unsigned long *)DMA_DST;
  for ( i = 0; i < hw_sect_size; i+=4 )
  {
    *p32++ = *dest_addr;
//    p = (unsigned char*)dest_addr;
//    DPRINT("%x:%x:%x:%x:", p[i], p[i+1], p[i+2], p[i+3]);
    dest_addr++;
  }
#else

printk("mmc_read_sector: reading\n");
  for(i = 0; i < hw_sect_size; i++)
  {
    pBuffer[i] = ReadBlock[i];
  }
#endif

  return 0;

}

unsigned char 
mmc_write_sector (unsigned long addr,
                  unsigned char *pBuffer)
{
#if MCI_DMA_ENABLED
  unsigned long *p32 = (unsigned long*)pBuffer;
#endif
  unsigned long i = 0;

  DPRINT("mmc_write_sector (%x)\n", addr);

#if MCI_DMA_ENABLED
  src_addr = (unsigned long *)DMA_SRC;
  for ( i = 0; i < hw_sect_size; i+=4 )
  {
    *src_addr = *p32++;    
    src_addr++;
  }
#else
  for(i = 0; i < hw_sect_size; i++)
  {
    WriteBlock[i] = pBuffer[i];
  }
#endif


  if ( MCI_Write_Block( addr ) != TRUE )
  {
    printk("\nmmc_write_sector: ERROR when calling MCI_Write_Block\n");
    return( -1 );     /* Fatal error */
  }


  /* 
   * When TX_ACTIVE is not set, it indicates TX is done from
   * Interrupt_Read_Block, now, validation of RX and TX buffer can start. 
   */
  while ( MCI_STATUS & MCI_TX_ACTIVE );

  /* Note TXEnable() is called in the Interrupt_Read_Block(). */
  MCI_TXDisable();

/*
  {
    unsigned char* p = pBuffer;
    for(i = 0; i < 512; i++)
    {
      DPRINT("%x:", p[i]);
    }
  }
*/


  return 0;

}






