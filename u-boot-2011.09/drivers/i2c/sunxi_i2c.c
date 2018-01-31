/*
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* This code should work for both the S3C2400 and the S3C2410
 * as they seem to have the same I2C controller inside.
 * The different address mapping is handled by the s3c24xx.h files below.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/twi.h>
#include <sys_config.h>
#include <asm/arch/timer.h>
#include <asm/arch/ccmu.h>
#include <asm/io.h>
#include <asm/arch/platform.h>

#define	I2C_WRITE		0
#define I2C_READ		1

#define I2C_OK			0
#define I2C_NOK			1
#define I2C_NACK		2
#define I2C_NOK_LA		3	/* Lost arbitration */
#define I2C_NOK_TOUT	4	/* time out */

#define I2C_START_TRANSMIT     0x08
#define I2C_RESTART_TRANSMIT   0x10
#define I2C_ADDRWRITE_ACK	   0x18
#define I2C_ADDRREAD_ACK	   0x40
#define I2C_DATAWRITE_ACK      0x28
#define I2C_READY			   0xf8
#define I2C_DATAREAD_NACK	   0x58
#define I2C_DATAREAD_ACK	   0x50
/* status or interrupt source */
/*------------------------------------------------------------------------------
* Code   Status
* 00h    Bus error
* 08h    START condition transmitted
* 10h    Repeated START condition transmitted
* 18h    Address + Write bit transmitted, ACK received
* 20h    Address + Write bit transmitted, ACK not received
* 28h    Data byte transmitted in master mode, ACK received
* 30h    Data byte transmitted in master mode, ACK not received
* 38h    Arbitration lost in address or data byte
* 40h    Address + Read bit transmitted, ACK received
* 48h    Address + Read bit transmitted, ACK not received
* 50h    Data byte received in master mode, ACK transmitted
* 58h    Data byte received in master mode, not ACK transmitted
* 60h    Slave address + Write bit received, ACK transmitted
* 68h    Arbitration lost in address as master, slave address + Write bit received, ACK transmitted
* 70h    General Call address received, ACK transmitted
* 78h    Arbitration lost in address as master, General Call address received, ACK transmitted
* 80h    Data byte received after slave address received, ACK transmitted
* 88h    Data byte received after slave address received, not ACK transmitted
* 90h    Data byte received after General Call received, ACK transmitted
* 98h    Data byte received after General Call received, not ACK transmitted
* A0h    STOP or repeated START condition received in slave mode
* A8h    Slave address + Read bit received, ACK transmitted
* B0h    Arbitration lost in address as master, slave address + Read bit received, ACK transmitted
* B8h    Data byte transmitted in slave mode, ACK received
* C0h    Data byte transmitted in slave mode, ACK not received
* C8h    Last byte transmitted in slave mode, ACK received
* D0h    Second Address byte + Write bit transmitted, ACK received
* D8h    Second Address byte + Write bit transmitted, ACK not received
* F8h    No relevant status information or no interrupt
*-----------------------------------------------------------------------------*/

