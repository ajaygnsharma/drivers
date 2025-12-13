/* linux/drivers/spi/spi_lpc2xxx.c
 *
 * Copyright (c) 2008 Embedded Artists AB
 *
 * Based on spi_s3c24xx.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>

#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>

#include <asm/io.h>
#include <asm/dma.h>
#include <asm/hardware.h>

#include <asm/arch/board.h>

// SPI Register Map

#define LPC2XXX_S0SPCR  0x00
#define LPC2XXX_S0SPSR  0x04
#define LPC2XXX_S0SPDR  0x08
#define LPC2XXX_S0SPCCR 0x0C
#define LPC2XXX_S0SPINT 0x1C 

// SPI Control register bits
#define LPC2XXX_SPCR_BE        (1<<2)
#define LPC2XXX_SPCR_CPHA      (1<<3)
#define LPC2XXX_SPCR_CPOL_HIGH (1<<4)
#define LPC2XXX_SPCR_MSTR      (1<<5)
#define LPC2XXX_SPCR_LSBF      (1<<6)
#define LPC2XXX_SPCR_SPIE      (1<<7)

// SPI Status Register bits
#define LPC2XXX_SPSR_ABRT (1<<3)
#define LPC2XXX_SPSR_MODF (1<<4)
#define LPC2XXX_SPSR_ROVR (1<<5)
#define LPC2XXX_SPSR_WCOL (1<<6)
#define LPC2XXX_SPSR_SPIF (1<<7)


struct lpc2xxx_spi {
	/* bitbang has to be first */
	struct spi_bitbang	 bitbang;
	struct completion	 done;

	void __iomem		*regs;
	int			 irq;
	int			 len;
	int			 count;
	int			err;

	void			(*set_cs)(struct lpc2xxx_spi_info *spi,
					  int cs, int pol);

	/* data buffers */
	const unsigned char	*tx;
	unsigned char		*rx;

//	struct clk		*clk;
	struct resource	*ioarea;
	struct spi_master	*master;
	struct spi_device	*curdev;
	struct device		*dev;
	struct lpc2xxx_spi_info *pdata;
};

unsigned int LPC2xxx_Fcclk;
unsigned int LPC2xxx_Fpclk;

static inline struct lpc2xxx_spi *to_hw(struct spi_device *sdev)
{
	return spi_master_get_devdata(sdev->master);
}

/*
static void s3c24xx_spi_gpiocs(struct s3c2410_spi_info *spi, int cs, int pol)
{
	s3c2410_gpio_setpin(spi->pin_cs, pol);
}
*/

static void lpc2xxx_spi_chipsel(struct spi_device *spi, int value)
{
	struct lpc2xxx_spi *hw = to_hw(spi);
	unsigned int cspol = spi->mode & SPI_CS_HIGH ? 1 : 0;
	unsigned int spcon;

	switch (value) {
	case BITBANG_CS_INACTIVE:
		hw->set_cs(hw->pdata, spi->chip_select, cspol^1);
		break;

	case BITBANG_CS_ACTIVE:

		spcon = readw(hw->regs + LPC2XXX_S0SPCR);

		if (spi->mode & SPI_CPHA)
			spcon |= LPC2XXX_SPCR_CPHA;
		else
			spcon &= ~LPC2XXX_SPCR_CPHA;

		if (spi->mode & SPI_CPOL)
			spcon &= ~LPC2XXX_SPCR_CPOL_HIGH;			
		else
			spcon |= LPC2XXX_SPCR_CPOL_HIGH; // setting the bit means active low



		//spcon |= S3C2410_SPCON_ENSCK;

		/* write new configration */

		writew(spcon, hw->regs + LPC2XXX_S0SPCR);
		hw->set_cs(hw->pdata, spi->chip_select, cspol);

		break;
	}
}

static int lpc2xxx_spi_setupxfer(struct spi_device *spi,
				 struct spi_transfer *t)
{
	struct lpc2xxx_spi *hw = to_hw(spi);
	unsigned int bpw;
	unsigned int hz;
	unsigned int div;
	
	bpw = t ? t->bits_per_word : spi->bits_per_word;
	hz  = t ? t->speed_hz : spi->max_speed_hz;

	if (bpw != 8) {
		dev_err(&spi->dev, "invalid bits-per-word (%d)\n", bpw);
		return -EINVAL;
	}

	// TODO use clk_-interface instead, e.g. clk_get_rate(hw->clk)

	/* 
	 * Calculate the SPI clock (PCLK_SPI)
	 * PCLKSEL0 controls the rate of the clock signal supplied to
	 * peripheral. PCLK_SPI settings are in bit 16:17
	 * 
	 * 00 -> PCLK_SPI = CCLK/4
	 * 01 -> PCLK_SPI = CCLK
	 * 10 -> PCLK_SPI = CCLK/2
	 * 11 -> PCLK_SPI = CCLK/8
	 */
	div = ((PCLKSEL0 >> 16) & 0x03);
	switch (div) {
	case 0:
		div = LPC2xxx_Fcclk / 4;
		break;
	case 1: 
		div = LPC2xxx_Fcclk;
		break;
	case 2: 
		div = LPC2xxx_Fcclk / 2;
		break;
	case 3: 
		div = LPC2xxx_Fcclk / 8;
		break;
	}

