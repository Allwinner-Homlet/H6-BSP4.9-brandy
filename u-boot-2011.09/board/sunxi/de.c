/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
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
#include <common.h>
#include <malloc.h>
#if defined(CONFIG_VIDEO_SUNXI_V3)
#include <asm/arch/sunxi_display2.h>
#elif defined(CONFIG_VIDEO_SUNXI_V2) || defined(CONFIG_VIDEO_SUNXI_V1)
#include <asm/arch/drv_display.h>
#else
#endif
#include <sys_config.h>
#include <bmp_layout.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SUNXI_DISPLAY
static __u32 screen_id = 0;
static __u32 disp_para = 0;
static __u32 init_disp = 0;
static __u32 saved_tv_vdid = 0;
static __u32 ban_lcd_bl = 0;

int get_display_resolution(int display_type);
int get_saved_hdmi_vendor_id(void);
int get_hdmi_vendor_id(int sel);
int reset_hdmi_mode(int sel,int mode);
int get_disp_ban_bl(int sel);

#if (defined CONFIG_VIDEO_SUNXI_V1)
extern __s32 disp_delay_ms(__u32 ms);
#define DELAY_MS(x) disp_delay_ms(x)
#define MY_GET_HDMI_HPD_STATUS DISP_CMD_HDMI_GET_HPD_STATUS
#define MY_GET_CVBS_HPD_STATUS 0
#define MY_HDMI_SUPPORT_MODE DISP_CMD_HDMI_SUPPORT_MODE
#define MY_HDMI_GET_EDID 0

#elif (defined CONFIG_VIDEO_SUNXI_V2)
extern __s32 bsp_disp_delay_ms(u32 ms);
#define DELAY_MS(x) bsp_disp_delay_ms(x)
#define MY_GET_HDMI_HPD_STATUS DISP_CMD_HDMI_GET_HPD_STATUS
#define MY_GET_CVBS_HPD_STATUS 0
#define MY_HDMI_SUPPORT_MODE DISP_CMD_HDMI_SUPPORT_MODE
#define MY_HDMI_GET_EDID 0

#else //(defined CONFIG_VIDEO_SUNXI_V3)
extern s32 disp_delay_ms(u32 ms);
#define DELAY_MS(x) disp_delay_ms(x)
#define MY_GET_HDMI_HPD_STATUS DISP_HDMI_GET_HPD_STATUS
#define MY_GET_CVBS_HPD_STATUS DISP_TV_GET_HPD_STATUS
#define MY_HDMI_SUPPORT_MODE DISP_HDMI_SUPPORT_MODE
#define MY_HDMI_GET_EDID DISP_HDMI_GET_EDID

#endif

