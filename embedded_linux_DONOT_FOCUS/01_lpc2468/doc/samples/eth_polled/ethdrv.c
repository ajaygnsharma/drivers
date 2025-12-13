/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2009 Embedded Artists AB
 *
 * Description:
 *    Ethernet driver
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <printf_P.h>
#include <stdlib.h>
#include <string.h>
#include "general.h"
#include "lpc24xx.h"
#include "delay.h"


/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

#define MAX_PHY_INIT_RETRY 10

/* EMAC MODULE ID   */
#define OLD_EMAC_MODULE_ID  ((0x3902 << 16) | 0x2000)

/* MAC registers and parameters */
#define PCONP_MAC_CLOCK    0x40000000

#define SPEED_100           1
#define SPEED_10            0
#define FULL_DUPLEX         1
#define HALF_DUPLEX         0

#define MAC_RAM_ADDR       0x7FE00000
#define MAC_RAM_SIZE       0x00004000

#define MAC_TX_DESCRIPTOR_COUNT    5
#define MAC_RX_DESCRIPTOR_COUNT    4

/*
 * (Rx|Tx)Descriptor ::
 *   [4] packet  - base address of the buffer containing the data
 *   [4] control - control information
 */
#define TX_DESCRIPTOR_SIZE  (MAC_TX_DESCRIPTOR_COUNT * 8)
#define RX_DESCRIPTOR_SIZE  (MAC_RX_DESCRIPTOR_COUNT * 8)

/* 
 * TxStatus ::
 *   [4] status
 */
#define TX_STATUS_SIZE      (MAC_TX_DESCRIPTOR_COUNT * 4)

/* 
 * RxStatus ::
 *   [4] status        - receive status flags
 *   [4] statusHashCRC - concat of dest addr hash CRC and src addr CRC
 */
#define RX_STATUS_SIZE      (MAC_RX_DESCRIPTOR_COUNT * 8)


#define TOTAL_DESCRIPTOR_SIZE   (TX_DESCRIPTOR_SIZE + RX_DESCRIPTOR_SIZE + TX_STATUS_SIZE + RX_STATUS_SIZE)

/* descriptors are placed at the end of the emac address space */
#define MAC_DESCRIPTOR_ADDR    (MAC_RAM_ADDR + MAC_RAM_SIZE - TOTAL_DESCRIPTOR_SIZE) 

#define TX_DESCRIPTOR_ADDR  MAC_DESCRIPTOR_ADDR
#define TX_STATUS_ADDR      (MAC_DESCRIPTOR_ADDR + TX_DESCRIPTOR_SIZE)
#define RX_DESCRIPTOR_ADDR  (TX_STATUS_ADDR + TX_STATUS_SIZE)
#define RX_STATUS_ADDR      (RX_DESCRIPTOR_ADDR + RX_DESCRIPTOR_SIZE)

#define MAC_DMA_ADDR       MAC_RAM_ADDR
#define MAC_DMA_SIZE       MAC_RAM_ADDR + MAC_RAM_END - TOTAL_DESCRIPTOR_SIZE


#define MAC_BLOCK_SIZE         0x600
#define MAC_TX_BLOCK_NUM       5   
#define MAC_RX_BLOCK_NUM       5
#define TOTAL_MAC_BLOCK_NUM    10

#define MAC_BUFFER_SIZE    (MAC_BLOCK_SIZE * (MAC_TX_BLOCK_NUM + MAC_RX_BLOCK_NUM ))
#define MAC_TX_BUFFER_ADDR MAC_RAM_ADDR
#define MAC_RX_BUFFER_ADDR (MAC_RAM_ADDR + MAC_BLOCK_SIZE * MAC_TX_BLOCK_NUM) 


/* EMAC Descriptor TX and RX Control fields */
#define MAC_TX_DESC_INT        0x80000000
#define MAC_TX_DESC_LAST       0x40000000
#define MAC_TX_DESC_CRC        0x20000000
#define MAC_TX_DESC_PAD        0x10000000
#define MAC_TX_DESC_HUGE       0x08000000
#define MAC_TX_DESC_OVERRIDE   0x04000000

#define MAC_RX_DESC_INT        0x80000000

/* EMAC Descriptor status related definition */
#define TX_DESC_STATUS_ERR      0x80000000
#define TX_DESC_STATUS_NODESC   0x40000000
#define TX_DESC_STATUS_UNDERRUN 0x20000000
#define TX_DESC_STATUS_LCOL     0x10000000
#define TX_DESC_STATUS_ECOL     0x08000000
#define TX_DESC_STATUS_EDEFER   0x04000000
#define TX_DESC_STATUS_DEFER    0x02000000
#define TX_DESC_STATUS_COLCNT   0x01E00000  /* four bits, it's a mask, not exact count */

