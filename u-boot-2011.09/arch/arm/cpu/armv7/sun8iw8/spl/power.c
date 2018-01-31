/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Ming <liaoyongming@allwinnertech.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "common.h"
#include <power/axp20_reg.h>
#include <power/axp15_reg.h>
#include <i2c.h>

#define AXP_TWI_ID			(0)

static int axp20_probe(void)
{
	u8    pmu_type;
  //__msdelay(100);//ȷ��Ӳ��ADC׼���á�
	if(i2c_read(AXP_TWI_ID, AXP20_ADDR, BOOT_POWER20_VERSION, 1, &pmu_type, 1))
	{
		printf("axp209 read error\n");
		return -1;
	}
	pmu_type &= 0x0f;
	if(pmu_type & 0x01)
	{
		/* pmu type AXP209 */
		printf("PMU: AXP209\n");
		return 0;
	}

	return -1;	
}
	
static int axp20_set_dcdc2(int set_vol, int onoff)
{
    u32 vol, tmp;
	volatile u32 i;
	u8  reg_addr, value;
	if(set_vol == -1)
	{
		set_vol = 1400;
	}

	//PMU is AXP209
	reg_addr = BOOT_POWER20_DC2OUT_VOL;
	if(i2c_read(AXP_TWI_ID, AXP20_ADDR, reg_addr, 1, &value, 1))
	{
		return -1;
	}
	tmp     = value & 0x3f;
	vol     = tmp * 25 + 700;
	//�����ѹ���ߣ������
	while(vol > set_vol)
	{
		tmp -= 1;
		value &= ~0x3f;
		value |= tmp;
		reg_addr = BOOT_POWER20_DC2OUT_VOL;
		if(i2c_write(AXP_TWI_ID, AXP20_ADDR, reg_addr, 1, &value, 1))
		{
			return -1;
		}
		for(i=0;i<2000;i++);
		reg_addr = BOOT_POWER20_DC2OUT_VOL;
		if(i2c_read(AXP_TWI_ID, AXP20_ADDR, reg_addr, 1, &value, 1))
		{
			return -1;
		}
		tmp     = value & 0x3f;
		vol     = tmp * 25 + 700;
    }
	//�����ѹ���ͣ�����ߣ������ȵ����ٵ��ߵĹ��̣���֤��ѹ����ڵ����û��趨��ѹ+
	while(vol < set_vol)
	{
		tmp += 1;
		value &= ~0x3f;
		value |= tmp;
		reg_addr = BOOT_POWER20_DC2OUT_VOL;
		if(i2c_write(AXP_TWI_ID, AXP20_ADDR, reg_addr, 1, &value, 1))
		{
			return -1;
		}
		for(i=0;i<2000;i++);
		reg_addr = BOOT_POWER20_DC2OUT_VOL;
		if(i2c_read(AXP_TWI_ID, AXP20_ADDR, reg_addr, 1, &value, 1))
		{
			return -1;
		}
		tmp     = value & 0x3f;
		vol     = tmp * 25 + 700;
	}
	printf("after set, dcdc2 =%dmv\n",vol);

	return 0;
}

int axp15_probe(void)
{
	u8    pmu_type;
	//__msdelay(100);//ȷ��Ӳ��ADC׼���á�
	if(i2c_read(AXP_TWI_ID, AXP15_ADDR,BOOT_POWER15_VERSION, 1, &pmu_type, 1))
	{
		printf("axp152 read error\n");
		return -1;
	}
	//printf("pmu_type = %x\n",pmu_type);
	pmu_type &= 0x0f;
	if(pmu_type == 0x05)
	{
		/* pmu type AXP152 */
		printf("PMU: AXP152\n");
		return 0;
	}
	return -1;
}

int axp15_set_dcdc2(int set_vol,int onoff)
{
    u8  reg_value;
	u32 	vol;
	u8  tmp;
	u32 i = 0 ;
    if(set_vol > 0)
    {
        if(set_vol <700)
        {
            set_vol = 700;
        }
        else if(set_vol >2275)
        {
            set_vol = 2275;
        }
        if(i2c_read(AXP_TWI_ID, AXP15_ADDR,BOOT_POWER15_DC2OUT_VOL,1, &reg_value, 1))
        {
        	printf("can't read dcdc2_vol!!!\n");
        	return -1;
        }
		tmp = reg_value & 0x3f;
		vol = tmp*25+700;
		printf("the vol is %d\n",vol);
		while(vol>set_vol)
		{
			tmp -= 1;
	        reg_value &= ~0x3f;
	        reg_value |= tmp;
	        if(i2c_write(AXP_TWI_ID, AXP15_ADDR, BOOT_POWER15_DC2OUT_VOL, 1, &reg_value, 1))
	        {
	            return -1;
	        }
	        for(i=0;i<2000;i++);
	        if(i2c_read(AXP_TWI_ID, AXP15_ADDR, BOOT_POWER15_DC2OUT_VOL, 1, &reg_value, 1))
	        {
	            return -1;
	        }
	        tmp = reg_value & 0x3f;
	        vol = tmp * 25 + 700;
			printf("the vol is %d\n",vol);
		}
		while(vol<set_vol)
		{
			tmp += 1;
	        reg_value &= ~0x3f;
	        reg_value |= tmp;
	        if(i2c_write(AXP_TWI_ID, AXP15_ADDR, BOOT_POWER15_DC2OUT_VOL, 1, &reg_value, 1))
	        {
	            return -1;
	        }
	        for(i=0;i<2000;i++);
	        if(i2c_read(AXP_TWI_ID, AXP15_ADDR, BOOT_POWER15_DC2OUT_VOL, 1,&reg_value, 1))
	        {
	            return -1;
	        }
	        tmp = reg_value & 0x3f;
	        vol = tmp * 25 + 700;
			printf("the vol is %d\n",vol);
		}
    }
	if(onoff<0)
	{
		return 0;
	}
	if(i2c_read(AXP_TWI_ID, AXP15_ADDR,BOOT_POWER15_OUTPUT_CTL,1,&reg_value,1))
	{
		printf("sunxi pmu error : unable to onoff dcdc2\n");
		return -1;
	}
	if(onoff == 0)
	{
		reg_value &= ~(1<<6);
	}
	else
	{
		reg_value |= (1<<6);
	}
	if(i2c_write(AXP_TWI_ID, AXP15_ADDR,BOOT_POWER15_OUTPUT_CTL,1,&reg_value,1))
	{
		printf("sunxi pmu error : unable to onoff dcdc2\n");
		return -1;
	}

	if(i2c_read(AXP_TWI_ID, AXP15_ADDR,BOOT_POWER15_DC2OUT_VOL,1,&reg_value,1))
	{
		printf("can't read output_ctl_reg!!\n");
		return -1;
	}
	return 0;
}

int pmu_set_vol(int set_vol, int onoff)
{
	i2c_init(AXP_TWI_ID, CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	if (!axp20_probe())
	{
		//set sys vol +
		if(axp20_set_dcdc2(set_vol, onoff))
		{
				printf("axp20 set dcdc2 vol fail, maybe no pmu \n");
				return -1;
		}
		printf("axp20 set dcdc2 success \n");
	}
	else if (!axp15_probe())
	{
		if(axp15_set_dcdc2(set_vol, onoff))
		{
				printf("axp15 set dcdc2 vol fail, maybe no pmu \n");
				return -1;
		}
		printf("axp15 set dcdc2 success \n");	
	}
	else
	{
		return -1;	
	}
	return 0;
}