/*
**********************************************************************************************************************
*                                               sw_iic_exit
*
* Description:  ͨ��IIC��������ȡIIC�豸һ���ֽڣ���ʱֻ֧�ֱ�׼���豸
*
* Arguments  :
*
* Returns    :   ��ȡ�ɹ�����0�����򷵻� -1
*
* Notes      :    none
*
**********************************************************************************************************************
*/
static __s32 i2c_sendstart(uint twi_host)
{
    __s32  time = 0xfffff;
    __u32  tmp_val;

	struct sunxi_twi_reg *i2c = (struct sunxi_twi_reg *)twi_host;
    i2c->eft  = 0;
    i2c->srst = 1;
    i2c->ctl |= 0x20;

    while((time--)&&(!(i2c->ctl & 0x08)));
	if(time <= 0)
	{
		return -I2C_NOK_TOUT;
	}

	tmp_val = i2c->status;
    if(tmp_val != I2C_START_TRANSMIT)
    {
		return -I2C_START_TRANSMIT;
    }

    return I2C_OK;
}
/*
**********************************************************************************************************************
*                                               TWIC_SendReStart
*
* Description:
*
* Arguments  :
*
* Returns    :
*
* Notes      :
*
**********************************************************************************************************************
*/
static __s32 i2c_sendRestart(uint twi_host)
{
    __s32  time = 0xffff;
    __u32  tmp_val;

	struct sunxi_twi_reg *i2c = (struct sunxi_twi_reg *)twi_host;
#if defined(CONFIG_ARCH_SUN7I) | defined(CONFIG_ARCH_SUN5I)
	tmp_val = i2c->ctl & 0xC0;
#else
	tmp_val = i2c->ctl;
#endif
	tmp_val |= 0x20;
	i2c->ctl = tmp_val;

    while( (time--) && (!(i2c->ctl & 0x08)) );
	if(time <= 0)
	{
		return -I2C_NOK_TOUT;
	}

	tmp_val = i2c->status;
    if(tmp_val != I2C_RESTART_TRANSMIT)
    {
		return -I2C_RESTART_TRANSMIT;
    }

    return I2C_OK;
}
/*
**********************************************************************************************************************
*                                               TWIC_SendSlaveAddr
*
* Description:
*
* Arguments  :
*
* Returns    :    EPDK_OK = successed;   EPDK_FAIL = failed
*
* Notes      :     none
*
**********************************************************************************************************************
*/
static __s32 i2c_sendslaveaddr(uint twi_host, __u32 saddr,  __u32 rw)
{
    __s32  time = 0xffff;
    __u32  tmp_val;
	struct sunxi_twi_reg *i2c = (struct sunxi_twi_reg *)twi_host;
	rw &= 1;
	i2c->data = ((saddr & 0xff) << 1)| rw;
#if defined(CONFIG_ARCH_SUN5I) || defined(CONFIG_ARCH_SUN7I)
    i2c->ctl &=  0xF7; //write 0 to clean int flag
#else
    i2c->ctl  |= (0x01<<3);//write 1 to clean int flag
#endif

	while(( time-- ) && (!( i2c->ctl & 0x08 )));
	if(time <= 0)
	{
		return -I2C_NOK_TOUT;
	}

	tmp_val = i2c->status;
	if(rw == I2C_WRITE)//+write
	{
		if(tmp_val != I2C_ADDRWRITE_ACK)
		{
			return -I2C_ADDRWRITE_ACK;
		}
	}
	else//+read
	{
		if(tmp_val != I2C_ADDRREAD_ACK)
		{
			return -I2C_ADDRREAD_ACK;
		}
	}

	return I2C_OK;
}
/*
**********************************************************************************************************************
*                                               i2c_SendByteAddr
*
* Description:
*
* Arguments  :
*
* Returns    :    EPDK_OK = successed;   EPDK_FAIL = failed
*
* Notes      :     none
*
**********************************************************************************************************************
*/
static __s32 i2c_sendbyteaddr(uint twi_host, __u32 byteaddr)
{
    __s32  time = 0xffff;
    __u32  tmp_val;

	struct sunxi_twi_reg *i2c = (struct sunxi_twi_reg *)twi_host;
	i2c->data = byteaddr & 0xff;
#if defined(CONFIG_ARCH_SUN5I)||defined(CONFIG_ARCH_SUN7I)
	i2c->ctl &= 0xF7;   //write 0 to clean int flag
#else
    i2c->ctl |= (0x01<<3);//write 1 to clean int flag
#endif
    while( (time--) && (!(i2c->ctl & 0x08)) );

	if(time <= 0)
	{
		return -I2C_NOK_TOUT;
	}

	tmp_val = i2c->status;
	if(tmp_val != I2C_DATAWRITE_ACK)
	{
		return -I2C_DATAWRITE_ACK;
	}

	return I2C_OK;
}
/*
**********************************************************************************************************************
*                                               TWIC_GetData
*
* Description:
*
* Arguments  :
*
* Returns    :    EPDK_OK = successed;   EPDK_FAIL = failed
*
* Notes      :     none
*
**********************************************************************************************************************
*/
static __s32 i2c_getdata(uint twi_host, __u8 *data_addr, __u32 data_count)
{
    __s32  time = 0xffff;
    __u32  tmp_val;
    __u32  i;
    struct sunxi_twi_reg *i2c = (struct sunxi_twi_reg *)twi_host;

	if(data_count == 1)
    {
#if defined(CONFIG_ARCH_SUN5I)||defined(CONFIG_ARCH_SUN7I)
		i2c->ctl &= 0xF7;
#else
		i2c->ctl |= (0x01<<3);
#endif
	    while( (time--) && (!(i2c->ctl & 0x08)) );

		if(time <= 0)
		{
	            return -I2C_NOK_TOUT;
		}
		for(time=0;time<100;time++);
		*data_addr = i2c->data;

		tmp_val = i2c->status;
		if(tmp_val != I2C_DATAREAD_NACK)
		{
			return -I2C_DATAREAD_NACK;
		}
	}
	else
	{
		for(i=0; i< data_count - 1; i++)
		{
			time = 0xffff;
			tmp_val = i2c->ctl | (0x01<<2);//���Ͷ�����ݵ�ʱ��ÿ�δ�һ�����ݣ�������Ҫ��һ��ack
#if defined(CONFIG_ARCH_SUN5I)||defined(CONFIG_ARCH_SUN7I)
			tmp_val = i2c->ctl & (0xf7);
#else
			tmp_val = i2c->ctl | (0x01<<3);
#endif
			tmp_val |= 0x04;
            i2c->ctl = tmp_val;
		 	//i2c->ctl |=(0x01<<3);

			while( (time--) && (!(i2c->ctl & 0x08)) );
			if(time <= 0)
			{
				return -I2C_NOK_TOUT;
			}
			for(time=0;time<100;time++);
			time = 0xffff;
			data_addr[i] = i2c->data;

			while( (time--) && (i2c->status != I2C_DATAREAD_ACK) );

			if(time <= 0)
			{
				return -I2C_NOK_TOUT;
			}
		}

        time = 0xffff;
        i2c->ctl &= 0xFb;  //�����һ�����ݵ�ʱ�򣬾ͽ���λ���㣬�������ӻ�ack��Ĭ�ϴ������
#if defined(CONFIG_ARCH_SUN5I)||defined(CONFIG_ARCH_SUN7I)
        i2c->ctl &= 0xf7;
#else
        i2c->ctl |= (0x01<<3);
#endif
		while( (time--) && (!(i2c->ctl & 0x08)) );

		if(time <= 0)
		{
			return -I2C_NOK_TOUT;
		}
		for(time=0;time<100;time++);
		data_addr[data_count - 1] = i2c->data;
	    while( (time--) && (i2c->status != I2C_DATAREAD_NACK) );
		if(time <= 0)
		{
			return -I2C_NOK_TOUT;
		}
	}

	return I2C_OK;
}
/*
**********************************************************************************************************************
*                                               i2c_SendData
*
* Description:
*
* Arguments  :
*
* Returns    :    EPDK_OK = successed;   EPDK_FAIL = failed
*
* Notes      :     none
*
**********************************************************************************************************************
*/
static __s32 i2c_senddata(uint twi_host, __u8  *data_addr, __u32 data_count)
{
    __s32  time = 0xffff;
    __u32  i;

	struct sunxi_twi_reg *i2c = (struct sunxi_twi_reg *)twi_host;
	for(i=0; i< data_count; i++)
	{
		time = 0xffff;
	    i2c->data = data_addr[i];
#if defined(CONFIG_ARCH_SUN5I)|defined(CONFIG_ARCH_SUN7I)
		i2c->ctl &= 0xF7;
#else
        i2c->ctl |= (0x01<<3);
#endif
        while( (time--) && (!(i2c->ctl & 0x08)) );
		if(time <= 0)
		{
			return -I2C_NOK_TOUT;
		}
		time = 0xffff;
		while( (time--) && (i2c->status != I2C_DATAWRITE_ACK) );
        if(time <= 0)
		{
			return -I2C_NOK_TOUT;
		}
	}

	return I2C_OK;
}