#define RX_DESC_STATUS_ERR      0x80000000
#define RX_DESC_STATUS_LAST     0x40000000
#define RX_DESC_STATUS_NODESC   0x20000000
#define RX_DESC_STATUS_OVERRUN  0x10000000
#define RX_DESC_STATUS_ALGNERR  0x08000000
#define RX_DESC_STATUS_RNGERR   0x04000000
#define RX_DESC_STATUS_LENERR   0x02000000
#define RX_DESC_STATUS_SYMERR   0x01000000
#define RX_DESC_STATUS_CRCERR   0x00800000
#define RX_DESC_STATUS_BCAST    0x00400000
#define RX_DESC_STATUS_MCAST    0x00200000
#define RX_DESC_STATUS_FAILFLT  0x00100000
#define RX_DESC_STATUS_VLAN     0x00080000
#define RX_DESC_STATUS_CTLFRAM  0x00040000

#define DESC_SIZE_MASK          0x000007FF  /* 11 bits for both TX and RX */

/* EMAC interrupt controller related definition */
#define MAC_INT_RXOVERRUN  0x01 << 0
#define MAC_INT_RXERROR    0x01 << 1 
#define MAC_INT_RXFINISHED 0x01 << 2
#define MAC_INT_RXDONE     0x01 << 3 
#define MAC_INT_TXUNDERRUN 0x01 << 4
#define MAC_INT_TXERROR    0x01 << 5 
#define MAC_INT_TXFINISHED 0x01 << 6
#define MAC_INT_TXDONE     0x01 << 7 
#define MAC_INT_SOFTINT    0x01 << 12
#define MAC_INT_WOL        0x01 << 13 


/* Micrel KSZ8001 PHY related registers */

/* PHY_ADDR, by default, AD0 has pull-up, AD1~4 have pull-downs, 
so, the default address is 0x0001 */
//#define PHY_ADDR        (0x0001 << 8)   /* in MAC_MADR, bit 8~12 */

static volatile int phyAddr = 0;
#define NATIONAL_PHY    0
#define MICREL_PHY      1
static volatile int phyType = MICREL_PHY;

#define PHY_BMCR        0x0000
#define PHY_BMSR        0x0001
#define PHY_PHYIDR1     0x0002
#define PHY_PHYIDR2     0x0003
#define PHY_ANAR        0x0004
#define PHY_ANLPAR      0x0005
#define PHY_ANLPARNP    0x0005
#define PHY_ANER        0x0006
#define PHY_ANNPTR      0x0007

#define PHY_PHYSTS      0x0010
#define PHY_MICR        0x0011
#define PHY_MISR        0x0012
#define PHY_RESERVED1   0x0013
#define PHY_FCSCR       0x0014
#define PHY_RECR        0x0015
#define PHY_PCSR        0x0016
#define PHY_RBR         0x0017
#define PHY_LEDCR       0x0018
#define PHY_PHYCR       0x0019
#define PHY_10BTSCR     0x001A
#define PHY_CDCTRL1     0x001B
#define PHY_RESERVED2   0x001C
#define PHY_EDCR        0x001D

/* Below is the Micrel PHY definition */
#define MIC_PHY_RXER_CNT			0x0015
#define MIC_PHY_INT_CTRL			0x001B
#define MIC_PHY_LINKMD_CTRL			0x001D
#define MIC_PHY_PHY_CTRL			0x001E
#define MIC_PHY_100BASE_PHY_CTRL	0x001F

/* BMCR setting */
#define BMCR_RESET          0x8000
#define BMCR_LOOPBACK       0x4000
#define BMCR_SPEED_100      0x2000
#define BMCR_AN             0x1000
#define BMCR_POWERDOWN      0x0800
#define BMCR_ISOLATE        0x0400
#define BMCR_RE_AN          0x0200
#define BMCR_DUPLEX         0x0100

/* BMSR setting */
#define BMSR_100BE_T4       0x8000
#define BMSR_100TX_FULL     0x4000
#define BMSR_100TX_HALF     0x2000
#define BMSR_10BE_FULL      0x1000
#define BMSR_10BE_HALF      0x0800
#define BMSR_AUTO_DONE      0x0020
#define BMSR_REMOTE_FAULT   0x0010
#define BMSR_NO_AUTO        0x0008
#define BMSR_LINK_ESTABLISHED   0x0004