int board_display_layer_request(void)
{
#if defined(CONFIG_VIDEO_SUNXI_V1)
	__u32 arg[4];

	arg[0] = screen_id;
	arg[1] = DISP_LAYER_WORK_MODE_NORMAL;

	gd->layer_hd = disp_ioctl(NULL, DISP_CMD_LAYER_REQUEST, (void*)arg);
	if(gd->layer_hd == 0)
	{
        tick_printf("sunxi display error : display request layer failed\n");

        return -1;
	}

	return 0;
#elif defined(CONFIG_VIDEO_SUNXI_V2)
	gd->layer_hd = 0;
	return 0;
#else
	gd->layer_hd = 0;
	return 0;
#endif
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_layer_release(void)
{
#if defined(CONFIG_VIDEO_SUNXI_V1)
	__u32 arg[4];

	if(gd->layer_hd == 0)
	{
        tick_printf("sunxi display error : display layer is NULL\n");

        return -1;
	}

	arg[0] = screen_id;
	arg[1] = gd->layer_hd;

	return disp_ioctl(NULL, DISP_CMD_LAYER_RELEASE, (void*)arg);
#elif defined(CONFIG_VIDEO_SUNXI_V2)
	return 0;
#else
	return 0;
#endif
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_wait_lcd_open(void)
{
	int ret;
	int timedly = 5000;
	int check_time = timedly/50;
	uint arg[4] = { 0 };
	uint cmd = 0;

#if defined(CONFIG_VIDEO_SUNXI_V3)
	cmd = DISP_LCD_CHECK_OPEN_FINISH;
#else
	cmd = DISP_CMD_LCD_CHECK_OPEN_FINISH;
#endif

	do
	{
    	ret = disp_ioctl(NULL, cmd, (void*)arg);
		if(ret == 1)		//open already
		{
			break;
		}
		else if(ret == -1)  //open falied
		{
			return -1;
		}
		__msdelay(50);
		check_time --;
		if(check_time <= 0)
		{
			return -1;
		}
	}
	while(1);

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_wait_lcd_close(void)
{
	int ret;
	int timedly = 5000;
	int check_time = timedly/50;
	uint arg[4] = { 0 };
	uint cmd = 0;

#if defined(CONFIG_VIDEO_SUNXI_V3)
	cmd = DISP_LCD_CHECK_CLOSE_FINISH;
#else
	cmd = DISP_CMD_LCD_CHECK_CLOSE_FINISH;
#endif
	do
	{
    	ret = disp_ioctl(NULL, cmd, (void*)arg);
		if(ret == 1)		//open already
		{
			break;
		}
		else if(ret == -1)  //open falied
		{
			return -1;
		}
		__msdelay(50);
		check_time --;
		if(check_time <= 0)
		{
			return -1;
		}
	}
	while(1);

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_set_exit_mode(int lcd_off_only)
{
	uint arg[4] = { 0 };
	uint cmd = 0;

#if defined(CONFIG_VIDEO_SUNXI_V3)
	cmd = DISP_SET_EXIT_MODE;
#else
	cmd = DISP_CMD_SET_EXIT_MODE;
#endif
	if(lcd_off_only)
	{
		arg[0] = DISP_EXIT_MODE_CLEAN_PARTLY;
		disp_ioctl(NULL, cmd, (void *)arg);
	}
	else
	{
#if defined(CONFIG_VIDEO_SUNXI_V3)
	cmd = DISP_LCD_DISABLE;
#elif defined(CONFIG_VIDEO_SUNXI_V2)
	cmd = DISP_CMD_LCD_DISABLE;
#else
	cmd = DISP_CMD_LCD_OFF;
#endif

		disp_ioctl(NULL, cmd, (void *)arg);
		board_display_wait_lcd_close();
	}

	return 0;
}
/*
*******************************************************************************
*                     board_display_layer_open
*
* Description:
*    ��ͼ��
*
* Parameters:
*    Layer_hd    :  input. ͼ����
*
* Return value:
*    0  :  �ɹ�
*   !0  :  ʧ��
*
* note:
*    void
*
*******************************************************************************
*/
int board_display_layer_open(void)
{
    uint arg[4];

	arg[0] = screen_id;
	arg[1] = gd->layer_hd;
	arg[2] = 0;
	arg[3] = 0;

#if defined(CONFIG_VIDEO_SUNXI_V3)
	{
		disp_layer_config *config = (disp_layer_config *)gd->layer_para;

		arg[1] = (unsigned long)config;
		arg[2] = 1;
		disp_ioctl(NULL,DISP_LAYER_GET_CONFIG,(void*)arg);
		config->enable = 1;
		disp_ioctl(NULL,DISP_LAYER_SET_CONFIG,(void*)arg);
	}
#elif defined(CONFIG_VIDEO_SUNXI_V2)
	disp_ioctl(NULL,DISP_CMD_LAYER_ENABLE,(void*)arg);
#else
	disp_ioctl(NULL,DISP_CMD_LAYER_OPEN,(void*)arg);
#endif

    return 0;
}


/*
*******************************************************************************
*                     board_display_layer_close
*
* Description:
*    �ر�ͼ��
*
* Parameters:
*    Layer_hd    :  input. ͼ����
*
* Return value:
*    0  :  �ɹ�
*   !0  :  ʧ��
*
* note:
*    void
*
*******************************************************************************
*/
int board_display_layer_close(void)
{
    uint arg[4];

	arg[0] = screen_id;
	arg[1] = gd->layer_hd;
	arg[2] = 0;
	arg[3] = 0;

#if defined(CONFIG_VIDEO_SUNXI_V3)
	{
		disp_layer_config *config = (disp_layer_config *)gd->layer_para;

		arg[1] = (unsigned long)config;
		arg[2] = 1;
		disp_ioctl(NULL,DISP_LAYER_GET_CONFIG,(void*)arg);
		config->enable = 0;
		disp_ioctl(NULL,DISP_LAYER_SET_CONFIG,(void*)arg);
	}
#elif defined(CONFIG_VIDEO_SUNXI_V2)
	disp_ioctl(NULL,DISP_CMD_LAYER_DISABLE,(void*)arg);
#else
	disp_ioctl(NULL,DISP_CMD_LAYER_CLOSE,(void*)arg);
#endif

    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_layer_para_set(void)
{
    uint arg[4];

	arg[0] = screen_id;
	arg[1] = gd->layer_hd;
	arg[2] = gd->layer_para;
	arg[3] = 0;

#if defined(CONFIG_VIDEO_SUNXI_V3)
	arg[1] = gd->layer_para;
	arg[2] = 1;
	disp_ioctl(NULL,DISP_LAYER_SET_CONFIG,(void*)arg);
#elif defined(CONFIG_VIDEO_SUNXI_V2)
	disp_ioctl(NULL,DISP_CMD_LAYER_SET_INFO,(void*)arg);
#else
	disp_ioctl(NULL,DISP_CMD_LAYER_SET_PARA,(void*)arg);
#endif

    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_show_until_lcd_open(int display_source)
{
    if(!display_source)
    {
    	board_display_wait_lcd_open();
    }
    board_display_layer_para_set();
	board_display_layer_open();

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_show(int display_source)
{
    board_display_layer_para_set();
	board_display_layer_open();

	return 0;

}

#if ((defined CONFIG_ARCH_HOMELET) && (defined CONFIG_ARCH_SUN9IW1P1))
#define LOGOSIZE_WIDTH_DEFAULT 1280
#define LOGOSIZE_HEIGHT_DEFAULT 720
/*
************************************************************************************************************
*
*											  function
*
*	 name		   : board_display_get_virtual_logosize
*
*	 parmeters	   : __disp_rectsz_t *
*
*	 return 	   : int
*
*	 note		   :
*
*
************************************************************************************************************
*/
int board_display_get_virtual_logosize(disp_size *logosize)
{
	int value = 0;

	if((script_parser_fetch("boot_disp", "virtual_logo_width", &value, 1) == 0) && (value > 0))
	{
		logosize->width = value;
	}
	else
	{
		tick_printf("fetch script data boot_disp.virtual_logo_width fail or vale=%d\n", value);
		goto out;

	}
	value = 0;
	if((script_parser_fetch("boot_disp", "virtual_logo_height", &value, 1) == 0) && (value > 0))
	{
		logosize->height = value;
	}
	else
	{
		tick_printf("fetch script data boot_disp.virtual_logo_height fail or vale=%d\n", value);
		goto out;

	}

	return 0;

out:
	logosize->width = LOGOSIZE_WIDTH_DEFAULT;
	logosize->height = LOGOSIZE_HEIGHT_DEFAULT;

	return 0;
}
#undef LOGOSIZE_WIDTH_DEFAULT
#undef LOGOSIZE_HEIGHT_DEFAULT

/*
************************************************************************************************************
*
*											  function
*
*	 name		   : board_display_reset_scn_win
*
*	 parmeters	   : __disp_rect_t const *: the real bootlogo size;
                       uint const width, uint const height: the width and height of screen;
                       __disp_rect_t *scn_win: the result of reseting scn_win
*
*	 return 	   :
*
*	 note		   :
*
*
************************************************************************************************************
*/
int board_display_reset_scn_win(disp_window const *src_win,uint const width, uint const height, disp_window *scn_win)
{
	disp_size logosize;
	uint w,h;

	board_display_get_virtual_logosize(&logosize);
	if((src_win->width <= logosize.width) && (src_win->height <= logosize.height))
	{
		scn_win->x = (logosize.width - src_win->width) >> 1;
		scn_win->y = (logosize.height - src_win->height) >> 1;
		scn_win->width = src_win->width;
		scn_win->height = src_win->height;
	}
	else
	{
		w = src_win->width * logosize.height;
		h = src_win->height * logosize.width;
		if(w > h)
		{
			scn_win->width = logosize.width;
			scn_win->height = src_win->height * logosize.width / src_win->width;
			scn_win->x = 0;
			scn_win->y = (logosize.height - scn_win->height) >> 1;
		}
		else
		{
			scn_win->width = src_win->width * logosize.height / src_win->height;
			scn_win->height = logosize.height;
			scn_win->x = (logosize.width - scn_win->width) >> 1;
			scn_win->y = 0;
		}
	}
#if 0
	printf("scn_win[%d %d %d %d],[%d %d] logo[%d %d]\n",
	    scn_win->x, scn_win->y,
	    scn_win->width, scn_win->height,
	    width, height, logosize.width, logosize.height);
#endif
	scn_win->x = scn_win->x * width / logosize.width;
	scn_win->y = scn_win->y * height / logosize.height;
	scn_win->width = scn_win->width * width / logosize.width;
	scn_win->height = scn_win->height * height / logosize.height;

	return 0;
}
#endif

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/

int card_board_display_framebuffer_set(int width, int height, int bitcount, void *buffer,int count)
{
#if defined(CONFIG_VIDEO_SUNXI_V3)
	disp_layer_config *layer_para;
	uint screen_width, screen_height;
	uint arg[4];
	int full = 0;

	if(!gd->layer_para)
	{
		layer_para = (disp_layer_config *)malloc(sizeof(disp_layer_config));
		if(!layer_para)
		{
			tick_printf("sunxi display error: unable to malloc memory for layer\n");

			return -1;
		}
	}
	else
	{
		layer_para = (disp_layer_config *)gd->layer_para;
	}

	if(script_parser_fetch("boot_disp", "output_full", &full, 1) < 0)
	{
		//tick_printf("fetch script data boot_disp.output_disp fail\n");
		full = 0;
	}

	arg[0] = screen_id;
	screen_width = disp_ioctl(NULL, DISP_GET_SCN_WIDTH, (void*)arg);
	screen_height = disp_ioctl(NULL, DISP_GET_SCN_HEIGHT, (void*)arg);
	//tick_printf("screen_id =%d, screen_width =%d, screen_height =%d\n", screen_id, screen_width, screen_height);
	memset((void *)layer_para, 0, sizeof(disp_layer_config));
	layer_para->info.fb.addr[0]		= (uint)buffer;
	//tick_printf("frame buffer address %x\n", (uint)buffer);
#if defined(CONFIG_ARCH_SUN8IW8)
	layer_para->channel = 2;
#else
	layer_para->channel = 1;
#endif
	layer_para->layer_id = 0;
	layer_para->info.fb.format		= (bitcount == 24)? DISP_FORMAT_RGB_888:DISP_FORMAT_ARGB_8888;
	layer_para->info.fb.size[0].width	= width;
	layer_para->info.fb.size[0].height	= height;
	layer_para->info.fb.crop.x	= 0;
	layer_para->info.fb.crop.y	= 0;
	layer_para->info.fb.crop.width	= ((unsigned long long)width) << 32;
	layer_para->info.fb.crop.height	= ((unsigned long long)height) << 32;
	layer_para->info.fb.flags = DISP_BF_NORMAL;
	layer_para->info.fb.scan = DISP_SCAN_PROGRESSIVE;
	debug("bitcount = %d\n", bitcount);
	layer_para->info.mode     = LAYER_MODE_BUFFER;
	layer_para->info.alpha_mode    = 1;
	layer_para->info.alpha_value   = 0xff;
	if(full) {
		layer_para->info.screen_win.x	= 0;
		layer_para->info.screen_win.y	= 0;
		layer_para->info.screen_win.width	= screen_width;
		layer_para->info.screen_win.height	= screen_height;
	} else {
		layer_para->info.screen_win.x	= (screen_width - width) / 2;
		layer_para->info.screen_win.y	=  height / 2;
		layer_para->info.screen_win.width	= count;
		layer_para->info.screen_win.height	= 20;
	}
	layer_para->info.b_trd_out		= 0;
	layer_para->info.out_trd_mode 	= 0;

#elif defined(CONFIG_VIDEO_SUNXI_V2)
	disp_layer_info *layer_para;
	uint screen_width, screen_height;
	uint arg[4];

	if(!gd->layer_para)
	{
		layer_para = (disp_layer_info *)malloc(sizeof(disp_layer_info));
		if(!layer_para)
		{
			tick_printf("sunxi display error: unable to malloc memory for layer\n");

			return -1;
		}
	}
	else
	{
		layer_para = (disp_layer_info *)gd->layer_para;
	}
	arg[0] = screen_id;
	screen_width = disp_ioctl(NULL, DISP_CMD_GET_SCN_WIDTH, (void*)arg);
	screen_height = disp_ioctl(NULL, DISP_CMD_GET_SCN_HEIGHT, (void*)arg);
	printf("screen_id =%d, screen_width =%d, screen_height =%d\n", screen_id, screen_width, screen_height);
	memset((void *)layer_para, 0, sizeof(disp_layer_info));
	layer_para->fb.addr[0]		= (uint)buffer;
	debug("frame buffer address %x\n", (uint)buffer);
	layer_para->fb.format		= (bitcount == 24)? DISP_FORMAT_RGB_888:DISP_FORMAT_ARGB_8888;
	layer_para->fb.size.width	= width;
	layer_para->fb.size.height	= height;
	debug("bitcount = %d\n", bitcount);
	layer_para->fb.b_trd_src 	= 0;
	layer_para->fb.trd_mode		= 0;
	layer_para->ck_enable		= 0;
	layer_para->mode            = DISP_LAYER_WORK_MODE_NORMAL;
	layer_para->alpha_mode 		= 1;
	layer_para->alpha_value		= 0xff;
	layer_para->pipe 			= 0;
	layer_para->screen_win.x		= (screen_width - width) / 2;
	layer_para->screen_win.y		= (screen_height - height) / 2;
	layer_para->screen_win.width	= width;
	layer_para->screen_win.height	= height;
	layer_para->b_trd_out		= 0;
	layer_para->out_trd_mode 	= 0;
#else
	__disp_layer_info_t *layer_para;
	uint screen_width, screen_height;
	uint arg[4];

	if(!gd->layer_para)
	{
		layer_para = (__disp_layer_info_t *)malloc(sizeof(__disp_layer_info_t));
		if(!layer_para)
		{
			tick_printf("sunxi display error: unable to malloc memory for layer\n");

			return -1;
		}
	}
	else
	{
		layer_para = (__disp_layer_info_t *)gd->layer_para;
	}
	arg[0] = screen_id;
	screen_width = disp_ioctl(NULL, DISP_CMD_SCN_GET_WIDTH, (void*)arg);
	screen_height = disp_ioctl(NULL, DISP_CMD_SCN_GET_HEIGHT, (void*)arg);
	debug("screen_width =%d, screen_height =%d\n", screen_width, screen_height);
	memset((void *)layer_para, 0, sizeof(__disp_layer_info_t));
	layer_para->fb.addr[0]		= (uint)buffer;
	debug("frame buffer address %x\n", (uint)buffer);
	layer_para->fb.size.width	= width;
	layer_para->fb.size.height	= height;
	layer_para->fb.mode			= DISP_MOD_INTERLEAVED;
	layer_para->fb.format		= (bitcount == 24)? DISP_FORMAT_RGB888:DISP_FORMAT_ARGB8888;
	debug("bitcount = %d\n", bitcount);
	layer_para->fb.br_swap		= 0;
	layer_para->fb.seq			= DISP_SEQ_ARGB;
	layer_para->fb.b_trd_src 	= 0;
	layer_para->fb.trd_mode		= 0;
	layer_para->ck_enable		= 0;
	layer_para->mode            = DISP_LAYER_WORK_MODE_NORMAL;
	layer_para->alpha_en 		= 1;
	layer_para->alpha_val		= 0xff;
	layer_para->pipe 			= 0;
	layer_para->src_win.x		= 0;
	layer_para->src_win.y		= 0;
	layer_para->src_win.width	= width;
	layer_para->src_win.height	= height;
	layer_para->scn_win.x		= (screen_width - width) / 2;
	layer_para->scn_win.y		= (screen_height - height) / 2;
	layer_para->scn_win.width	= width;
	layer_para->scn_win.height	= height;
	layer_para->b_trd_out		= 0;
	layer_para->out_trd_mode 	= 0;
#endif

	gd->layer_para = (uint)layer_para;

	return 0;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int board_display_framebuffer_set(int width, int height, int bitcount, void *buffer)
{
#if defined(CONFIG_VIDEO_SUNXI_V3)
	disp_layer_config *layer_para;
	uint screen_width, screen_height;
	uint arg[4];
	int full = 0;

	if(!gd->layer_para)
	{
		layer_para = (disp_layer_config *)malloc(sizeof(disp_layer_config));
		if(!layer_para)
		{
			tick_printf("sunxi display error: unable to malloc memory for layer\n");

			return -1;
		}
	}
	else
	{
		layer_para = (disp_layer_config *)gd->layer_para;
	}

	if(script_parser_fetch("boot_disp", "output_full", &full, 1) < 0)
	{
		tick_printf("fetch script data boot_disp.output_disp fail\n");
		full = 0;
	}

	arg[0] = screen_id;
	screen_width = disp_ioctl(NULL, DISP_GET_SCN_WIDTH, (void*)arg);
	screen_height = disp_ioctl(NULL, DISP_GET_SCN_HEIGHT, (void*)arg);
	tick_printf("screen_id =%d, screen_width =%d, screen_height =%d\n", screen_id, screen_width, screen_height);
	if ((0 == screen_width) || (0 == screen_height))
		return -1;
	memset((void *)layer_para, 0, sizeof(disp_layer_config));
	layer_para->info.fb.addr[0]		= (uint)buffer;
	tick_printf("frame buffer address %x\n", (uint)buffer);
    gd->fb_base = (uint)buffer - sizeof(bmp_header_t);

#if defined(CONFIG_ARCH_SUN8IW8)
	layer_para->channel = 2;
#else
	layer_para->channel = 1;
#endif

	layer_para->layer_id = 0;
	layer_para->info.fb.format		= (bitcount == 24)? DISP_FORMAT_RGB_888:DISP_FORMAT_ARGB_8888;
	layer_para->info.fb.size[0].width	= width;
	layer_para->info.fb.size[0].height	= height;
	layer_para->info.fb.crop.x	= 0;
	layer_para->info.fb.crop.y	= 0;
	layer_para->info.fb.crop.width	= ((unsigned long long)width) << 32;
	layer_para->info.fb.crop.height	= ((unsigned long long)height) << 32;
	layer_para->info.fb.flags = DISP_BF_NORMAL;
	layer_para->info.fb.scan = DISP_SCAN_PROGRESSIVE;
	debug("bitcount = %d\n", bitcount);
	layer_para->info.mode     = LAYER_MODE_BUFFER;
	layer_para->info.alpha_mode    = 1;
	layer_para->info.alpha_value   = 0xff;
	if(full) {
		layer_para->info.screen_win.x	= 0;
		layer_para->info.screen_win.y	= 0;
		layer_para->info.screen_win.width	= screen_width;
		layer_para->info.screen_win.height	= screen_height;
	} else {
		layer_para->info.screen_win.x	= (screen_width - width) / 2;
		layer_para->info.screen_win.y	= (screen_height - height) / 2;
		layer_para->info.screen_win.width	= width;
		layer_para->info.screen_win.height	= height;
	}
	layer_para->info.b_trd_out		= 0;
	layer_para->info.out_trd_mode 	= 0;

#elif defined(CONFIG_VIDEO_SUNXI_V2)
	disp_layer_info *layer_para;
	uint screen_width, screen_height;
	uint arg[4];

	if(!gd->layer_para)
	{
		layer_para = (disp_layer_info *)malloc(sizeof(disp_layer_info));
		if(!layer_para)
		{
			tick_printf("sunxi display error: unable to malloc memory for layer\n");

			return -1;
		}
	}
	else
	{
		layer_para = (disp_layer_info *)gd->layer_para;
	}
	arg[0] = screen_id;
	screen_width = disp_ioctl(NULL, DISP_CMD_GET_SCN_WIDTH, (void*)arg);
	screen_height = disp_ioctl(NULL, DISP_CMD_GET_SCN_HEIGHT, (void*)arg);
	printf("screen_id =%d, screen_width =%d, screen_height =%d\n", screen_id, screen_width, screen_height);
	memset((void *)layer_para, 0, sizeof(disp_layer_info));
	layer_para->fb.addr[0]		= (uint)buffer;
	debug("frame buffer address %x\n", (uint)buffer);
	gd->fb_base = (uint)buffer - sizeof(bmp_header_t);
	layer_para->fb.format		= (bitcount == 24)? DISP_FORMAT_RGB_888:DISP_FORMAT_ARGB_8888;
	layer_para->fb.size.width	= width;
	layer_para->fb.size.height	= height;
	debug("bitcount = %d\n", bitcount);
	layer_para->fb.b_trd_src 	= 0;
	layer_para->fb.trd_mode		= 0;
	layer_para->ck_enable		= 0;
	layer_para->mode            = DISP_LAYER_WORK_MODE_NORMAL;
	layer_para->alpha_mode 		= 1;
	layer_para->alpha_value		= 0xff;
	layer_para->pipe 			= 0;
#if ((defined CONFIG_ARCH_HOMELET) && (defined CONFIG_ARCH_SUN9IW1P1))
 //re-calculate screen_window to show the same size on different disp_mode
	layer_para->fb.src_win.x = 0;
	layer_para->fb.src_win.y = 0;
	layer_para->fb.src_win.width  = width;
	layer_para->fb.src_win.height = height;
	board_display_reset_scn_win(&(layer_para->fb.src_win),screen_width,
	    screen_height, &(layer_para->screen_win));
	layer_para->mode            = DISP_LAYER_WORK_MODE_SCALER;
#else
	layer_para->screen_win.x		= (screen_width - width) / 2;
	layer_para->screen_win.y		= (screen_height - height) / 2;
	layer_para->screen_win.width	= width;
	layer_para->screen_win.height	= height;
#endif
	layer_para->b_trd_out		= 0;
	layer_para->out_trd_mode 	= 0;
#else
	__disp_layer_info_t *layer_para;
	uint screen_width, screen_height;
	uint arg[4];

	if(!gd->layer_para)
	{
		layer_para = (__disp_layer_info_t *)malloc(sizeof(__disp_layer_info_t));
		if(!layer_para)
		{
			tick_printf("sunxi display error: unable to malloc memory for layer\n");

			return -1;
		}
	}
	else
	{
		layer_para = (__disp_layer_info_t *)gd->layer_para;
	}
	arg[0] = screen_id;
	screen_width = disp_ioctl(NULL, DISP_CMD_SCN_GET_WIDTH, (void*)arg);
	screen_height = disp_ioctl(NULL, DISP_CMD_SCN_GET_HEIGHT, (void*)arg);
	debug("screen_width =%d, screen_height =%d\n", screen_width, screen_height);
	memset((void *)layer_para, 0, sizeof(__disp_layer_info_t));
	layer_para->fb.addr[0]		= (uint)buffer;
	debug("frame buffer address %x\n", (uint)buffer);
	layer_para->fb.size.width	= width;
	layer_para->fb.size.height	= height;
	layer_para->fb.mode			= DISP_MOD_INTERLEAVED;
	layer_para->fb.format		= (bitcount == 24)? DISP_FORMAT_RGB888:DISP_FORMAT_ARGB8888;
	debug("bitcount = %d\n", bitcount);
	layer_para->fb.br_swap		= 0;
	layer_para->fb.seq			= DISP_SEQ_ARGB;
	layer_para->fb.b_trd_src 	= 0;
	layer_para->fb.trd_mode		= 0;
	layer_para->ck_enable		= 0;
	layer_para->mode            = DISP_LAYER_WORK_MODE_NORMAL;
	layer_para->alpha_en 		= 1;
	layer_para->alpha_val		= 0xff;
	layer_para->pipe 			= 0;
	layer_para->src_win.x		= 0;
	layer_para->src_win.y		= 0;
	layer_para->src_win.width	= width;
	layer_para->src_win.height	= height;
	layer_para->scn_win.x		= (screen_width - width) / 2;
	layer_para->scn_win.y		= (screen_height - height) / 2;
	layer_para->scn_win.width	= width;
	layer_para->scn_win.height	= height;
	layer_para->b_trd_out		= 0;
	layer_para->out_trd_mode 	= 0;
#endif

	gd->layer_para = (uint)layer_para;

	return 0;
}

void board_display_set_alpha_mode(int mode)
{
    if(!gd->layer_para)
    {
        return;
    }
#if defined(CONFIG_VIDEO_SUNXI_V3)
	disp_layer_config *layer_para;
	layer_para = (disp_layer_config *)gd->layer_para;
	layer_para->info.alpha_mode = mode;
#elif defined(CONFIG_VIDEO_SUNXI_V2)
    disp_layer_info *layer_para;
    layer_para = (disp_layer_info *)gd->layer_para;
    layer_para->alpha_mode 		= mode;
#else
    __disp_layer_info_t *layer_para;
    layer_para = (__disp_layer_info_t *)gd->layer_para;
    layer_para->alpha_en 		= mode;
#endif
}

int board_display_framebuffer_change(void *buffer)
{
#if defined(CONFIG_VIDEO_SUNXI_V3) || defined(CONFIG_VIDEO_SUNXI_V2)
	return 0;
#else
    uint arg[4];
	__disp_fb_t disp_fb;
	__disp_layer_info_t *layer_para = (__disp_layer_info_t *)gd->layer_para;

	arg[0] = screen_id;
	arg[1] = gd->layer_hd;
	arg[2] = (uint)&disp_fb;
	arg[3] = 0;

	if(disp_ioctl(NULL, DISP_CMD_LAYER_GET_FB, (void*)arg))
	{
		tick_printf("sunxi display error :get framebuffer failed\n");

		return -1;
	}
	disp_fb.addr[0] = (uint)buffer;
	arg[0] = screen_id;
    arg[1] = gd->layer_hd;
    arg[2] = (uint)&disp_fb;
    arg[3] = 0;
	//debug("try to set framebuffer %x\n", (uint)buffer);
    if(disp_ioctl(NULL, DISP_CMD_LAYER_SET_FB, (void*)arg))
    {
        tick_printf("sunxi display error :set framebuffer failed\n");

		return -1;
	}
	layer_para->fb.addr[0] = (uint)buffer;

	return 0;
#endif
}

#if ((defined CONFIG_ARCH_HOMELET) && ((defined CONFIG_ARCH_SUN9IW1P1) || (defined CONFIG_ARCH_SUN8IW6P1) || (defined CONFIG_ARCH_SUN8IW7P1)))
int board_display_device_open(void)
{
	int  value;
	int  ret = 0;
	int output_type = 0;
	int output_mode = 0;
	int hdmi_used;
	int cvbs_used;
	int lcd_channel;
	int hdmi_channel;
	int cvbs_channel;
	int hdmi_connect = 0;
	int cvbs_connect = 0;
	int hdmi_mode = 0;
	int cvbs_mode = 0;
	unsigned long arg[4] = {0};
	int i;

	if(script_parser_fetch("boot_disp", "auto_hpd", &value, 1) < 0){
		tick_printf("###fetch script data boot_disp.auto_hpd fail! This must be configed!###\n");
		return -1;
	}
	if(script_parser_fetch("boot_disp", "hdmi_channel", &hdmi_channel, 1) < 0){
		printf("###fetch script data boot_disp.hdmi_channel fail! it must be conifged in homlet###\n");
		hdmi_used = 0;
	} else {
		hdmi_used = 1;
		saved_tv_vdid = get_saved_hdmi_vendor_id();
		if(0 == (hdmi_mode = get_display_resolution(DISP_OUTPUT_TYPE_HDMI)))
			if(script_parser_fetch("boot_disp", "hdmi_mode", &hdmi_mode, 1) < 0) {
				printf("###fetch script data boot_disp.hdmi_mode fail! This must be configed!###\n");
				return -1;
			}
    }
	if(script_parser_fetch("boot_disp", "cvbs_channel", &cvbs_channel, 1) < 0){
		cvbs_used = 0;
	} else {
		cvbs_used = 1;
		if(0 == (cvbs_mode = get_display_resolution(DISP_OUTPUT_TYPE_TV)))
			if(script_parser_fetch("boot_disp", "cvbs_mode", &cvbs_mode, 1) < 0){
				printf("###fetch script data boot_disp.cvbs_mode fail! This must be configed!###\n");
				return -1;
			}
	}

	printf("boot_disp.auto_hpd=%d\n", value);
	if((1 == value) && ((1 == hdmi_used) || (1 == cvbs_used))){
		// 1. auto_hpd, support hdmi and cvbs
		arg[0] = hdmi_channel;
		arg[1] = 0;
		hdmi_connect = disp_ioctl(NULL, MY_GET_HDMI_HPD_STATUS, (void*)arg);
		for(i=0; (i<100)&&(hdmi_connect==0) && ((0==cvbs_used)||(i<50)||(cvbs_connect==0)); i++){
			DELAY_MS(10);
			arg[0] = hdmi_channel;
			hdmi_connect = disp_ioctl(NULL, MY_GET_HDMI_HPD_STATUS, (void*)arg);
		    if(cvbs_connect == 0){
				arg[0] = cvbs_channel;
				cvbs_connect = disp_ioctl(NULL, MY_GET_CVBS_HPD_STATUS, (void*)arg);
		    }
		}
		printf("auto hpd check has %d times!\n",i);
		// 2. get output_type
		if(hdmi_connect != 0){
			output_type = DISP_OUTPUT_TYPE_HDMI;
		} else if(cvbs_connect != 0) {
			output_type = DISP_OUTPUT_TYPE_TV;
		} else {// i donot want to output none
			if(script_parser_fetch("boot_disp", "output_type", &output_type, 1) < 0){
				printf("###fetch script data boot_disp.output_type fail! This must be configed!###\n");
				return -1;
			}
		    printf("auto check no any connected, the output_type is %d\n", output_type);
		}
    } else if(0 == value){ // not auto_hpd for homlet
        if(script_parser_fetch("boot_disp", "output_type", &output_type, 1) < 0){
            printf("###output_type must be configed as auto_hpd=0!###\n");
            return -1;
        }
		printf("not auto hotplud, the default output_type is %d\n", output_type);
    } else {
        printf("check me: exception!!!\n");
    }
	// 3. get the screen_id and output_mode
    switch(output_type) {
    case DISP_OUTPUT_TYPE_HDMI:
        screen_id = hdmi_channel;
		if(hdmi_connect) {
			//check mode
			hdmi_mode = reset_hdmi_mode(hdmi_channel, hdmi_mode);
		} else {
			output_mode = hdmi_mode;
			goto go_out; // dont not to open hdmi
		}
		output_mode = hdmi_mode;
        break;
    case DISP_OUTPUT_TYPE_TV:
        screen_id = cvbs_channel;
		output_mode = cvbs_mode;
        break;
    case DISP_OUTPUT_TYPE_LCD:
        if(script_parser_fetch("boot_disp", "lcd_channel", &lcd_channel, 1) < 0){
            printf("###fetch script data boot_disp.hdmi_channel fail! it must be conifged in homlet###\n");
            return -1;
        }
        ban_lcd_bl = get_disp_ban_bl(lcd_channel);
        if (ban_lcd_bl) {
            printf("ban lcd bl in boot\n");
            return 0;
        }
        screen_id = lcd_channel;
        break;
    default:
		printf("check what the output_type=%d\n", output_type);
		return -1;
    }
	// 4. open device
#if (defined CONFIG_VIDEO_SUNXI_V3)
	/* CONFIG_VIDEO_SUNXI_V3 */
		arg[0] = screen_id;
		arg[1] = output_type;
		arg[2] = output_mode;
		ret = disp_ioctl(NULL, DISP_DEVICE_SWITCH, (void *)arg);
#else
	if(output_type == DISP_OUTPUT_TYPE_LCD)
	{
		printf("lcd open\n");
		arg[0] = screen_id;
		arg[1] = 0;
		arg[2] = 0;
#if (defined CONFIG_VIDEO_SUNXI_V1)
		ret = disp_ioctl(NULL, DISP_CMD_LCD_ON, (void*)arg);
#else
		ret = disp_ioctl(NULL, DISP_CMD_LCD_ENABLE, (void*)arg);
#endif
		debug("lcd open,ret=%d\n",ret);
	}
	else if(output_type == DISP_OUTPUT_TYPE_HDMI)
	{
		printf("hdmi open\n");
		arg[0] = screen_id;
		arg[1] = output_mode;
		arg[2] = 0;
		disp_ioctl(NULL, DISP_CMD_HDMI_SET_MODE, (void *)arg);
#if !(defined CONFIG_VIDEO_SUNXI_V2)
		ret = disp_ioctl(NULL, DISP_CMD_HDMI_ON, (void *)arg);
#else
		ret = disp_ioctl(NULL, DISP_CMD_HDMI_ENABLE, (void *)arg);
#endif
	}
	else if(output_type == DISP_OUTPUT_TYPE_TV)
	{
		printf("tv open\n");
		arg[0] = screen_id;
		arg[1] = output_mode;
		arg[2] = 0;
		disp_ioctl(NULL, DISP_CMD_TV_SET_MODE, (void *)arg);
		ret = disp_ioctl(NULL, DISP_CMD_TV_ON, (void *)arg);
	}
	else if(output_type == DISP_OUTPUT_TYPE_VGA)
	{
		printf("vga open\n");
		arg[0] = screen_id;
		arg[1] = output_mode;
		arg[2] = 0;
	}
	else
	{
		printf("open device err, output_type=%d\n", output_type);
	}
#endif

go_out:
	// 5. save display para as disp_para
#if (defined CONFIG_ARCH_SUN9IW1P1)
	disp_para = ((output_type & 0xff) << 16) | ((output_mode & 0xff) << 8) | ((screen_id & 0xff) << 0);
	disp_para = (0 == ret) ? disp_para : (disp_para | 0xFF000000);
#else
    if(DISP_OUTPUT_TYPE_HDMI == output_type && 0 == hdmi_connect) {
        disp_para = 0;
    } else {
		disp_para = ((output_type << 8) | (output_mode)) << (screen_id*16);
	}
#endif
	tick_printf("finally, output_type=0x%x, output_mode=0x%x, screen_id=0x%x, disp_para=0x%x\n",
		output_type, output_mode, screen_id, disp_para);
	if(hdmi_used)
		init_disp |= (((DISP_OUTPUT_TYPE_HDMI & 0xff) << 8) | (hdmi_mode & 0xff)) << (hdmi_channel << 4);
	if(cvbs_used)
		init_disp |= (((DISP_OUTPUT_TYPE_TV & 0xff) << 8) | (cvbs_mode & 0xff)) << (cvbs_channel << 4);

	return ret;
}

#else

int board_display_device_open(void)
{
#if (defined CONFIG_ARCH_SUN8IW5P1)
	int  ret = 0;
	__u32 output_type = 0;
	__u32 output_mode = 0;
	unsigned long arg[4] = {0};
	output_type = DISP_OUTPUT_TYPE_LCD;
	if(output_type == DISP_OUTPUT_TYPE_LCD)
	{
		debug("lcd open\n");
		arg[0] = screen_id;
		arg[1] = 0;
		arg[2] = 0;
		ret = disp_ioctl(NULL, DISP_CMD_LCD_ENABLE, (void*)arg);
		debug("lcd open,ret=%d\n",ret);
	}
	else if(output_type == DISP_OUTPUT_TYPE_HDMI)
	{
	}
#else
	int  value = 1;
	int  ret = 0;
	__u32 output_type = 0;
	__u32 output_mode = 0;
	__u32 auto_hpd = 0;
	__u32 err_count = 0;
	unsigned long arg[4] = {0};
	int i;

	debug("De_OpenDevice\n");

//channel
	if(script_parser_fetch("boot_disp", "output_disp", &value, 1) < 0)
	{
		tick_printf("fetch script data boot_disp.output_disp fail\n");
		err_count ++;
		value = 0;
	}
	else
	{
		tick_printf("boot_disp.output_disp=%d\n", value);
		screen_id = value;
	}

//screen0_output_type
	if(script_parser_fetch("boot_disp", "output_type", &value, 1) < 0)
	{
		tick_printf("fetch script data boot_disp.output_type fail\n");
		err_count ++;
		value = 0;
	}
	else
	{
		tick_printf("boot_disp.output_type=%d\n", value);
	}

	if(value == 0)
	{
		output_type = DISP_OUTPUT_TYPE_NONE;
	}
	else if(value == 1)
	{
		output_type = DISP_OUTPUT_TYPE_LCD;
	}
	else if(value == 2)
	{
		output_type = DISP_OUTPUT_TYPE_TV;
	}
	else if(value == 3)
	{
		output_type = DISP_OUTPUT_TYPE_HDMI;
	}
	else if(value == 4)
	{
		output_type = DISP_OUTPUT_TYPE_VGA;
	}
	else
	{
		tick_printf("invalid screen0_output_type %d\n", value);
		return -1;
	}
//screen0_output_mode
	if(script_parser_fetch("boot_disp", "output_mode", &value, 1) < 0)
	{
		tick_printf("fetch script data boot_disp.output_mode fail\n");
		err_count ++;
		value = 0;
	}
	else
	{
		tick_printf("boot_disp.output_mode=%d\n", value);
	}

	if(output_type == DISP_OUTPUT_TYPE_TV || output_type == DISP_OUTPUT_TYPE_HDMI)
	{
		output_mode = value;
	}
	else if(output_type == DISP_OUTPUT_TYPE_VGA)
	{
		output_mode = value;
	}

//auto hot plug detect
	if(script_parser_fetch("boot_disp", "auto_hpd", &value, 1) < 0)
	{
		tick_printf("fetch script data boot_disp.auto_hpd fail\n");
		err_count ++;
		value = 0;
	}else
	{
		auto_hpd = value;
		tick_printf("boot_disp.auto_hpd=%d\n", value);
	}

	if(err_count == 4)//no boot_disp config
	{
		if(script_parser_fetch("lcd0_para", "lcd_used", &value, 1) < 0)
		{
			tick_printf("fetch script data lcd0_para.lcd_used fail\n");
			value = 0;
		}else
		{
			tick_printf("lcd0_para.lcd_used=%d\n", value);
		}

		if(value == 1) //lcd available
		{
			output_type = DISP_OUTPUT_TYPE_LCD;
		}
		else
		{
			arg[0] = screen_id;
			arg[1] = 0;
			arg[2] = 0;
			ret = 0;
			for(i=0; (i<3)&&(ret==0); i++)
			{
#if defined(CONFIG_VIDEO_SUNXI_V1) || defined(CONFIG_VIDEO_SUNXI_V2)
				ret = disp_ioctl(NULL, DISP_CMD_HDMI_GET_HPD_STATUS, (void*)arg);
#endif
			}
			if(ret == 1)
			{
				output_type = DISP_OUTPUT_TYPE_HDMI;
				//output_mode = (output_mode == -1)? DISP_TV_MOD_720P_50HZ:output_mode;
				output_mode = DISP_TV_MOD_720P_50HZ;
			}
		}

	}
	else//has boot_disp config
	{
		if(output_type == DISP_OUTPUT_TYPE_LCD)
		{

		}
		else if(auto_hpd == 1)
		{
			arg[0] = screen_id;
			arg[1] = 0;
			for(i=0; (i<3)&&(ret==0); i++)
			{
#if defined(CONFIG_VIDEO_SUNXI_V1) || defined(CONFIG_VIDEO_SUNXI_V2)
				ret=disp_ioctl(NULL, DISP_CMD_HDMI_GET_HPD_STATUS, (void*)arg);
                        debug("hdmi pending%d\n",i);
                       // disp_delay_ms(200);
#endif

			}
			if (ret == 1)
			{
				output_type = DISP_OUTPUT_TYPE_HDMI;
				output_mode = (output_mode == -1)? DISP_TV_MOD_720P_50HZ:output_mode;
				//output_mode = DISP_TV_MOD_1080P_60HZ;
				debug("------DISP_OUTPUT_TYPE_HDMI-----\n");

			}
			else
			{
#if defined(CONFIG_VIDEO_SUNXI_V1)
                    ret = 0;
                    arg[0] = screen_id;
                    arg[1] = 0;
			for(i=0; (i<6)&&(ret==0); i++)
                    {
                        debug("%d\n",i);
                        ret = disp_ioctl(NULL, DISP_CMD_TV_GET_INTERFACE, (void*)arg);
                        disp_delay_ms(200);
                    }
                       debug("tv detect, ret = %d\n", ret);
                    if((ret & DISP_TV_CVBS) == DISP_TV_CVBS)
                    {
                        output_type = DISP_OUTPUT_TYPE_TV;
                        output_mode = DISP_TV_MOD_PAL;
			debug("------DISP_TV_CVBS-----\n");
                    }else if((ret & DISP_TV_YPBPR) == DISP_TV_YPBPR)
                    {

                            output_type = DISP_OUTPUT_TYPE_VGA;
                            output_mode = DISP_VGA_H1024_V768;
			debug("------DISP_TV_YPBPR-----\n");

                    }else
                    {
                        output_type = DISP_OUTPUT_TYPE_NONE;
			debug("------DISP_OUTPUT_TYPE_NONE-----\n");

                    }
#endif
                }
		}
	}
	if(0 != (value = get_display_resolution(output_type)))
	{
		output_mode = value;
		printf("Get android config:output:type=%d,mode=%d\n", output_type, output_mode);
	}

#if !defined(CONFIG_VIDEO_SUNXI_V3)
	if(output_type == DISP_OUTPUT_TYPE_LCD)
	{
		printf("lcd open\n");
		arg[0] = screen_id;
		arg[1] = 0;
		arg[2] = 0;
#if !(defined CONFIG_ARCH_SUN9IW1P1) && !(defined CONFIG_ARCH_SUN8IW8P1)&& !(defined CONFIG_ARCH_SUN8IW7P1)
		ret = disp_ioctl(NULL, DISP_CMD_LCD_ON, (void*)arg);
#else
		ret = disp_ioctl(NULL, DISP_CMD_LCD_ENABLE, (void*)arg);
#endif
		debug("lcd open,ret=%d\n",ret);
	}
	else if(output_type == DISP_OUTPUT_TYPE_HDMI)
	{
		printf("hdmi open\n");
		arg[0] = screen_id;
		arg[1] = output_mode;
		arg[2] = 0;
#if !(defined CONFIG_ARCH_SUN9IW1P1) && !(defined CONFIG_ARCH_SUN8IW8P1)&& !(defined CONFIG_ARCH_SUN8IW7P1)
		disp_ioctl(NULL, DISP_CMD_HDMI_SET_MODE, (void *)arg);
		ret = disp_ioctl(NULL, DISP_CMD_HDMI_ON, (void *)arg);
#else
		disp_ioctl(NULL, DISP_CMD_HDMI_SET_MODE, (void *)arg);
		ret = disp_ioctl(NULL, DISP_CMD_HDMI_ENABLE, (void *)arg);
#endif
	}
#if defined(CONFIG_VIDEO_SUNXI_V1)
	else if(output_type == DISP_OUTPUT_TYPE_TV)
	{
		printf("tv open\n");
		//udelay(1000*1000*10);
		arg[0] = screen_id;
		arg[1] = output_mode;
		arg[2] = 0;
		disp_ioctl(NULL, DISP_CMD_TV_SET_MODE, (void *)arg);
		ret = disp_ioctl(NULL, DISP_CMD_TV_ON, (void *)arg);
	}
	else if(output_type == DISP_OUTPUT_TYPE_VGA)
	{
		printf("vga open\n");
		//udelay(1000*200);
		arg[0] = screen_id;
		arg[1] = output_mode;
		arg[2] = 0;
		disp_ioctl(NULL, DISP_CMD_VGA_SET_MODE, (void *)arg);
		ret = disp_ioctl(NULL, DISP_CMD_VGA_ON, (void *)arg);
	}
#endif // !(defined CONFIG_ARCH_SUN9IW1P1)

#else
	printf("disp%d device type(%d) enable\n", screen_id, output_type);
/* CONFIG_VIDEO_SUNXI_V3 */
	arg[0] = screen_id;
	arg[1] = output_type;
	arg[2] = output_mode;
	disp_ioctl(NULL, DISP_DEVICE_SWITCH, (void *)arg);
#endif // !CONFIG_VIDEO_SUNXI_V3

#endif // CONFIG_ARCH_SUN8IW5P1
	disp_para = ((output_type << 8) | (output_mode)) << (screen_id*16);
	return ret;
}
#endif //((defined CONFIG_ARCH_HOMELET) && (defined CONFIG_ARCH_SUN9IW1P1))

int board_display_setenv(char *data)
{
	if (!data)
		return -1;

#if ((defined CONFIG_ARCH_HOMELET) && (defined CONFIG_ARCH_SUN9IW1P1))
	sprintf(data, " disp_rsl=%x init_disp=%x tv_vdid=%x",
		disp_para, init_disp, saved_tv_vdid);
#else
	sprintf(data, " disp_para=%x init_disp=%x tv_vdid=%x ban_bl=%x",
		disp_para, init_disp, saved_tv_vdid, ban_lcd_bl);
#endif
	printf("board_display_setenv: %s\n", data);
	return 0;
}

int borad_display_get_screen_width(void)
{
	unsigned long arg[4] = {0};

	arg[0] = screen_id;
#if defined(CONFIG_VIDEO_SUNXI_V3)
	return disp_ioctl(NULL, DISP_GET_SCN_WIDTH, (void*)arg);
#elif defined(CONFIG_VIDEO_SUNXI_V2)
	return disp_ioctl(NULL, DISP_CMD_GET_SCN_WIDTH, (void*)arg);
#else
	return disp_ioctl(NULL, DISP_CMD_SCN_GET_WIDTH, (void*)arg);
#endif
}

int borad_display_get_screen_height(void)
{
	unsigned long arg[4] = {0};

	arg[0] = screen_id;
#if defined(CONFIG_VIDEO_SUNXI_V3)
	return disp_ioctl(NULL, DISP_GET_SCN_HEIGHT, (void*)arg);
#elif defined(CONFIG_VIDEO_SUNXI_V2)
	return disp_ioctl(NULL, DISP_CMD_GET_SCN_HEIGHT, (void*)arg);
#else
	return disp_ioctl(NULL, DISP_CMD_SCN_GET_HEIGHT, (void*)arg);
#endif
}

int get_display_resolution(int display_type)
{
  char disp_set_buf[512] = {0};
  int read_bytes = 0;
  int temp_value = 0;
  char *p_buf = disp_set_buf;
  char str[10] = {0};
  int i = 0;

    read_bytes = aw_fat_fsload("Reserve0", "disp_rsl.fex", disp_set_buf, 512);

	while (read_bytes > 0)
	{
		if (*p_buf == '\n')
		{
			//printf("str = %s\n", str);
			temp_value = (ulong)simple_strtoul(str, NULL, 16);
			if ((temp_value >> 8 & 0xff) == display_type)
			{
				printf("[get_display_resolution] = %x\n", temp_value);
				return (temp_value & 0xff);
			}
			i = 0;
		}
		else
		{
			str[i++] = *p_buf;
		}
		p_buf++;
		str[i] = '\0';
		read_bytes--;
	}
  return 0;
}

int get_saved_hdmi_vendor_id(void)
{
    char data_buf[512] = {0};
    char *pvendor_id, *pdata, *pdata_end;
    int vendor_id = 0;
    int i = 0;
    int ret = 0;

    memset(data_buf, 0, 512);
	ret = aw_fat_fsload("Reserve0", "tv_vdid.fex", data_buf, 512);
    if(0 >= ret)
        return 0;
    pvendor_id = data_buf;
    pdata = data_buf;
    pdata_end = pdata + ret;
    for(;(i < 4) && (pdata != pdata_end); pdata++) {
        if('\n' == *pdata) {
            *pdata = '\0';
            ret = (int)simple_strtoul(pvendor_id, NULL, 16);
            vendor_id |= ((ret & 0xFF) << (8 * (3 - i)));
            //printf("pvendor_id=%s, ret=0x%x, vendorID=0x%x\n",
            //    pvendor_id, ret, vendor_id);
            pvendor_id = pdata + 1;
            i++;
        }
    }
    return vendor_id;
}

int get_disp_ban_bl(int sel)
{
    char data_buf[8] = {0};
    int read_bytes = 0;

    read_bytes = aw_fat_fsload("Reserve0", "ban_bl.fex", data_buf, sizeof(data_buf));

    if (read_bytes >= 0)
    {
        printf("[get_display_setting]str = %s\n", data_buf);
        return 1;
    }
    return 0;
}

int get_hdmi_edid(int sel, unsigned char *edid_buf, int length)
{
	unsigned long arg[4] = {0};

	arg[0] = sel;
	arg[1] = (unsigned long)edid_buf;
	arg[2] = length;
	disp_ioctl(NULL, MY_HDMI_GET_EDID, (void*)arg);
	return 0;
}

int get_hdmi_vendor_id(int sel)
{
#define ID_VENDOR 0x08
	int vendor_id = 0;
	unsigned char *pVendor = NULL;
	unsigned char edid_buf[1024] = {0};

	get_hdmi_edid(sel, edid_buf, 1024);
	pVendor = edid_buf + ID_VENDOR;
	vendor_id = (pVendor[0] << 24);
	vendor_id |= (pVendor[1] << 16);
	vendor_id |= (pVendor[2] << 8);
	vendor_id |= pVendor[3];
	//printf("vendor_id=%d\n", vendor_id);
	return vendor_id;
}

int reset_hdmi_mode(int sel, int mode)
{
	unsigned long arg[4] = {0};
	int hdmi_mode_check = 1;
	int hdmi_vendor_id = 0;
	int saved_hdmi_vendor_id = saved_tv_vdid;
    const int HDMI_INLINE_DEFAULT_MODE = DISP_TV_MOD_720P_50HZ;

	if(script_parser_fetch("boot_disp", "hdmi_mode_check", &hdmi_mode_check, 1) < 0){
		hdmi_mode_check = 1;
	}
	if(2 == hdmi_mode_check){
		// if vendor id change , check mode: hdmi_mode_check = 1;
		hdmi_vendor_id = get_hdmi_vendor_id(sel);
		if(hdmi_vendor_id && (hdmi_vendor_id != saved_hdmi_vendor_id)) {
			hdmi_mode_check = 1;
			saved_tv_vdid = hdmi_vendor_id;
			printf("vendor:0x%x, saved_vendor:0x%x\n",
				hdmi_vendor_id, saved_hdmi_vendor_id);
		}
	}
	if(1 == hdmi_mode_check){  //MY_HDMI_SUPPORT_MODE: check if support the output_mode by television, return 0 is not support
		arg[0] = sel;
		arg[1] = mode;
		if(0 == disp_ioctl(NULL, MY_HDMI_SUPPORT_MODE, (void*)arg)) {
			printf("not support mode[%d], use inline mode[%d]\n",
				mode, HDMI_INLINE_DEFAULT_MODE);
			mode = HDMI_INLINE_DEFAULT_MODE;
		}
	}

	return mode;
}

#else
int board_display_layer_request(void)
{
	return 0;
}

int board_display_layer_release(void)
{
	return 0;
}
int board_display_wait_lcd_open(void)
{
	return 0;
}
int board_display_wait_lcd_close(void)
{
	return 0;
}
int board_display_set_exit_mode(int lcd_off_only)
{
	return 0;
}
int board_display_layer_open(void)
{
	return 0;
}

int board_display_layer_close(void)
{
	return 0;
}

int board_display_layer_para_set(void)
{
	return 0;
}

int board_display_show_until_lcd_open(int display_source)
{
	return 0;
}

int board_display_show(int display_source)
{
	return 0;
}

int board_display_framebuffer_set(int width, int height, int bitcount, void *buffer)
{
	return 0;
}

void board_display_set_alpha_mode(int mode)
{
	return ;
}

int board_display_framebuffer_change(void *buffer)
{
	return 0;
}
int board_display_device_open(void)
{
	return 0;
}

int borad_display_get_screen_width(void)
{
	return 0;
}

int borad_display_get_screen_height(void)
{
	return 0;
}

int board_display_setenv(char *data)
{
	return -1;
}

#endif
