#include <linux/delay.h>
#include <mach/gpio.h>
#include <mach/iomux.h>
#include <mach/board.h>
#if defined(CONFIG_RK_HDMI)
#include "../../rockchip/hdmi/rk_hdmi.h"
#endif
#ifdef CONFIG_RK610_LVDS
#include "../transmitter/rk610_lcd.h"
#endif

#define HDMI_NO_DEBUG			0
#define HDMI_1920X1080P_60HZ 		1
#define HDMI_1920X1080P_50HZ 		2
#define HDMI_1280X720P_60HZ 		3
#define HDMI_1280X720P_50HZ 		4
#define HDMI_720X576P 			5
#define HDMI_720X480P 			6

#define DEBUG_HDMI_SCREEN   	HDMI_720X480P
/* Base */
#define OUT_TYPE		SCREEN_RGB

#define OUT_FACE		OUT_P888    //OUT_P888  
#define OUT_CLK			 50000000        // 65000000
#define LCDC_ACLK        33000000//312000000           //29 lcdc axi DMA Ƶ��

/* Timing */
#define H_PW			30
#define H_BP			16
#define H_VD			800
#define H_FP			210

#define V_PW			13
#define V_BP			10
#define V_VD			600
#define V_FP			12

#define LCD_WIDTH       154
#define LCD_HEIGHT      91//600

/* scaler Timing    */
//1920*1080*60

#define S_OUT_CLK		SCALE_RATE(148500000,33000000)  //m=16 n=9 no=8 
#define S_H_PW			30
#define S_H_BP			16
#define S_H_VD			800
#define S_H_FP			34

#define S_V_PW			3
#define S_V_BP			10
#define S_V_VD			600
#define S_V_FP			12

#define S_H_ST			0
#define S_V_ST			12

//1920*1080*50
#define S1_OUT_CLK		SCALE_RATE(148500000,30375000)  //m=18 n=11 no=8 
#define S1_H_PW			30
#define S1_H_BP			16
#define S1_H_VD			800
#define S1_H_FP			126

#define S1_V_PW			3
#define S1_V_BP			9
#define S1_V_VD			600
#define S1_V_FP			13

#define S1_H_ST			0
#define S1_V_ST			13
//1280*720*60
#define S2_OUT_CLK		SCALE_RATE(74250000,33000000)  //m=32 n=9 no=8 
#define S2_H_PW			30
#define S2_H_BP			16
#define S2_H_VD			800
#define S2_H_FP			34

#define S2_V_PW			10
#define S2_V_BP			10
#define S2_V_VD			600
#define S2_V_FP			6

#define S2_H_ST			0
#define S2_V_ST			10
//1280*720*50

#define S3_OUT_CLK		SCALE_RATE(74250000,30375000)   // m=36 n=11 no=8 
#define S3_H_PW			30
#define S3_H_BP			16
#define S3_H_VD			800
#define S3_H_FP			126

#define S3_V_PW			10
#define S3_V_BP			10
#define S3_V_VD			600
#define S3_V_FP			12

#define S3_H_ST			0
#define S3_V_ST			10

//720*576*50
#define S4_OUT_CLK		SCALE_RATE(27000000,31640625)  //m=75 n=8 no=8 
#define S4_H_PW			30
#define S4_H_BP			16
#define S4_H_VD			800
#define S4_H_FP			126

#define S4_V_PW			6
#define S4_V_BP			20
#define S4_V_VD			600
#define S4_V_FP			25

#define S4_H_ST			40
#define S4_V_ST			25
//720*480*60
#define S5_OUT_CLK		SCALE_RATE(27000000,39375000)  //m=35 n=3 no=8 
#define S5_H_PW			30
#define S5_H_BP			16
#define S5_H_VD			800
#define S5_H_FP			165

#define S5_V_PW			6
#define S5_V_BP			20
#define S5_V_VD			600
#define S5_V_FP			25

#define S5_H_ST			250
#define S5_V_ST			24

#define S_DCLK_POL       0

/* Other */
#define DCLK_POL	0
#define DEN_POL		0
#define VSYNC_POL	0
#define HSYNC_POL	0