#define MII_BMSR_TIMEOUT    0x1000000

#define CMD_RX_ENABLE 0x01
#define CMD_TX_ENABLE 0x02


#define m_nic_read(reg) (reg)
#define m_nic_write(reg, data) (reg) = (data)
#define m_nic_bfs(reg, data) (reg) |= (data)
#define m_nic_bfc(reg, data) (reg) &= ~(data)

#define m_nic_read2(reg) (*(volatile unsigned long *)(reg))
#define m_nic_write2(reg, data) ((*(volatile unsigned long *)(reg)) = (volatile unsigned long)(data))
#define m_nic_bfs2(reg, data) (*(volatile unsigned long *)(reg)) |= (data)
#define m_nic_bfc2(reg, data) (*(volatile unsigned long *)(reg)) &= ~(data)

/******************************************************************************
 * Local variables
 *****************************************************************************/

static volatile tU32 emacDuplex;
static volatile tU32 emacSpeed;
static unsigned char macAddr[6];

/******************************************************************************
 * Implementation of local functions
 *****************************************************************************/

/*****************************************************************************
 *
 * Description:
 *    Write PHY register
 *
 ****************************************************************************/
static void 
writePhy(tU32 phyReg, 
         tU32 phyData)
{
  // write command
  m_nic_write(MAC_MCMD, 0x0000);              

  // [12:8] == PHY addr, [4:0]=0x00(BMCR) register addr
  m_nic_write(MAC_MADR, (phyAddr | phyReg)); 
  m_nic_write(MAC_MWTD,  phyData);
  while ( m_nic_read(MAC_MIND) != 0 );
}

/*****************************************************************************
 *
 * Description:
 *    Read PHY register
 *
 ****************************************************************************/
static tU32 
readPhy(tU32 phyReg)
{

  // read command
  m_nic_write(MAC_MCMD, 0x0001);

  // [12:8] == PHY addr, [4:0]=0x00(BMCR) register addr
  m_nic_write(MAC_MADR, (phyAddr | phyReg));

  while ( m_nic_read(MAC_MIND) != 0 );

  m_nic_write(MAC_MCMD, 0x0000);

  return m_nic_read(MAC_MRDD);
}

static int 
check_Micrel_KSZ8001L(void)
{
  volatile tU32 regValue = 0;

  // See Micrel KSZ8001L Users Manual for more details
  regValue = readPhy(PHY_PHYIDR1);
  if ( (regValue & 0xffff) != 0x0022)
  {
    return FALSE;
  }

  regValue = readPhy(PHY_PHYIDR2);
  if ( (regValue & 0xfff0) != 0x1610)
  {
    return FALSE;
  }
  return TRUE;
}


static int 
check_National_DP83848(void)
{
  volatile tU32 regValue = 0;

  // See Micrel KSZ8001L Users Manual for more details
  regValue = readPhy(PHY_PHYIDR1);
  if ( (regValue & 0x2000) != 0x2000)
  {
    return FALSE;
  }

  regValue = readPhy(PHY_PHYIDR2);
  if ( (regValue & 0x5C90) != 0x5C90)
  {
    return FALSE;
  }
  return TRUE;
}

/*****************************************************************************
 *
 * Description:
 *    Initialize the PHY
 *
 ****************************************************************************/
static int 
phyInit(void)
{
  tU32 regValue = 0;
  tU32 timeout = 0;  
  
  // MII Mgmt. Divided by 20.
  m_nic_write(MAC_MCFG, 0x0018);
  m_nic_write(MAC_MCMD, 0);

  // RMII configuration
  m_nic_bfs(MAC_COMMAND,  0x0200); 
  m_nic_write(MAC_SUPP,  0x0900);  /* RMII setting, PHY support: [8]=0 ->10 Mbps mode, =1 -> 100 Mbps mode */
  /*  (note bit 4 was set in original test, although spec says its unused) */  
  
  mdelay(50);
  m_nic_write(MAC_SUPP,  0x0100);

  
  // check PHY IDs.
  phyAddr = 0;
  if (check_Micrel_KSZ8001L() == FALSE)
  {
    phyAddr = 0x100;
    if (check_National_DP83848() == FALSE)
    {
      printf("emac: cannot identify external PHY\n");
      phyAddr = 0;
      printf(": [0]->(%x, %x)", readPhy(PHY_PHYIDR1), readPhy(PHY_PHYIDR2));
      phyAddr = 0x100;
      printf(": [1]->(%x, %x)", readPhy(PHY_PHYIDR1), readPhy(PHY_PHYIDR2));
      return -1;
    }
    else
    {
      phyType = NATIONAL_PHY;
      printf("emac: found National DP83848 PHY\n");
    }
  }
  else
  {
    phyType = MICREL_PHY;
    printf("emac: found Micrel KSZ8001L PHY\n");
  }
  
  
  // software reset of PHY. The bit is self-clearing
  writePhy(PHY_BMCR, BMCR_RESET);
  mdelay(100);    
  
  
  timeout = MII_BMSR_TIMEOUT * 4;  
    
  while (timeout != 0)
  {
    regValue = readPhy(PHY_BMCR);

    
    if ((regValue & BMCR_RESET) == 0x0000)
      {
        // reset bit has been cleared
        break;
      }
    timeout--;
  }  
  
  if (timeout == 0)
  {
    printf("  Error: phyInit failed to reset PHY\n");
    return -1;//-EFAULT;
  }



  return 0; 
}