/*
**********************************************************************************************************************
*                                               i2c_Stop
*
* Description:
*
* Arguments  :
*
* Returns    :    EPDK_OK = successed;   EPDK_FAIL = failed
*
* Notes      :     none
*
**********************************************************************************************************************
*/
static __s32 busid_to_addr(uint bus_id, uint *twi_host)
{
	if(bus_id < SUNXI_TWI_COUNT)		//cpux i2c
	{
		*twi_host = SUNXI_TWI0_BASE + SUNXI_TWI_OFFSET * bus_id;
		return 1;
	}
	else if((bus_id & 0x80000000) && (!(bus_id & 0x01))) //cpus i2c
	{
#if defined(CONFIG_CPUS_I2C)
		*twi_host = SUNXI_CPUS_TWI_BASE;
		return 2;
#endif
	}

	printf("i2c bus id %d is error\n", bus_id);
	return -1;
}

/*
**********************************************************************************************************************
*                                               i2c_Stop
*
* Description:
*
* Arguments  :
*
* Returns    :    EPDK_OK = successed;   EPDK_FAIL = failed
*
* Notes      :     none
*
**********************************************************************************************************************
*/
static void set_cpux_i2c_clock(uint bus_id)
{
	int reg_value = 0;
	bus_id = bus_id;
#if defined(CONFIG_ARCH_SUN9IW1P1)
    //reset apb1 twi[bus_id]
    reg_value = *((unsigned int *)CCM_APB1_RST_REG0);
    reg_value |= (0x01 << bus_id);
    *((unsigned int *)CCM_APB1_RST_REG0) = reg_value;
	__msdelay(1);

	//set apb1 twi[bus_id] gating
	reg_value = *((unsigned int *)CCM_APB1_GATE0_CTRL);
    reg_value |= (0x01 << bus_id);
    *((unsigned int *)CCM_APB1_GATE0_CTRL) = reg_value;
#elif defined(CONFIG_ARCH_SUN8IW6P1)
    reg_value = *((unsigned int *)CCMU_BUS_SOFT_RST_REG4);
    reg_value |= (0x01 << bus_id);
    *((unsigned int *)CCMU_BUS_SOFT_RST_REG4) = reg_value;
	__msdelay(1);

	reg_value = *((unsigned int *)CCMU_BUS_CLK_GATING_REG3);
	reg_value |= (0x01 << bus_id);
	*((unsigned int *)CCMU_BUS_CLK_GATING_REG3) = reg_value ;
#else
	struct sunxi_ccm_reg *ccm_reg = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
#if !defined(CONFIG_ARCH_SUN5I)||!defined(CONFIG_ARCH_SUN7I)
	/* reset i2c clock    */
	/* reset apb2 twi0*/
	reg_value = *((unsigned int *)0x01c202d8);
	reg_value |= 0x01;
	*((unsigned int *)0x01c202d8) = reg_value;
	__msdelay(1);
#endif
	ccm_reg->apb1_gate &= ~1;
	__msdelay(1);
	ccm_reg->apb1_gate |=  1;
#endif
	return;
}
/*
**********************************************************************************************************************
*                                               i2c_Stop
*
* Description:
*
* Arguments  :
*
* Returns    :    EPDK_OK = successed;   EPDK_FAIL = failed
*
* Notes      :     none
*
**********************************************************************************************************************
*/
#if defined(CONFIG_CPUS_I2C)
static void set_cpus_i2c_clock(uint bus_id)
{
    int reg_value = 0;
    bus_id = bus_id;
    reg_value = *((unsigned int *)(R_PRCE_APB0_RESET));
    reg_value |= 0x01 << 6;
    *((unsigned int *)(R_PRCE_APB0_RESET)) = reg_value;
	__msdelay(1);
	reg_value = *((unsigned int *)(R_PRCM_APB0_GATING));
    reg_value |= 0x01 << 6;
    *((unsigned int *)(R_PRCM_APB0_GATING)) = reg_value;
	__msdelay(1);
	return;
}
#endif
/*
**********************************************************************************************************************
*                                               i2c_Stop
*
* Description:
*
* Arguments  :
*
* Returns    :    EPDK_OK = successed;   EPDK_FAIL = failed
*
* Notes      :     none
*
**********************************************************************************************************************
*/
static __s32 i2c_stop(uint twi_host)
{
    __s32  time = 0xffff;
    __u32  tmp_val;

	struct sunxi_twi_reg *i2c = (struct sunxi_twi_reg *)twi_host;

	i2c->ctl |= (0x01 << 4);
#if defined(CONFIG_ARCH_SUN5I)|defined(CONFIG_ARCH_SUN7I)
    i2c->ctl &= 0xf7;
#else
    i2c->ctl |= (0x01 << 3);
#endif
    while( (time--) && (i2c->ctl & 0x10) );

	if(time <= 0)
	{
		return -I2C_NOK_TOUT;
	}
	time = 0xffff;
	while( (time--) && (i2c->status != I2C_READY) );
	tmp_val = i2c->status;
	if(tmp_val != I2C_READY)
	{
		return -I2C_NOK_TOUT;
	}

	return I2C_OK;
}



