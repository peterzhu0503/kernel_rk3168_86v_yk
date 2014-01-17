/**
 * File: charge.c
 *
 * Copyright (C) 2013 Hi-level Technology Limited
 * David.Pu <david_pu@hi-levelhk.com; pujianfeng@gmail.com>
 *
 * 非充电模式，长按Power键: 判断低电压，电压过低显示低电，电压够则直接开机
 * boot_source=1, bootMode=BOOT_MODE_NORMAL, power supply status=POWER_SUPPLY_STATUS_UNKNOWN, capacity=100
 *
 * 非充电模式，短按Power键: 判断低电压，电压过低显示低电，电压够则直接开机
 * boot_source=2, bootMode=BOOT_MODE_NORMAL, power supply status=POWER_SUPPLY_STATUS_UNKNOWN, capacity=100
 *
 * 非充电模式，Reset: 直接关机
 * boot_source=2, bootMode=BOOT_MODE_NORMAL, power supply status=POWER_SUPPLY_STATUS_UNKNOWN, capacity=100
 *
 * 充电模式，长按Power键: 电池出错就直接开机。电池良好就判断低电压，电压过低显示充电，电压够则直接开机
 * boot_source=1, bootMode=BOOT_MODE_NORMAL, power supply status=POWER_SUPPLY_STATUS_CHARGING, capacity=100
 *
 * 充电模式，短按Power键: 电池出错就显示电池出错。电池良好就显示充电或电池满
 * boot_source=2, bootMode=BOOT_MODE_NORMAL, power supply status=POWER_SUPPLY_STATUS_CHARGING, capacity=100
 *
 * 充电模式，Reset: 电池出错就显示电池出错。电池良好就显示充电或电池满
 * boot_source=2, bootMode=BOOT_MODE_NORMAL, power supply status=POWER_SUPPLY_STATUS_CHARGING, capacity=100
 *
 * 充电模式，插入DC: 电池出错就显示电池出错。电池良好就显示充电或电池满
 * boot_source=2, bootMode=BOOT_MODE_NORMAL, power supply status=POWER_SUPPLY_STATUS_CHARGING, capacity=100
 *
 * 显示充电或电池满时，拔出DC立即关机，按下Power按键直接开机。
 */

#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/fb.h>
#include <linux/power_supply.h>
#include <linux/linux_logo.h>
#include <linux/rk_fb.h>
#include <mach/gpio.h>
#include <plat/key.h>

#ifdef CONFIG_M68K
#include <asm/setup.h>
#endif

#ifdef CONFIG_MIPS
#include <asm/bootinfo.h>
#endif

#define BATTERY_FAIL_VOLTAGE 2000000

#define LOGO_INDEX_BATTERY_PER0 	0
#define LOGO_INDEX_BATTERY_PER100 	10
#define LOGO_INDEX_BATTERY_FULL   	11
#define LOGO_INDEX_BATTERY_FAIL  	12
#define LOGO_INDEX_LOW_BATTERY		13

#define LOGO_DELAYWORK_INTERVAL		10	  // 间隔10毫秒
#define LOGO_UPDATE_SHOW_COUNT		500	  // 500毫秒更新显示一次
#define LOGO_POWEROFF_COUNT			15000 // 15秒关机
#define LOGO_POWERON_COUNT			2000  // 2秒开机

struct charge_logo_data *charge_logo;

extern int pwr_on_thrsd;
int __weak get_boot_source(void){}
extern void kernel_power_off(void);

static int nologo;
module_param(nologo, bool, 0);
MODULE_PARM_DESC(nologo, "Disables startup charge logo");

extern struct list_head rk_psy_head;

const struct linux_logo *logo_battery[] = {
	&logo_battery_per0_clut224,
	&logo_battery_per10_clut224,
	&logo_battery_per20_clut224,
	&logo_battery_per30_clut224,
	&logo_battery_per40_clut224,
	&logo_battery_per50_clut224,
	&logo_battery_per60_clut224,
	&logo_battery_per70_clut224,
	&logo_battery_per80_clut224,
	&logo_battery_per90_clut224,
	&logo_battery_per100_clut224,
	&logo_battery_full_clut224,
	&logo_battery_fail_clut224,
	&logo_low_battery_clut224,
};
static const int logo_battery_count = sizeof(logo_battery)/sizeof(struct linux_logo *);