/*****************************************************************************
 *
 * Description:
 *    Initialize TX descriptors
 *
 ****************************************************************************/
static void 
emacTxDescriptorInit(void)
{
  int i = 0;
  tU32* txDescAddr   = NULL;
  tU32* txStatusAddr = NULL;


  // base address of tx descriptor array
  m_nic_write(MAC_TXDESCRIPTOR, TX_DESCRIPTOR_ADDR);

  // base address of tx status
  m_nic_write(MAC_TXSTATUS, TX_STATUS_ADDR);

  // number of tx descriptors
  m_nic_write(MAC_TXDESCRIPTORNUM, MAC_TX_DESCRIPTOR_COUNT-1);

  for (i = 0; i < MAC_TX_DESCRIPTOR_COUNT; i++)
  {
    txDescAddr = (tU32 *)(TX_DESCRIPTOR_ADDR + i * 8);
    m_nic_write2(txDescAddr, (MAC_TX_BUFFER_ADDR + i * MAC_BLOCK_SIZE));

    // control field in descriptor
    txDescAddr++;
    m_nic_write2(txDescAddr, (MAC_TX_DESC_INT | (MAC_BLOCK_SIZE-1)));
  }

  for (i = 0; i < MAC_TX_DESCRIPTOR_COUNT; i++)
  {

    txStatusAddr = (tU32 *)(TX_STATUS_ADDR + i * 4);

    // set status info to 0
    m_nic_write2(txStatusAddr, 0);
  }

  m_nic_write(MAC_TXPRODUCEINDEX, 0);
}

/*****************************************************************************
 *
 * Description:
 *    Initialize RX descriptors
 *
 ****************************************************************************/
static void 
emacRxDescriptorInit(void) 
{
  int i;
  tU32* rxDescAddr   = NULL;
  tU32* rxStatusAddr = NULL;


  // base address of rx descriptor array
  m_nic_write(MAC_RXDESCRIPTOR, RX_DESCRIPTOR_ADDR);

  // base address of rx status
  m_nic_write(MAC_RXSTATUS, RX_STATUS_ADDR);

  // number of rx descriptors
  m_nic_write(MAC_RXDESCRIPTORNUM,  MAC_RX_DESCRIPTOR_COUNT-1);

  for (i = 0; i < MAC_RX_DESCRIPTOR_COUNT; i++)
  {
    rxDescAddr = (tU32 *)(RX_DESCRIPTOR_ADDR + i * 8);
    m_nic_write2(rxDescAddr, (MAC_RX_BUFFER_ADDR + i * MAC_BLOCK_SIZE));
    rxDescAddr++;
    m_nic_write2(rxDescAddr, (MAC_RX_DESC_INT | ((MAC_BLOCK_SIZE - 1) & DESC_SIZE_MASK) ) );
  }

  for (i = 0; i < MAC_RX_DESCRIPTOR_COUNT; i++)
  {
    // RX status, two words, status info. and status hash CRC.
    rxStatusAddr = (tU32 *)(RX_STATUS_ADDR + i * 8);  
    m_nic_write2(rxStatusAddr, 0);
    rxStatusAddr++;
    m_nic_write2(rxStatusAddr, 0);
  }

  m_nic_write(MAC_RXCONSUMEINDEX, 0);
}

/*****************************************************************************
 *
 * Description:
 *    Initialize the Link
 *
 ****************************************************************************/