/*
**********************************************************************************************************************
*                                               i2c_init
*
* Description:
*
* Arguments  :
*
* Returns    :    none
*
* Notes      :    none
*
**********************************************************************************************************************
*/
void i2c_init(uint bus_id, int speed, int slaveaddr)
{
	int i, ret, clk_n, clk_m;
	char twi_para[16] = {0};
	uint twi_host = 0;
    struct sunxi_twi_reg *i2c = NULL;

	ret = busid_to_addr(bus_id, &twi_host);		//cpux i2c
	if(ret == 1)
	{
		sprintf(twi_para, "twi%d", bus_id);
		set_cpux_i2c_clock(bus_id);
	}
	else if(ret == 2) //cpus i2c
	{
#if defined(CONFIG_CPUS_I2C)
#ifdef CONFIG_ARCH_SUN8IW7P1
		strcpy(twi_para, "s_rsb0");
#else
		strcpy(twi_para, "s_twi0");
#endif
		set_cpus_i2c_clock(bus_id);
#else
		printf("not define CONFIG_CPUS_I2C\n");
		return;
#endif
	}
	else
	{
		printf("i2c bus id error %d\n", ret);
		return ;
	}

    i2c = (struct sunxi_twi_reg *)twi_host;

	/* set i2c gpio */
    //gpio_request_early((void *)uboot_spare_head.boot_data.twi_gpio, 2, 1);
    gpio_request_simple(twi_para, NULL);
	/* reset i2c control  */
    i = 0xffff;
    i2c->srst = 1;
    while((i2c->srst) && (i))
    {
    	i --;
    }
    if((i2c->lcr & 0x30) != 0x30 )
    {
    	/* toggle I2CSCL until bus idle */
    	i2c->lcr = 0x05;
    	__usdelay(500);
    	i = 10;
		while ((i > 0) && ((i2c->lcr & 0x02) != 2))
		{
			i2c->lcr |= 0x08;
			__usdelay(1000);
			i2c->lcr &= ~0x08;
			__usdelay(1000);
			i--;
		}
		i2c->lcr = 0x0;
		__usdelay(500);
    }

	if(speed < 100)
	{
		speed = 100;
	}
	else if(speed > 400)
	{
		speed = 400;
	}
#if (defined(CONFIG_ARCH_SUN9IW1P1) && defined(CONFIG_ARCH_HOMELET))
	clk_n = 2;
	clk_m = (64000/10)/((2^clk_n) * speed) - 1;
#else
	clk_n = 1;
	clk_m = (24000/10)/((2^clk_n) * speed) - 1;
#endif
    i2c->clk = (clk_m<<3) | clk_n;
    i2c->ctl = 0x40;
    i2c->eft = 0;
    return ;
}