	div = div / hz;

	/*
	 * In master mode the S0SPCCR register must be set to an even
	 * number greater than or equal to 8.
	 */

	if (div < 8)
		div = 8;

	if (div > 254)
		div = 254;

	if (div & 0x01)
		div += 1;

	dev_dbg(&spi->dev, "setting pre-scaler to %d (hz %d)\n", div, hz);
	writeb(div, hw->regs + LPC2XXX_S0SPCCR);

	spin_lock(&hw->bitbang.lock);
	if (!hw->bitbang.busy) {
		hw->bitbang.chipselect(spi, BITBANG_CS_INACTIVE);
		/* need to ndelay for 0.5 clocktick ? */
	}
	spin_unlock(&hw->bitbang.lock);

	return 0;
}

static int lpc2xxx_spi_setup(struct spi_device *spi)
{
	int ret;

	if (!spi->bits_per_word)
		spi->bits_per_word = 8;

	if ((spi->mode & SPI_LSB_FIRST) != 0)
		return -EINVAL;

	ret = lpc2xxx_spi_setupxfer(spi, NULL);
	if (ret < 0) {
		dev_err(&spi->dev, "setupxfer returned %d\n", ret);
		return ret;
	}

	dev_dbg(&spi->dev, "%s: mode %d, %u bpw, %d hz\n",
		__FUNCTION__, spi->mode, spi->bits_per_word,
		spi->max_speed_hz);

	return 0;
}

static inline unsigned int hw_txbyte(struct lpc2xxx_spi *hw, int count)
{
	return hw->tx ? hw->tx[count] : 0;
}

static int lpc2xxx_spi_txrx(struct spi_device *spi, struct spi_transfer *t)
{
	struct lpc2xxx_spi *hw = to_hw(spi);

	dev_dbg(&spi->dev, "txrx: tx %p, rx %p, len %d\n",
		t->tx_buf, t->rx_buf, t->len);

	hw->tx = t->tx_buf;
	hw->rx = t->rx_buf;
	hw->len = t->len;
	hw->err = 0;
	hw->count = 0;

	do {
		writeb(hw_txbyte(hw, hw->count), hw->regs + LPC2XXX_S0SPDR);
		wait_for_completion(&hw->done);
	} while (hw->count < hw->len || hw->err);


	return hw->count;
}

static irqreturn_t lpc2xxx_spi_irq(int irq, void *dev)
{
	struct lpc2xxx_spi *hw = dev;
	unsigned int spsta = readb(hw->regs + LPC2XXX_S0SPSR);
	unsigned int count = hw->count;

	if (spsta & LPC2XXX_SPSR_WCOL) {

		dev_dbg(hw->dev, "write collision\n");
		hw->err = 1;
		complete(&hw->done);
		goto irq_done;
	}

	if (!(spsta & LPC2XXX_SPSR_SPIF)) {
		dev_dbg(hw->dev, "spi not ready for tx?\n");
		hw->err = 1;
		complete(&hw->done);
		goto irq_done;
	}

	hw->count++;

	if (hw->rx) {
		hw->rx[count] = readb(hw->regs + LPC2XXX_S0SPDR);
	}

	count++;

	//if (count < hw->len)
	//	writeb(hw_txbyte(hw, count), hw->regs + LPC2XXX_S0SPDR);
	//else
		complete(&hw->done);

	// clear interrupt
	writeb(1, hw->regs + LPC2XXX_S0SPINT);	

 irq_done:
	return IRQ_HANDLED;
}