static int 
initLink(void)
{
  tU32 regValue  = 0;
  tU32 regValue2 = 0;
  tU32 timeout   = 0;
  int tries     = 0;
  
  do {

    // auto negotiation
    writePhy(PHY_BMCR, (BMCR_AN | BMCR_RE_AN));

    timeout = 10*20;  //10 sec (10 * 50ms)

    while ( timeout != 0 )
    {
    
      if (phyType == MICREL_PHY)
      {
        regValue = readPhy(MIC_PHY_100BASE_PHY_CTRL);
        
        if (((regValue & 0x001c) != 0x0000) && ((regValue & 0x0080) == 0x0080))
        {
          // link established when AN status bit (bit 7) is set
          break;
        }        
        
      }
      else
      {
        regValue = readPhy(PHY_PHYSTS);


        if ((regValue & 0x0011) == 0x0011 )
        {
          // link established when link status bit (bit 2) is set
          break;
        }
      }
      
      timeout--;

      //print progress marker
      switch(timeout%4)
      {
        case 0:  printf("\b\\"); break;
        case 1:  printf("\b-"); break;
        case 2:  printf("\b/"); break;
        case 3:  printf("\b|"); break;
        default: printf("\b "); break;
      }
      
      mdelay(15);
      
    }

    if ( timeout == 0 )
    {
      printf("\n\rNO LINK\n");
      if(tries++ < 2)
      {
        if(phyInit() >= 0)
          continue;
          
      }
      return -1;//-ENETDOWN;
    }
    else
      break;

  } while(1);

  
#if 0  
  // Link established from here on
  regValue2 = readPhy(0x1e); 
  regValue2 = readPhy(0x1e); 

  printf("\remac: (reg 0x1e=0x%4x, reg 0x1f=0x%4x), link status = ", regValue2, regValue);

  // check duplex mode and speed
  if (regValue & 0x04)
  {
    emacDuplex = FULL_DUPLEX;
    printf("full duplex");
  }
  else
  {
    emacDuplex = HALF_DUPLEX;
    printf("half duplex");
  }

  //if (regValue & 0x04)
  if (regValue & 0x02)
  {
    emacSpeed = SPEED_10;
    printf(", 10Mbps");
  }
  else
  {
    emacSpeed = SPEED_100;
    printf(", 100Mbps");
  }

  printf("\n");
#endif


  printf("\r\nlink status = ");
  if (phyType == MICREL_PHY)
  {
    regValue = readPhy( MIC_PHY_100BASE_PHY_CTRL );
	  /* successful negotiations; update link info */
	  regValue &= 0x001C;
	  switch ( regValue )
	  {
	  case 0x0004:
		  emacSpeed = SPEED_10;
		  emacDuplex = HALF_DUPLEX;
      printf("10Mbps, half duplex\n");
	    break;
	  case 0x0008:
		  emacSpeed = SPEED_100;
		  emacDuplex = HALF_DUPLEX;
      printf("100Mbps, half duplex\n");
	    break;
	  case 0x0014:
		  emacSpeed = SPEED_10;
		  emacDuplex = FULL_DUPLEX;
      printf("10Mbps, full duplex\n");
	    break;
	  case 0x0018:
		  emacSpeed = SPEED_100;
		  emacDuplex = FULL_DUPLEX;
      printf("100Mbps, full duplex\n");
	    break;
	  default:	// Should not come here, force to set default, 100 FULL_DUPLEX
		  emacSpeed = SPEED_100;
		  emacDuplex = FULL_DUPLEX;
      printf("forced to 100Mbps, full duplex\n");
		  break;
	  }
  }
  else
  {
    regValue = readPhy( PHY_PHYSTS );

	  /* Link established from here on */
  	if ( regValue & 0x02 )
    {
	    emacSpeed = SPEED_10;
      printf("10Mbps, ");
    }
	  else
    {
	    emacSpeed = SPEED_100;
      printf("100Mbps, ");
    }

	  if ( regValue & 0x04 )
    {
	    emacDuplex = FULL_DUPLEX;
      printf("full duplex\n");
    }
	  else
    {
	    emacDuplex = HALF_DUPLEX;
      printf("half duplex\n");
    }

  }  
  


  return 0;
}

/*****************************************************************************
 *
 * Description:
 *    Setup Link and determin speed and duplex 
 *
 ****************************************************************************/