#define SWAP_RB		0
#define SWAP_RG		0
#define SWAP_GB		0
 

#if  ( defined(CONFIG_ONE_LCDC_DUAL_OUTPUT_INF)&& defined(CONFIG_RK610_LVDS) ) || defined(CONFIG_HDMI_DUAL_DISP)

static int set_scaler_info(struct rk29fb_screen *screen, u8 hdmi_resolution)
{
	screen->s_clk_inv = S_DCLK_POL;
	screen->s_den_inv = 0;
	screen->s_hv_sync_inv = 0;
	static int count=0;
	switch(hdmi_resolution){
	case HDMI_1920x1080p_60Hz:
		/* Scaler Timing    */
		screen->hdmi_resolution = hdmi_resolution;
		screen->s_pixclock = S_OUT_CLK;
		screen->s_hsync_len = S_H_PW;
		screen->s_left_margin = S_H_BP;
		screen->s_right_margin = S_H_FP;
		screen->s_hsync_len = S_H_PW;
		screen->s_upper_margin = S_V_BP;
		screen->s_lower_margin = S_V_FP;
		screen->s_vsync_len = S_V_PW;
		screen->s_hsync_st = S_H_ST;
#if(DEBUG_HDMI_SCREEN == HDMI_1920X1080P_60HZ)
		screen->s_vsync_st = S_V_ST + count;
		count++;
#else
		screen->s_vsync_st = S_V_ST;
#endif
		break;
	case HDMI_1920x1080p_50Hz:
		/* Scaler Timing    */
		screen->hdmi_resolution = hdmi_resolution;
		screen->s_pixclock = S1_OUT_CLK;
		screen->s_hsync_len = S1_H_PW;
		screen->s_left_margin = S1_H_BP;
		screen->s_right_margin = S1_H_FP;
		screen->s_hsync_len = S1_H_PW;
		screen->s_upper_margin = S1_V_BP;
		screen->s_lower_margin = S1_V_FP;
		screen->s_vsync_len = S1_V_PW;
		screen->s_hsync_st = S1_H_ST;
#if(DEBUG_HDMI_SCREEN == HDMI_1920X1080P_50HZ)
		screen->s_vsync_st = S1_V_ST + count;
		count++;
#else
		screen->s_vsync_st = S1_V_ST;
#endif
		break;
	case HDMI_1280x720p_60Hz:
		/* Scaler Timing    */
		screen->hdmi_resolution = hdmi_resolution;
		screen->s_pixclock = S2_OUT_CLK;
		screen->s_hsync_len = S2_H_PW;
		screen->s_left_margin = S2_H_BP;
		screen->s_right_margin = S2_H_FP;
		screen->s_hsync_len = S2_H_PW;
		screen->s_upper_margin = S2_V_BP;
		screen->s_lower_margin = S2_V_FP;
		screen->s_vsync_len = S2_V_PW;
		screen->s_hsync_st = S2_H_ST;
#if(DEBUG_HDMI_SCREEN == HDMI_1280X720P_60HZ)
		screen->s_vsync_st = S2_V_ST + count;
		count++;
#else
		screen->s_vsync_st = S2_V_ST;
#endif
		break;
	case HDMI_1280x720p_50Hz:
		/* Scaler Timing    */
		screen->hdmi_resolution = hdmi_resolution;
		screen->s_pixclock = S3_OUT_CLK;
		screen->s_hsync_len = S3_H_PW;
		screen->s_left_margin = S3_H_BP;
		screen->s_right_margin = S3_H_FP;
		screen->s_hsync_len = S3_H_PW;
		screen->s_upper_margin = S3_V_BP;
		screen->s_lower_margin = S3_V_FP;
		screen->s_vsync_len = S3_V_PW;
		screen->s_hsync_st = S3_H_ST;
#if(DEBUG_HDMI_SCREEN == HDMI_1280X720P_50HZ)
		screen->s_vsync_st = S3_V_ST + count;
		count++;
#else
		screen->s_vsync_st = S3_V_ST;
#endif
		break;
	case HDMI_720x576p_50Hz_4_3:
	case HDMI_720x576p_50Hz_16_9:
		/* Scaler Timing    */
		screen->hdmi_resolution = hdmi_resolution;
		screen->s_pixclock = S4_OUT_CLK;
		screen->s_hsync_len = S4_H_PW;
		screen->s_left_margin = S4_H_BP;
		screen->s_right_margin = S4_H_FP;
		screen->s_hsync_len = S4_H_PW;
		screen->s_upper_margin = S4_V_BP;
		screen->s_lower_margin = S4_V_FP;
		screen->s_vsync_len = S4_V_PW;
		screen->s_hsync_st = S4_H_ST;
#if(DEBUG_HDMI_SCREEN == HDMI_720X576P)
		screen->s_vsync_st = S4_V_ST + count;
		count++;
#else
		screen->s_vsync_st = S4_V_ST;
#endif
		break;
	case HDMI_720x480p_60Hz_16_9:
	case HDMI_720x480p_60Hz_4_3:
		/* Scaler Timing    */
		screen->hdmi_resolution = hdmi_resolution;
		screen->s_pixclock = S5_OUT_CLK;
		screen->s_hsync_len = S5_H_PW;
		screen->s_left_margin = S5_H_BP;
		screen->s_right_margin = S5_H_FP;
		screen->s_hsync_len = S5_H_PW;
		screen->s_upper_margin = S5_V_BP;
		screen->s_lower_margin = S5_V_FP;
		screen->s_vsync_len = S5_V_PW;
		screen->s_hsync_st = S5_H_ST;
#if(DEBUG_HDMI_SCREEN == HDMI_720X480P)
		screen->s_hsync_st = S5_H_PW+count;
		screen->s_vsync_st = S5_V_ST;
		count++;
#else
		screen->s_vsync_st = S5_V_ST;
#endif
		break;
	default :
		printk("%s lcd not support dual display at this hdmi resolution %d \n",__func__,hdmi_resolution);
		return -1;
		break;
	}
	
	printk("[gll]count = %d\n",count);
	return 0;
}
#else
static int set_scaler_info(struct rk29fb_screen *screen, u8 hdmi_resolution){return 0;}
#endif