/*
**********************************************************************************************************************
*                                               i2c_init
*
* Description:
*
* Arguments  :
*
* Returns    :    none
*
* Notes      :    none
*
**********************************************************************************************************************
*/
void i2c_exit(uint bus_id)
{
	int ret;
	uint twi_host = 0;
	uint reg_value = 0;
	reg_value = reg_value;
	ret = busid_to_addr(bus_id, &twi_host);
	if(ret == 1) //cpux i2c
	{
#if defined(CONFIG_ARCH_SUN9IW1P1)
		reg_value = *((unsigned int *)CCM_APB1_GATE0_CTRL);
		reg_value &= ~(0x01 << bus_id);
		*((unsigned int *)CCM_APB1_GATE0_CTRL) = reg_value ;
#else
#if defined(CONFIG_ARCH_SUN8IW6P1)
		reg_value = *((unsigned int *)CCMU_BUS_CLK_GATING_REG3);
		reg_value &= ~(0x01 << bus_id);
		*((unsigned int *)CCMU_BUS_CLK_GATING_REG3) = reg_value ;
#else
		struct sunxi_ccm_reg *ccm_reg = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
		/* close i2c clock    */
		ccm_reg->apb1_gate &= ~1;
#endif
#endif
	}
	else if(ret == 2) //cpus i2c stwi0
	{
#if defined(CONFIG_CPUS_I2C)
	    reg_value = *((unsigned int *)(R_PRCE_APB0_RESET));
	    reg_value &= ~(0x01 << 6);
	    *((unsigned int *)(R_PRCE_APB0_RESET)) = reg_value;
	    __msdelay(1);
	    reg_value = *((unsigned int *)(R_PRCM_APB0_GATING));
	    reg_value &= ~(0x01 << 6);
	    *((unsigned int *)(R_PRCM_APB0_GATING)) = reg_value;
#else
		printf("not define CONFIG_CPUS_I2C\n");
#endif
	}
	else
	{
		printf("i2c bus id error %d\n", ret);
		return ;
	}
	__msdelay(1);
	return ;
}
/*
****************************************************************************************************
*
*                                       i2c_read
*
*  Description:
*
*
*  Parameters:
*
*  Return value:
*
*  Read/Write interface:
*	 bus_id:  the id of the twi bus: 4,3,2,1,0 is to twi4..twi0
*    chip:    I2C slave chip address, range 0..127
*    addr:    Memory (register) address within the chip
*    alen:    Number of bytes to use for addr (
*             0, 1: addr len = 8bit
*			  2: addr len = 16bit
*			  3, 4: addr len = 32bit
*
*    buffer:  Where to read/write the data
*    len:     How many bytes to read/write
*
*    Returns: 0 on success, not 0 on failure
*
****************************************************************************************************
*/
int i2c_read(uint bus_id, uchar chip, uint addr, int alen, uchar *buffer, int len)
{
    int   i, ret, ret0, addrlen;
    char  *slave_reg;
	uint twi_host = 0;

	ret0 = -1;
	ret = busid_to_addr(bus_id, &twi_host);
	if(ret < 0)
	{
		return ret0;
	}

    ret = i2c_sendstart(twi_host);
    if(ret)
	{
		goto i2c_read_err_occur;
	}

    ret = i2c_sendslaveaddr(twi_host, chip, I2C_WRITE);
    if(ret)
    {
	    goto i2c_read_err_occur;
	}
    //send byte address
    if(alen >= 3)
    {
		addrlen = 2;
    }
    else if(alen <= 1)
    {
		addrlen = 0;
    }
    else
    {
		addrlen = 1;
    }
    slave_reg = (char *)&addr;

	for (i = addrlen; i>=0; i--)
    {
		ret = i2c_sendbyteaddr(twi_host, slave_reg[i] & 0xff);
    	if(ret)
    	{
    		goto i2c_read_err_occur;
    	}
    }
    ret = i2c_sendRestart(twi_host);
    if(ret)
	{
		goto i2c_read_err_occur;
	}
    ret = i2c_sendslaveaddr(twi_host, chip, I2C_READ);
    if(ret)
    {
        goto i2c_read_err_occur;
	}
    //get data
	ret = i2c_getdata(twi_host, buffer, len);
	if(ret)
	{
		goto i2c_read_err_occur;
	}
    ret0 = 0;

i2c_read_err_occur:
	i2c_stop(twi_host);

	return ret0;
}
/*
****************************************************************************************************
*
*             TWIC_Write
*
*  Description:
*       DRV_MOpen
*
*  Parameters:
*
*  Return value:
*       EPDK_OK
*       EPDK_FAIL
****************************************************************************************************
*/
int i2c_write(uint bus_id, uchar chip, uint addr, int alen, uchar *buffer, int len)
{
    int   i, ret, ret0, addrlen;
    char  *slave_reg;
    uint twi_host = 0;

    ret0 = -1;
	ret = busid_to_addr(bus_id, &twi_host);
	if(ret < 0)
	{
		return ret0;
	}
    ret = i2c_sendstart(twi_host);
    if(ret)
	{
		goto i2c_write_err_occur;
	}

    ret = i2c_sendslaveaddr(twi_host, chip, I2C_WRITE);
    if(ret)
    {
	    goto i2c_write_err_occur;
	}
    //send byte address
    if(alen >= 3)
    {
        addrlen = 2;
    }
    else if(alen <= 1)
    {
        addrlen = 0;
    }
    else
    {
		addrlen = 1;
    }

	slave_reg = (char *)&addr;
    for (i = addrlen; i>=0; i--)
    {
		ret = i2c_sendbyteaddr(twi_host, slave_reg[i] & 0xff);
    	if(ret)
    	{
    		goto i2c_write_err_occur;
    	}
    }

	ret = i2c_senddata(twi_host, buffer, len);
	if(ret)
	{
		goto i2c_write_err_occur;
	}
    ret0 = 0;

i2c_write_err_occur:
	i2c_stop(twi_host);

	return ret0;
}