static int 
setupLink(void)
{
  int ret = 0;

  if ((ret = initLink()) < 0)
    return ret;

  // 10 Mbps and half-duplex
  if (emacSpeed == SPEED_10 && emacDuplex == HALF_DUPLEX)
  {
    // CRC and PAD enabled
    m_nic_write(MAC_MAC2, 0x30);

    // 10 Mbps mode
    m_nic_bfc(MAC_SUPP, 0x0100);

    // PassRuntFrame and RMII
    m_nic_bfc(MAC_COMMAND, 0x0240);

    // back-to-back Inter-Packet-Gap register. Recommended value from manual.
    m_nic_write(MAC_IPGT, 0x12);
  }
  else if (emacSpeed == SPEED_100 && emacDuplex == HALF_DUPLEX)
  {
    // CRC and PAD enabled
    m_nic_write(MAC_MAC2, 0x30);

    // 100 Mbps mode
    m_nic_bfs(MAC_SUPP, 0x0100);

    // PassRuntFrame and RMII
    m_nic_bfs(MAC_COMMAND, 0x0240);

    // back-to-back Inter-Packet-Gap register. Recommended value from manual.
    m_nic_write(MAC_IPGT, 0x12);
  }
  else if (emacSpeed == SPEED_10 && emacDuplex == FULL_DUPLEX)
  {
    // CRC and PAD enabled, Full-Duplex
    m_nic_write(MAC_MAC2, 0x31);

    // 10 Mbps mode
    m_nic_bfc(MAC_SUPP, 0x0100);

    // PassRuntFrame, RMII and Full-Duplex
    m_nic_bfs(MAC_COMMAND, 0x0640);

    // back-to-back Inter-Packet-Gap register. Recommended value from manual.
    m_nic_write(MAC_IPGT, 0x15);
  }
  else if ( emacSpeed == SPEED_100 && emacDuplex == FULL_DUPLEX )
  {
    // CRC and PAD enabled, Full-Duplex
    m_nic_write(MAC_MAC2, 0x31);

    // 100 Mbps mode
    m_nic_bfs(MAC_SUPP, 0x0100);

    // PassRuntFrame, RMII and Full-Duplex
    m_nic_bfs(MAC_COMMAND, 0x0640);

    // back-to-back Inter-Packet-Gap register. Recommended value from manual.
    m_nic_write(MAC_IPGT, 0x15);
  }

  return 0;

}

/*****************************************************************************
 *
 * Description:
 *    Check interrupt status 
 *
 ****************************************************************************/
static void 
emac_interrupt(void)
{
  volatile tU32 regValue = 0;

  regValue = m_nic_read(MAC_INTSTATUS);

  do
  {
    if (regValue == 0)
    {
      break;
    }

    if (regValue & MAC_INT_RXOVERRUN)
    {
      m_nic_write(MAC_INTCLEAR, MAC_INT_RXOVERRUN);
      printf("RO\n");      
      break;
    }

    if (regValue & MAC_INT_RXERROR)
    {
      m_nic_write(MAC_INTCLEAR, MAC_INT_RXERROR);
      //printf("RE\n");
      break;   
    }

    if (regValue & MAC_INT_RXFINISHED)
    {
      m_nic_write(MAC_INTCLEAR, MAC_INT_RXFINISHED);   
      //while ( MAC_RXPRODUCEINDEX != (MAC_RXCONSUMEINDEX - 1) );
      //printf("RF\n");      
    }

    if (regValue & MAC_INT_RXDONE)
    {
      m_nic_write(MAC_INTCLEAR, MAC_INT_RXDONE);
      //ethernetif_input(myNetif);


    }

    if (regValue & MAC_INT_TXUNDERRUN)
    {
      m_nic_write(MAC_INTCLEAR, MAC_INT_TXUNDERRUN);
      printf("TU\n");      
      break;  
    }

    if (regValue & MAC_INT_TXERROR)
    {
      m_nic_write(MAC_INTCLEAR, MAC_INT_TXERROR);
      printf("TE\n");      
      break;  
    }

    if (regValue & MAC_INT_TXFINISHED)
    {
      m_nic_write(MAC_INTCLEAR, MAC_INT_TXFINISHED);      
    }

    if (regValue & MAC_INT_TXDONE)
    {
      m_nic_write(MAC_INTCLEAR, MAC_INT_TXDONE);
      //printf("TX\n");      
      //emac_tx(dev);
    }

  } while (0);

  VICVectAddr = 0; /* Acknowledge Interrupt */  
  
}

/*****************************************************************************
 *
 * Description:
 *    Initialize the MAC controller 
 *
 ****************************************************************************/