void set_lcd_info(struct rk29fb_screen *screen,  struct rk29lcd_info *lcd_info )
{
	/* screen type & face */
	screen->type = OUT_TYPE;
	screen->face = OUT_FACE;

	/* Screen size */
	screen->x_res = H_VD;
	screen->y_res = V_VD;

	screen->width = LCD_WIDTH;
	screen->height = LCD_HEIGHT;

	/* Timing */
	screen->lcdc_aclk = LCDC_ACLK;
	screen->pixclock = OUT_CLK;
	screen->left_margin = H_BP;
	screen->right_margin = H_FP;
	screen->hsync_len = H_PW;
	screen->upper_margin = V_BP;
	screen->lower_margin = V_FP;
	screen->vsync_len = V_PW;

	/* Pin polarity */
	screen->pin_hsync = HSYNC_POL;
	screen->pin_vsync = VSYNC_POL;
	screen->pin_den = DEN_POL;
	screen->pin_dclk = DCLK_POL;

	/* Swap rule */
	screen->swap_rb = SWAP_RB;
	screen->swap_rg = SWAP_RG;
	screen->swap_gb = SWAP_GB;
	screen->swap_delta = 0;
	screen->swap_dumy = 0;

	/* Operation function*/
	screen->init = NULL;
	screen->standby = NULL;
	screen->sscreen_get = set_scaler_info;
#ifdef CONFIG_RK610_LVDS
    screen->sscreen_set = rk610_lcd_scaler_set_param;
#endif
}

size_t get_fb_size(void)
{
	size_t size = 0;
	#if defined(CONFIG_THREE_FB_BUFFER)
		size = ((H_VD)*(V_VD)<<2)* 3; //three buffer
	#else
		size = ((H_VD)*(V_VD)<<2)<<1; //two buffer
	#endif
	return ALIGN(size,SZ_1M);
}