static int lpc2xxx_spi_probe(struct platform_device *pdev)
{
	struct lpc2xxx_spi *hw;
	struct spi_master *master;
	struct spi_board_info *bi;
	struct resource *res;
	int err = 0;
	int i;

	master = spi_alloc_master(&pdev->dev, sizeof(struct lpc2xxx_spi));
	if (master == NULL) {
		dev_err(&pdev->dev, "No memory for spi_master\n");
		err = -ENOMEM;
		goto err_nomem;
	}

	hw = spi_master_get_devdata(master);
	memset(hw, 0, sizeof(struct lpc2xxx_spi));

	hw->master = spi_master_get(master);
	hw->pdata = pdev->dev.platform_data;
	hw->dev = &pdev->dev;

	if (hw->pdata == NULL) {
		dev_err(&pdev->dev, "No platform data supplied\n");
		err = -ENOENT;
		goto err_no_pdata;
	}

	platform_set_drvdata(pdev, hw);
	init_completion(&hw->done);

	/* setup the state for the bitbang driver */

	hw->bitbang.master         = hw->master;
	hw->bitbang.setup_transfer = lpc2xxx_spi_setupxfer;
	hw->bitbang.chipselect     = lpc2xxx_spi_chipsel;
	hw->bitbang.txrx_bufs      = lpc2xxx_spi_txrx;
	hw->bitbang.master->setup  = lpc2xxx_spi_setup;

	dev_dbg(hw->dev, "bitbang at %p\n", &hw->bitbang);

	/* find and map our resources */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "Cannot get IORESOURCE_MEM\n");
		err = -ENOENT;
		goto err_no_iores;
	}

	hw->ioarea = request_mem_region(res->start, (res->end - res->start)+1,
					pdev->name);

	if (hw->ioarea == NULL) {
		dev_err(&pdev->dev, "Cannot reserve region\n");
		err = -ENXIO;
		goto err_no_iores;
	}

	hw->regs = ioremap(res->start, (res->end - res->start)+1);
	if (hw->regs == NULL) {
		dev_err(&pdev->dev, "Cannot map IO\n");
		err = -ENXIO;
		goto err_no_iomap;
	}

	hw->irq = platform_get_irq(pdev, 0);
	if (hw->irq < 0) {
		dev_err(&pdev->dev, "No IRQ specified\n");
		err = -ENOENT;
		goto err_no_irq;
	}

	err = request_irq(hw->irq, lpc2xxx_spi_irq, 0, pdev->name, hw);
	if (err) {
		dev_err(&pdev->dev, "Cannot claim IRQ\n");
		goto err_no_irq;
	}

//	hw->clk = clk_get(&pdev->dev, "spi");
//	if (IS_ERR(hw->clk)) {
//		dev_err(&pdev->dev, "No clock for device\n");
//		err = PTR_ERR(hw->clk);
//		goto err_no_clk;
//	}

	/* for the moment, permanently enable the clock */

//	clk_enable(hw->clk);

	/* program defaults into the registers */
	writew((LPC2XXX_SPCR_MSTR|LPC2XXX_SPCR_SPIE), hw->regs + LPC2XXX_S0SPCR);

	/* setup any gpio we can */

	hw->set_cs = hw->pdata->set_cs;

	/* register our spi controller */

	err = spi_bitbang_start(&hw->bitbang);
	if (err) {
		dev_err(&pdev->dev, "Failed to register SPI master\n");
		goto err_register;
	}


	/* register all the devices associated */

	bi = &hw->pdata->board_info[0];
	for (i = 0; i < hw->pdata->board_size; i++, bi++) {
		dev_info(hw->dev, "registering %s\n", bi->modalias);
		bi->controller_data = hw;
		spi_new_device(master, bi);
	}

	return 0;

 err_register:
//	clk_disable(hw->clk);
//	clk_put(hw->clk);

 err_no_clk:
	free_irq(hw->irq, hw);

 err_no_irq:
	iounmap(hw->regs);

 err_no_iomap:
	release_resource(hw->ioarea);
	kfree(hw->ioarea);

 err_no_iores:
 err_no_pdata:
	spi_master_put(hw->master);;

 err_nomem:
	return err;
}

static int lpc2xxx_spi_remove(struct platform_device *dev)
{
	struct lpc2xxx_spi *hw = platform_get_drvdata(dev);
	platform_set_drvdata(dev, NULL);

	spi_unregister_master(hw->master);

//	clk_disable(hw->clk);
//	clk_put(hw->clk);

	free_irq(hw->irq, hw);
	iounmap(hw->regs);

	release_resource(hw->ioarea);
	kfree(hw->ioarea);

	spi_master_put(hw->master);
	return 0;
}


#ifdef CONFIG_PM

static int lpc2xxx_spi_suspend(struct platform_device *pdev, pm_message_t msg)
{
	// TODO
	return 0;
}

static int lpc2xxx_spi_resume(struct platform_device *pdev)
{
	// TODO
	return 0;
}

#else
#define lpc2xxx_spi_suspend NULL
#define lpc2xxx_spi_resume  NULL
#endif

static struct platform_driver lpc2xxx_spidrv = {
	.probe		= lpc2xxx_spi_probe,
	.remove		= lpc2xxx_spi_remove,
	.suspend	= lpc2xxx_spi_suspend,
	.resume		= lpc2xxx_spi_resume,
	.driver		= {
		.name	= "lpc2xxx-spi",
		.owner	= THIS_MODULE,
	},
};

static int __init lpc2xxx_spi_init(void)
{
        return platform_driver_register(&lpc2xxx_spidrv);
}

static void __exit lpc2xxx_spi_exit(void)
{
        platform_driver_unregister(&lpc2xxx_spidrv);
}

module_init(lpc2xxx_spi_init);
module_exit(lpc2xxx_spi_exit);

MODULE_DESCRIPTION("LPC2XXX SPI Driver");
MODULE_AUTHOR("Embedded Artists AB, <support@embeddedartists.com>");
MODULE_LICENSE("GPL");