static 
int emacInit(void)
{
  tU32 regValue = 0;
  int ret = 0;
  
  // turn on the ethernet MAC clock in PCONP, bit 30 
  m_nic_bfs(PCONP, PCONP_MAC_CLOCK);


  /*------------------------------------------------------------------------------
   * write to PINSEL2/3 to select the PHY functions on P1[17:0]
   *-----------------------------------------------------------------------------*/
  /* documentation needs to be updated */

  /* P1.6, ENET-TX_CLK, has to be set for EMAC to address a BUG in the engineering 
  version, even if this pin is not used for RMII interface. This bug has been fixed,
  and this port pin can be used as GPIO pin in the future release. */ 
  /* Unfortunately, this MCB2300 board still has the old eng. version LPC23xx chip
  on it. */

  /*------------------------------------------------------
   * Write to PINSEL2/3 to select the PHY functions on P1[17:0]
   * P1.6, ENET-TX_CLK, has to be set for Rev '-' devices and it
   * must not be set for Rev 'A’ and newer devices
   *------------------------------------------------------*/

  regValue = m_nic_read(MAC_MODULEID);  
  
  if ( regValue == OLD_EMAC_MODULE_ID )
  {       
    
    /* On Rev. '-', MAC_MODULEID should be equal to
       OLD_EMAC_MODULE_ID, P1.6 should be set. 
     */
    
    m_nic_write(PINSEL2, 0x50151105);
    /* selects P1[0,1,4,6,8,9,10,14,15] */
  }
  else
  {       
    
    /* on rev. 'A', MAC_MODULEID should not equal to
       OLD_EMAC_MODULE_ID, P1.6 should not be set. */
    m_nic_bfs(PINSEL2, 0x50150105);
    
    /* selects P1[0,1,4,8,9,10,14,15] */
  }

  m_nic_bfs(PINSEL3, 0x00000005);   /* selects P1[17:16] */

  // reset MAC modules, tx, mcs_tx, rx, mcs_rx, simulation and soft reset 
  m_nic_write(MAC_MAC1, 0xCF00);

  // reset datapaths and host registers
  m_nic_write(MAC_COMMAND, 0x0038);

  // short delay after reset
  mdelay(10);

  m_nic_write(MAC_MAC1, 0x0);

  // disable Tx
  m_nic_bfc(MAC_COMMAND, CMD_TX_ENABLE);
  // disable Rx
  m_nic_bfc(MAC_COMMAND, CMD_RX_ENABLE);

  // non back-to-back Inter-Packet-Gap register
  // The manual recommends the value 0x12
  m_nic_write(MAC_IPGR, 0x12);

  // collision window/retry register. Using recommended value from manual.
  m_nic_write(MAC_CLRT, 0x370F);


 
  // intialize PHY. emacSpeed and emacDuplex will be set in phyInit
  if ((ret = phyInit()) < 0)
    return ret;  
  
  // write the mac address
  m_nic_write(MAC_SA0, (macAddr[5] << 8 | macAddr[4]));
  m_nic_write(MAC_SA1, (macAddr[3] << 8 | macAddr[2]));
  m_nic_write(MAC_SA2, (macAddr[1] << 8 | macAddr[0]));

  setupLink();
  
  emacTxDescriptorInit();
  emacRxDescriptorInit();  
  
  // pass all receive frames
  m_nic_bfs(MAC_MAC1, 0x0002);

  // set up the Rx filter 
  // [0]-AllUnicast, [1]-AllBroadCast, [2]-AllMulticast, [3]-UnicastHash
  // [4]-MulticastHash, [5]-Perfect, [12]-MagicPacketEnWoL, [13]-RxFilterEnWoL
  m_nic_write(MAC_RXFILTERCTRL, 0x0022);


  // clear all interrupts
  m_nic_write(MAC_INTCLEAR, 0xFFFF);
        

  // enable interrupts (not SoftInt and WoL)
  m_nic_write(MAC_INTENABLE, 0x00FF);
  
  // enable Rx & Tx
  m_nic_bfs(MAC_COMMAND, CMD_RX_ENABLE);
  m_nic_bfs(MAC_COMMAND, CMD_TX_ENABLE);
  m_nic_bfs(MAC_MAC1,    0x01);  

  return 0;
}

/******************************************************************************
 * Implementation of public functions
 *****************************************************************************/

/*****************************************************************************
 *
 * Description:
 *    Resets the NIC and initializes all required hardware registers. 
 *
 * Params:
 *    [in] pEthAddr - the ethernet address (MAC) that should be assigned to 
 *                    the driver. 
 *
 ****************************************************************************/
void
ethIf_init(tU8* pEthAddr)
{

  memcpy(macAddr, pEthAddr, 6);

  /* the reset failed */
  if (emacInit() != 0)
    return;

}

