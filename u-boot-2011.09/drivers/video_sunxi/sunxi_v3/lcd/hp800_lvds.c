#include "default_panel.h"

#define hp800_lvds_panel_reset(val)	\
	sunxi_lcd_gpio_set_value(0, 0, val)

static void lcd_power_on(u32 sel);
static void lcd_power_off(u32 sel);
static void lcd_blackright_open(u32 sel);
static void lcd_blackright_close(u32 sel);

static void lcd_panel_init(u32 sel);
static void lcd_panel_exit(u32 sel);

const int blacklight_table[256] = {
127, 127, 127, 127, 127, 127, 127, 128, 128, 128, 128, 128, 128, 128, 129, 129,
129, 129, 129, 129, 129, 130, 130, 130, 130, 130, 130, 131, 131, 131, 131, 131,
131, 131, 132, 132, 132, 132, 132, 132, 132, 133, 133, 133, 133, 133, 133, 134,
134, 134, 134, 134, 134, 134, 135, 135, 135, 135, 135, 135, 135, 136, 136, 136,
136, 136, 136, 136, 137, 137, 137, 137, 137, 137, 138, 138, 138, 138, 138, 138,
138, 139, 139, 139, 139, 139, 139, 139, 140, 140, 140, 140, 140, 140, 141, 141,
141, 141, 141, 141, 141, 142, 142, 142, 142, 142, 142, 142, 143, 143, 143, 143,
143, 143, 143, 144, 144, 144, 144, 144, 144, 145, 145, 145, 145, 145, 145, 145,
146, 146, 146, 146, 146, 146, 146, 147, 147, 147, 147, 147, 147, 148, 148, 148,
148, 148, 148, 148, 149, 149, 149, 149, 149, 149, 149, 150, 150, 150, 150, 150,
150, 150, 151, 151, 151, 151, 151, 151, 152, 152, 152, 152, 152, 152, 152, 153,
153, 153, 153, 153, 153, 153, 154, 154, 154, 154, 154, 154, 155, 155, 155, 155,
155, 155, 155, 156, 156, 156, 156, 156, 156, 156, 157, 157, 157, 157, 157, 157,
157, 158, 158, 158, 158, 158, 158, 159, 159, 159, 159, 159, 159, 159, 160, 160,
160, 160, 160, 160, 160, 161, 161, 161, 161, 161, 161, 162, 162, 162, 162, 162,
162, 162, 163, 163, 163, 163, 163, 163, 163, 164, 164, 164, 164, 164, 164, 165
};

static void lcd_cfg_panel_info(panel_extend_para * info)
{
	u32 i = 0, j = 0;
	u32 items;

	/* gamma table: {input value, corrected value} */
	u8 lcd_gamma_tbl[][2] = {
		{  0,   0}, { 15,  15}, { 30,  30}, { 45,  45},
		{ 60,  60}, { 75,  75}, { 90,  90}, {105, 105},
		{120, 120}, {135, 135}, {150, 150}, {165, 165},
		{180, 180}, {195, 195}, {210, 210}, {225, 225},
		{240, 240}, {255, 255},
	};

	u32 lcd_cmap_tbl[2][3][4] = {
		{
			{LCD_CMAP_G0, LCD_CMAP_B1, LCD_CMAP_G2, LCD_CMAP_B3},
			{LCD_CMAP_B0, LCD_CMAP_R1, LCD_CMAP_B2, LCD_CMAP_R3},
			{LCD_CMAP_R0, LCD_CMAP_G1, LCD_CMAP_R2, LCD_CMAP_G3},
		},
		{
			{LCD_CMAP_B3, LCD_CMAP_G2, LCD_CMAP_B1, LCD_CMAP_G0},
			{LCD_CMAP_R3, LCD_CMAP_B2, LCD_CMAP_R1, LCD_CMAP_B0},
			{LCD_CMAP_G3, LCD_CMAP_R2, LCD_CMAP_G1, LCD_CMAP_R0},
		},
	};

	items = sizeof(lcd_gamma_tbl) / 2;
	for (i = 0; i < items - 1; i++) {
		u32 num = lcd_gamma_tbl[i + 1][0] - lcd_gamma_tbl[i][0];

		for (j = 0; j < num; j++) {
			u32 value = 0;

			value = lcd_gamma_tbl[i][1] +
					((lcd_gamma_tbl[i + 1][1] - lcd_gamma_tbl[i][1]) * j) / num;

			info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] =
					(value << 16) + (value << 8) + value;
		}
	}

	info->lcd_gamma_tbl[255] =
		(lcd_gamma_tbl[items - 1][1] << 16) +
		(lcd_gamma_tbl[items - 1][1] <<  8) + lcd_gamma_tbl[items - 1][1];

	memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));

	/* adjust lcd blacklight */
	for (i = 0; i < 256; i++)
		info->lcd_bright_curve_tbl[i] = blacklight_table[i];
}

static s32 lcd_open_flow(u32 sel)
{
	LCD_OPEN_FUNC(sel, lcd_power_on, 30);
	LCD_OPEN_FUNC(sel, lcd_panel_init, 50);
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 100);
	LCD_OPEN_FUNC(sel, lcd_blackright_open, 0);
	return 0;
}

static s32 lcd_close_flow(u32 sel)
{
	LCD_CLOSE_FUNC(sel, lcd_blackright_close, 0);
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);
	LCD_CLOSE_FUNC(sel, lcd_panel_exit, 200);
	LCD_CLOSE_FUNC(sel, lcd_power_off, 500);
	return 0;
}

static void lcd_power_on(u32 sel)
{
	sunxi_lcd_power_enable(sel, 0);
	sunxi_lcd_pin_cfg(sel, 1);
}

static void lcd_power_off(u32 sel)
{
	sunxi_lcd_pin_cfg(sel, 0);
	sunxi_lcd_power_disable(sel, 0);
}

static void lcd_blackright_open(u32 sel)
{
	sunxi_lcd_pwm_enable(sel);
	sunxi_lcd_backlight_enable(sel);
}

static void lcd_blackright_close(u32 sel)
{
	sunxi_lcd_backlight_disable(sel);
	sunxi_lcd_pwm_disable(sel);
}

static void lcd_panel_init(u32 sel)
{
	printf("++ %s: called\n", __func__);

	hp800_lvds_panel_reset(1);
	sunxi_lcd_delay_ms(20);
	hp800_lvds_panel_reset(0);
	sunxi_lcd_delay_ms(60);
	hp800_lvds_panel_reset(1);
	sunxi_lcd_delay_ms(100);

	return;
}

static void lcd_panel_exit(u32 sel)
{
	return;
}

/*
 * sel: 0 -- lcd0
 *      1 -- lcd1
 */
static s32 lcd_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
	return 0;
}

__lcd_panel_t hp800_lvds = {
	.name = "hp800_lvds",
	.func = {
		.cfg_panel_info = lcd_cfg_panel_info,
		.cfg_open_flow = lcd_open_flow,
		.cfg_close_flow = lcd_close_flow,
		.lcd_user_defined_func = lcd_user_defined_func,
	},
};