extern int fb_get_color_depth(struct fb_var_screeninfo *var, struct fb_fix_screeninfo *fix);

int fb_init_charge_logo( struct fb_info *info )
{
	int depth = fb_get_color_depth(&info->var, &info->fix);
    struct linux_logo *logo = NULL;
    int i;

	if (info->flags & FBINFO_MISC_TILEBLITTING ||
	    info->flags & FBINFO_MODULE)
		return -1;

	if (info->fix.visual == FB_VISUAL_DIRECTCOLOR) {
		depth = info->var.blue.length;
		if (info->var.red.length < depth)
			depth = info->var.red.length;
		if (info->var.green.length < depth)
			depth = info->var.green.length;
	}

	if (info->fix.visual == FB_VISUAL_STATIC_PSEUDOCOLOR && depth > 4) {
		/* assume console colormap */
		depth = 4;
	}

	if ((nologo) || (depth < 8) || (depth >= 24))
		return -1;

	for (i = 0; i < logo_battery_count; i++) {
		logo = logo_battery[i];
  		logo->width = ((logo->data[0] << 8) + logo->data[1]);
		logo->height = ((logo->data[2] << 8) + logo->data[3]);
		logo->clutsize = logo->clut[0];
		logo->data += 4;
		logo->clut += 1;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(fb_init_charge_logo);

const struct linux_logo * fb_find_charge_logo( int index )
{
	if (index >= logo_battery_count)
		return NULL;

	return logo_battery[index];
}
EXPORT_SYMBOL_GPL(fb_find_charge_logo);

extern struct rk29_keys_platform_data rk29_keys_pdata;

static void charge_battery_status()
{
	union power_supply_propval val_status = { POWER_SUPPLY_STATUS_DISCHARGING };
	union power_supply_propval val_capacity = { 100 };
	union power_supply_propval val_voltage = { 4200000 };
	struct power_supply *psy;
	int online = 0;



	//if (rk_get_system_battery_status() != POWER_SUPPLY_TYPE_BATTERY)
	//	val_status.intval = POWER_SUPPLY_STATUS_CHARGING;

	//val_capacity.intval = rk_get_system_battery_capacity();



	list_for_each_entry(psy, &rk_psy_head, rk_psy_node)
	{
		if ((psy->type == POWER_SUPPLY_TYPE_MAINS) || (psy->type == POWER_SUPPLY_TYPE_USB)) {
			psy->get_property(psy, POWER_SUPPLY_PROP_ONLINE, &val_status);
			online += val_status.intval;
		} else if (psy->type == POWER_SUPPLY_TYPE_BATTERY) {
			psy->get_property(psy, POWER_SUPPLY_PROP_CAPACITY, &val_capacity);
			psy->get_property(psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &val_voltage);
		}
	}
	val_status.intval = (online >= 1) ? POWER_SUPPLY_STATUS_CHARGING : POWER_SUPPLY_STATUS_DISCHARGING;

	charge_logo->power_status = val_status.intval;
	charge_logo->battery_cap = val_capacity.intval;
	charge_logo->voltage_now = val_voltage.intval;
}

static int charge_check_power_key()
{
	int i;
	for (i = 0; i < rk29_keys_pdata.nbuttons; i++) {
		if (strcmp(rk29_keys_pdata.buttons[i].desc, "play") == 0 ) {
			if ((gpio_get_value(rk29_keys_pdata.buttons[i].gpio) ^
				rk29_keys_pdata.buttons[i].active_low)) {
				return 1;
			}
		}
	}
	return 0;
}

static void charge_logo_show( struct rk_fb_inf *fb_inf, int index )
{
	if (fb_prepare_charge_logo(fb_inf->fb[0], FB_ROTATE_UR, index)) {
		fb_set_cmap(&fb_inf->fb[0]->cmap, fb_inf->fb[0]);
		fb_show_charge_logo_sub(fb_inf->fb[0]);
		fb_inf->fb[0]->fbops->fb_pan_display(&(fb_inf->fb[0]->var), fb_inf->fb[0]);
	}
}

static inline int capacity_to_index( int capacity )
{
	return (capacity == 100) ? LOGO_INDEX_BATTERY_FULL : (LOGO_INDEX_BATTERY_PER0 + capacity/10);
}

static void charge_logo_delaywork_func(struct work_struct *work)
{
	struct rk_fb_inf *fb_inf = charge_logo->fb_inf;
	int powerkey_down = 0;

	mutex_lock(&charge_logo->mutex);
	charge_logo->poweroff_count--;
	if (charge_logo->poweroff_count == 0)
		kernel_power_off();

	if (charge_logo->show_interval == 0) {
		charge_battery_status();
		if (charge_logo->power_status == POWER_SUPPLY_STATUS_DISCHARGING)
			kernel_power_off();

		charge_logo_show(charge_logo->fb_inf, charge_logo->show_index);
		if (charge_logo->show_index != LOGO_INDEX_BATTERY_FULL) {
			charge_logo->show_index++;
			if (charge_logo->show_index > LOGO_INDEX_BATTERY_PER100)
				charge_logo->show_index = capacity_to_index(charge_logo->battery_cap);
		}
	}

	charge_logo->show_interval++;
	if (charge_logo->show_interval > (LOGO_UPDATE_SHOW_COUNT/LOGO_DELAYWORK_INTERVAL))
		charge_logo->show_interval = 0;

	powerkey_down = charge_check_power_key();
	if (charge_logo->powerkey_down) {
		if (powerkey_down) {
			charge_logo->powerkey_count++;
			if (charge_logo->powerkey_count > (LOGO_POWERON_COUNT/LOGO_DELAYWORK_INTERVAL)) {
				if(fb_prepare_logo(fb_inf->fb[0], FB_ROTATE_UR)) {
					/* Start display and show logo on boot */
					fb_set_cmap(&fb_inf->fb[0]->cmap, fb_inf->fb[0]);
					fb_show_logo(fb_inf->fb[0], FB_ROTATE_UR);
					fb_inf->fb[0]->fbops->fb_pan_display(&(fb_inf->fb[0]->var), fb_inf->fb[0]);
				}
				charge_logo->power_is_on = 1;
				mutex_unlock(&charge_logo->mutex);
				return;
			}
		} else {
			kernel_power_off();
		}
	} else {
		if (powerkey_down) {
			charge_logo->poweroff_count = (LOGO_POWEROFF_COUNT/LOGO_DELAYWORK_INTERVAL);
			charge_logo->powerkey_down = 1;
			charge_logo->powerkey_count = 0;
		}
	}

	schedule_delayed_work(&charge_logo->delaywork, msecs_to_jiffies(LOGO_DELAYWORK_INTERVAL));
	mutex_unlock(&charge_logo->mutex);
}

static void charge_logo_init_work( struct rk_fb_inf *fb_inf )
{
	charge_logo->show_index = capacity_to_index(charge_logo->battery_cap);
	charge_logo->show_interval = 0;
	charge_logo->poweroff_count = (LOGO_POWEROFF_COUNT/LOGO_DELAYWORK_INTERVAL);
	charge_logo->powerkey_down = 0;
	charge_logo->power_is_on = 0;
	charge_logo->fb_inf = fb_inf;
	fb_init_charge_logo(fb_inf->fb[0]);

	if (charge_logo->mode == CHARGE_LOGO_MODE_CHARGING) {
		INIT_DELAYED_WORK(&charge_logo->delaywork, charge_logo_delaywork_func);
		mutex_init(&charge_logo->mutex);
		schedule_delayed_work(&charge_logo->delaywork, msecs_to_jiffies(1));
	} else if (charge_logo->mode == CHARGE_LOGO_MODE_LOWER) {
		charge_logo_show(fb_inf, LOGO_INDEX_LOW_BATTERY);
	} else if (charge_logo->mode == CHARGE_LOGO_MODE_FAIL) {
		charge_logo_show(fb_inf, LOGO_INDEX_BATTERY_FAIL);
	}
}

static const char *bootModeName[] = {
	"BOOT_MODE_NORMAL",
	"BOOT_MODE_FACTORY2",
	"BOOT_MODE_RECOVERY",
	"BOOT_MODE_CHARGE",
	"BOOT_MODE_POWER_TEST",
	"BOOT_MODE_OFFMODE_CHARGING",
	"BOOT_MODE_REBOOT",
	"BOOT_MODE_PANIC",
};

static const char *powerSupplyStatusName[] = {
	"POWER_SUPPLY_STATUS_UNKNOWN",
	"POWER_SUPPLY_STATUS_CHARGING",
	"POWER_SUPPLY_STATUS_DISCHARGING",
	"POWER_SUPPLY_STATUS_NOT_CHARGING",
	"POWER_SUPPLY_STATUS_FULL",
};

static const char *chargeLogoModeName[] = {
	"CHARGE_LOGO_MODE_CONTINUE",
	"CHARGE_LOGO_MODE_CHARGING",
	"CHARGE_LOGO_MODE_LOWER",
	"CHARGE_LOGO_MODE_FAIL",
};

void fb_charge_logo( struct rk_fb_inf *fb_inf )
{
	int boot_source = 0;

	charge_logo = kzalloc(sizeof(struct charge_logo_data), GFP_KERNEL);
	if (!charge_logo) {
		printk("[logo_charge]: alloc data failed.\n");
		kernel_power_off();
	}

	if ((board_boot_mode() == BOOT_MODE_RECOVERY) ||
		(board_boot_mode() == BOOT_MODE_REBOOT)) {
		printk("recovery or reboot mode \n");
		charge_logo->mode = CHARGE_LOGO_MODE_CONTINUE;
		return;
	}

	boot_source = get_boot_source();
	charge_battery_status();
#if 1
	printk("stephen charge logo*****boot_source=%d, bootMode=%s, power supply status=%s, capacity=%d, voltage=%d\n",
		get_boot_source(), 
		bootModeName[board_boot_mode()], 
		powerSupplyStatusName[charge_logo->power_status], 
		charge_logo->battery_cap,
		charge_logo->voltage_now);
#endif

	if (charge_logo->power_status == POWER_SUPPLY_STATUS_CHARGING) {
	/* 充电模式 */
		if (boot_source == 1) {
		/* 长按Power键 */
			if ((charge_logo->battery_cap < pwr_on_thrsd) &&
				(charge_logo->voltage_now > BATTERY_FAIL_VOLTAGE)) {
				charge_logo->mode = CHARGE_LOGO_MODE_CHARGING;
				charge_logo_init_work(fb_inf);
			} else {
				charge_logo->mode = CHARGE_LOGO_MODE_CONTINUE;
			}
		} else if (boot_source == 2) {
		/* 短按Power键、Reset、插入DC */
			if (charge_logo->voltage_now < BATTERY_FAIL_VOLTAGE) {
				charge_logo->mode = CHARGE_LOGO_MODE_FAIL;
				charge_logo_init_work(fb_inf);
			} else {
				charge_logo->mode = CHARGE_LOGO_MODE_CHARGING;
				charge_logo_init_work(fb_inf);
			}
		}
	} else {
	/* 非充电模式 */
		if (boot_source == 1) {
		/* 长按Power键 */
			if (charge_logo->battery_cap < pwr_on_thrsd) {
				charge_logo->mode = CHARGE_LOGO_MODE_LOWER;
				charge_logo_init_work(fb_inf);
			} else {
				charge_logo->mode = CHARGE_LOGO_MODE_CONTINUE;
			}
		} else if (boot_source == 2) {
		/* 短按Power键、Reset */
			kernel_power_off();
			for(;;);
		}
	}
	printk("!!! PJF: charge_logo->mode=%s\n", chargeLogoModeName[charge_logo->mode]);
}
EXPORT_SYMBOL_GPL(fb_charge_logo);