/*****************************************************************************
 *
 * Description:
 *    Send an ethernet packet. 
 *
 * Params:
 *    [in] buf - the data to send
 *    [in] length   - length of the data to send
 *
 ****************************************************************************/
void
ethIf_send(tU8* buf,
           tU16 length)
{
  
  tU32 txProduceIndex = 0;
  tU32 txConsumeIndex = 0;
  tU8* pData          = 0;
  tU32 len            = length;
  tU32 sendLen        = 0;
  tU32* tx_desc_addr   = 0;
  
  txProduceIndex = m_nic_read(MAC_TXPRODUCEINDEX);
  txConsumeIndex = m_nic_read(MAC_TXCONSUMEINDEX);

  if (txConsumeIndex != txProduceIndex)
  {
    // TODO why return here? This just means that the transmit array isn't empty
    printf("emac: emac_tx transmit array isn't empty\n");
    return;
  }

  if (len > MAC_TX_BLOCK_NUM*MAC_BLOCK_SIZE)
  {
    printf("emac: block too large to transmit (%d)\n", len);
    return;
  }

  if (txProduceIndex == MAC_TX_DESCRIPTOR_COUNT)
  {
    // should never happen
    // TODO remove
    printf("emac: emac_tx produce index == count\n");
  }  
  

  if (len > 0)
  {
    pData = (tU8*)MAC_TX_BUFFER_ADDR;
    memcpy(pData, buf, length); 

    do
    {
      tx_desc_addr = (tU32*) (TX_DESCRIPTOR_ADDR + txProduceIndex * 8);

      sendLen = len;
      if (sendLen > MAC_BLOCK_SIZE)
      {
        sendLen = MAC_BLOCK_SIZE;
      }
      else
      {
        // last fragment
        sendLen |= MAC_TX_DESC_LAST;
      }

      m_nic_write2(tx_desc_addr,   (tU32)pData);
      tx_desc_addr++;
      m_nic_write2(tx_desc_addr, (tU32)(MAC_TX_DESC_INT | (sendLen -1)));

      txProduceIndex++;
      if (txProduceIndex == MAC_TX_DESCRIPTOR_COUNT)
      {
        txProduceIndex = 0;
      }

      m_nic_write(MAC_TXPRODUCEINDEX, txProduceIndex);

      len   -= (sendLen & ~MAC_TX_DESC_LAST);
      pData += (sendLen & ~MAC_TX_DESC_LAST);
    } while (len > 0);

  }

}

static tU16
getPacket(tU8* pBuf, tU16 bufLen)
{
  tU32 len;

  tU32 rxProduceIndex = 0;
  tU32 rxConsumeIndex = 0;
  tU32* rxStatusAddr  = 0;
  tU32* recvAddr      = 0;
  tU8* pData = NULL;
  
  rxProduceIndex = m_nic_read(MAC_RXPRODUCEINDEX);
  rxConsumeIndex = m_nic_read(MAC_RXCONSUMEINDEX);
  
  if (rxConsumeIndex == rxProduceIndex)
    return NULL;
  

  rxStatusAddr = (tU32*)(RX_STATUS_ADDR + rxConsumeIndex * 8);

  len =(tU32) m_nic_read2(rxStatusAddr);    

  if ((len & RX_DESC_STATUS_LAST) == 0)
  {
    // TODO: could this occur when MAC_BLOCK_SIZE == 0x0600?
    printf("getPacket: NOT LAST fragment\n");    
  }

  len = (len & DESC_SIZE_MASK) + 1; 

  if (len > bufLen)
  {
    printf("getPacket: buffer too small (%d < %d)\n", bufLen, len);
    len = bufLen;
  }

  recvAddr = (tU32*)(RX_DESCRIPTOR_ADDR + rxConsumeIndex * 8);    
  pData = (tU8*)m_nic_read2(recvAddr); 

  memcpy(pBuf, pData, len);
  

  rxConsumeIndex++;
  if (rxConsumeIndex == MAC_RX_DESCRIPTOR_COUNT)
  {
    rxConsumeIndex = 0;
  }
  m_nic_write(MAC_RXCONSUMEINDEX, rxConsumeIndex);  
  
  return len;  
}

/*****************************************************************************
 *
 * Description:
 *    Poll the driver
 *
 * Params:
 *    [in] pBuf - allocated buffer to which data will be copied
 *    [in] len  - length of buffer
 *
 * Returns:
 *    Number of copied bytes to the supplied buffer
 *
 ****************************************************************************/
tU16
ethIf_poll(tU8* pBuf,
           tU16 len)

{
  emac_interrupt();

  return getPacket(pBuf, len);
}


