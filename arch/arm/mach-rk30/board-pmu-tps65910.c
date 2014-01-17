#include <linux/regulator/machine.h>
#include <linux/i2c/twl.h>
#include <linux/mfd/tps65910.h>
#include <mach/sram.h>
#include <linux/platform_device.h>

#include <mach/gpio.h>
#include <mach/iomux.h>

#ifdef CONFIG_MFD_TPS65910

extern int platform_device_register(struct platform_device *pdev);

int tps65910_pre_init(struct tps65910 *tps65910){

	int val = 0;
	int i 	= 0;
	int err = -1;
		
	printk("%s,line=%d\n", __func__,__LINE__);	
	
	#ifdef CONFIG_RK_CONFIG
	if(sram_gpio_init(get_port_config(pmic_slp).gpio, &pmic_sleep) < 0){
		printk(KERN_ERR "sram_gpio_init failed\n");
		return -EINVAL;
	}
	if(port_output_init(pmic_slp, 0, "pmic_slp") < 0){
		printk(KERN_ERR "port_output_init failed\n");
		return -EINVAL;
	}
	#else
	if(sram_gpio_init(PMU_POWER_SLEEP, &pmic_sleep) < 0){
		printk(KERN_ERR "sram_gpio_init failed\n");
		return -EINVAL;
	}

	gpio_request(PMU_POWER_SLEEP, "NULL");
	gpio_direction_output(PMU_POWER_SLEEP, GPIO_LOW);
	#endif

#if 0
	/*************set vdd11 (pll) voltage 1.0v********************/
	val = tps65910_reg_read(tps65910, TPS65910_VDIG2);
	if (val<0) {
		printk(KERN_ERR "Unable to read TPS65910_VDIG2 reg\n");
		return val;
	}
	val &= (~(0x3<<2));
	err = tps65910_reg_write(tps65910, TPS65910_VDIG2, val);
	if (err) {
		printk(KERN_ERR "Unable to write TPS65910_VDIG2 reg\n");
		return err;
	}
	/****************************************/
#endif	
	val = tps65910_reg_read(tps65910, TPS65910_DEVCTRL2);
	if (val<0) {
		printk(KERN_ERR "Unable to read TPS65910_DEVCTRL2 reg\n");
		return val;
	}
	/* Set sleep state active high and allow device turn-off after PWRON long press */
	val |= (DEVCTRL2_SLEEPSIG_POL_MASK | DEVCTRL2_PWON_LP_OFF_MASK);

	err = tps65910_reg_write(tps65910, TPS65910_DEVCTRL2, val);
	if (err) {
		printk(KERN_ERR "Unable to write TPS65910_DEVCTRL2 reg\n");
		return err;
	}
	
	 #if 1
	/* set PSKIP=0 */
        val = tps65910_reg_read(tps65910, TPS65910_DCDCCTRL);
        if (val<0) {
                printk(KERN_ERR "Unable to read TPS65910_DCDCCTRL reg\n");
                return val;
        }

	val &= ~DEVCTRL_DEV_OFF_MASK;
	val &= ~DEVCTRL_DEV_SLP_MASK;
        err = tps65910_reg_write(tps65910, TPS65910_DCDCCTRL, val);
        if (err) {
                printk(KERN_ERR "Unable to write TPS65910_DCDCCTRL reg\n");
                return err;
        }
	#endif
	/* Set the maxinum load current */
	/* VDD1 */
	val = tps65910_reg_read(tps65910, TPS65910_VDD1);
	if (val<0) {
		printk(KERN_ERR "Unable to read TPS65910_VDD1 reg\n");
		return val;
	}

	val |= (1<<5);		//when 1: 1.5 A
	val &= (~(0x3 <<2));
	val |= (0x01<<2);	//TSTEP[3:2] = 01 : 12.5 mV/us(sampling 3 Mhz)
	err = tps65910_reg_write(tps65910, TPS65910_VDD1, val);
	if (err) {
		printk(KERN_ERR "Unable to write TPS65910_VDD1 reg\n");
		return err;
	}

	/* VDD2 */
	val = tps65910_reg_read(tps65910, TPS65910_VDD2);
	if (val<0) {
		printk(KERN_ERR "Unable to read TPS65910_VDD2 reg\n");
		return val;
	}

	val |= (1<<5);		//when 1: 1.5 A
	val &= (~(0x3 <<2));
	val |= (0x01<<2);	//TSTEP[3:2] = 01 : 12.5 mV/us(sampling 3 Mhz)
	err = tps65910_reg_write(tps65910, TPS65910_VDD2, val);
	if (err) {
		printk(KERN_ERR "Unable to write TPS65910_VDD2 reg\n");
		return err;
	}

	/* VIO */
	val = tps65910_reg_read(tps65910, TPS65910_VIO);
	if (val<0) {
		printk(KERN_ERR "Unable to read TPS65910_VIO reg\n");
		return -EIO;
	}

	val |= (1<<6);	//when 01: 1.0 A
	err = tps65910_reg_write(tps65910, TPS65910_VIO, val);
	if (err) {
		printk(KERN_ERR "Unable to write TPS65910_VIO reg\n");
		return err;
	}
	#if 1
	/* Mask ALL interrupts */
	err = tps65910_reg_write(tps65910,TPS65910_INT_MSK, 0xFF);
	if (err) {
		printk(KERN_ERR "Unable to write TPS65910_INT_MSK reg\n");
		return err;
	}
	
	err = tps65910_reg_write(tps65910, TPS65910_INT_MSK2, 0x03);
	if (err) {
		printk(KERN_ERR "Unable to write TPS65910_INT_MSK2 reg\n");
		return err;
	}

	/* Set RTC Power, disable Smart Reflex in DEVCTRL_REG */
	val = 0;
	val |= (DEVCTRL_SR_CTL_I2C_SEL_MASK);
	err = tps65910_reg_write(tps65910, TPS65910_DEVCTRL, val);
	if (err) {
		printk(KERN_ERR "Unable to write TPS65910_DEVCTRL reg\n");
		return err;
	}
	printk(KERN_INFO "TPS65910 Set disable Smart Reflex in DEVCTRL_REG.\n");
	#if 0
	//read sleep control register  for debug
	for(i=0; i<6; i++)
	{
        err = tps65910_reg_read(tps65910, &val, TPS65910_DEVCTRL+i);
        if (err) {
                printk(KERN_ERR "Unable to read TPS65910_DCDCCTRL reg\n");
                return -EIO;
        }
		else
		printk("%s.......is  0x%04x\n",__FUNCTION__,val);
	}
	#endif

	#if 1
	#if !defined(CONFIG_MFD_TRS65910)	//for trs65910 Trsilicon peter
	//sleep control register
	/*set func when in sleep mode */
	val = tps65910_reg_read(tps65910, TPS65910_DEVCTRL);
	if (val<0) {
		printk(KERN_ERR "Unable to read TPS65910_DEVCTRL reg\n");
		return val;
	}

	val |= (1 << 1);
	err = tps65910_reg_write(tps65910, TPS65910_DEVCTRL, val);
	if (err) {
		printk(KERN_ERR "Unable to read TPS65910 Reg at offset 0x%x= \n", TPS65910_DEVCTRL);
		return err;
	}
	#endif

	/* open ldo when in sleep mode */
        val = tps65910_reg_read(tps65910, TPS65910_SLEEP_KEEP_LDO_ON);
        if (val<0) {
                printk(KERN_ERR "Unable to read TPS65910_DCDCCTRL reg\n");
                return val;
        }
	
	val &= 0;
	err = tps65910_reg_write(tps65910, TPS65910_SLEEP_KEEP_LDO_ON, val);
	if (err) {
		printk(KERN_ERR "Unable to read TPS65910 Reg at offset 0x%x= \n", TPS65910_SLEEP_KEEP_LDO_ON);
		return err;
	}
		
	/*set dc mode when in sleep mode */
        val = tps65910_reg_read(tps65910, TPS65910_SLEEP_KEEP_RES_ON);
        if (val<0) {
                printk(KERN_ERR "Unable to read TPS65910_SLEEP_KEEP_RES_ON reg\n");
                return val;
        }
	
	val  |= 0xff;
	val  &= ~(0x07);   //set vdd1 vdd2 vio in pfm mode when in sleep
	err = tps65910_reg_write(tps65910, TPS65910_SLEEP_KEEP_RES_ON, val);
	if (err) {
		printk(KERN_ERR "Unable to read TPS65910 Reg at offset 0x%x= \n", TPS65910_SLEEP_KEEP_RES_ON);
		return err;
	}
	
	/*close ldo when in sleep mode */
        val = tps65910_reg_read(tps65910, TPS65910_SLEEP_SET_LDO_OFF);
        if (val<0) {
                printk(KERN_ERR "Unable to read TPS65910_SLEEP_SET_LDO_OFF reg\n");
                return val;
        }
#if defined(CONFIG_MACH_RK3028_PHONEPAD_780)
	val |= 0x2a;//0b->0a, to remain codec-es8323 power 
#else
	val |= 0x0a;
#endif
	err = tps65910_reg_write(tps65910, TPS65910_SLEEP_SET_LDO_OFF, val);
	if (err) {
		printk(KERN_ERR "Unable to read TPS65910 Reg at offset 0x%x= \n", TPS65910_SLEEP_SET_LDO_OFF);
		return err;
	}
	#endif
	#if 0
	//read sleep control register  for debug
	for(i=0; i<6; i++)
	{
        err = tps65910_reg_read(tps65910, &val, TPS65910_DEVCTRL+i);
        if (err) {
                printk(KERN_ERR "Unable to read TPS65910_DCDCCTRL reg\n");
                return -EIO;
        }
		else
		printk("%s.......is  0x%4x\n",__FUNCTION__,val);
	}
	#endif
	#endif
	
	/*****************set arm  and logic (dc1&dc2)in pwm ****************/
	  val = tps65910_reg_read(tps65910, TPS65910_DCDCCTRL);
        if (val<0) {
                printk(KERN_ERR "Unable to read TPS65910_DCDCCTRL reg\n");
                return val;
        }
	
	val &= ~(3<<4);
	err = tps65910_reg_write(tps65910, TPS65910_DCDCCTRL, val);
	if (err) {
		printk(KERN_ERR "Unable to read TPS65910 Reg at offset 0x%x= \n", TPS65910_DCDCCTRL);
		return err;
	}	
	/************************************************/
	
	printk("%s,line=%d\n", __func__,__LINE__);
	return 0;

}
int tps65910_post_init(struct tps65910 *tps65910)
{
	struct regulator *dcdc;
	struct regulator *ldo;
	int i=0;
	printk("%s,line=%d\n", __func__,__LINE__);

	g_pmic_type = PMIC_TYPE_TPS65910;
	printk("%s:g_pmic_type=%d\n",__func__,g_pmic_type);

	#ifdef CONFIG_RK30_PWM_REGULATOR
	platform_device_register(&pwm_regulator_device[0]);
	#endif

	for(i = 0; i < ARRAY_SIZE(tps65910_dcdc_info); i++)
	{
	dcdc =regulator_get(NULL, tps65910_dcdc_info[i].name);
	regulator_set_voltage(dcdc, tps65910_dcdc_info[i].min_uv, tps65910_dcdc_info[i].max_uv);
	regulator_enable(dcdc);
	printk("%s  %s =%dmV end\n", __func__,tps65910_dcdc_info[i].name, regulator_get_voltage(dcdc));
	regulator_put(dcdc);
	udelay(100);
	}
	
	for(i = 0; i < ARRAY_SIZE(tps65910_ldo_info); i++)
	{
	ldo =regulator_get(NULL, tps65910_ldo_info[i].name);
	regulator_set_voltage(ldo, tps65910_ldo_info[i].min_uv, tps65910_ldo_info[i].max_uv);
	regulator_enable(ldo);
	//printk("%s  %s =%dmV end\n", __func__,tps65910_dcdc_info[i].name, regulator_get_voltage(ldo));
	regulator_put(ldo);
	}

	printk("%s,line=%d END\n", __func__,__LINE__);
	
	return 0;
}

static struct regulator_consumer_supply tps65910_smps1_supply[] = {
	{
		.supply = "vdd1",
	},
	#if defined(CONFIG_MACH_RK3028_PHONEPAD_760) || defined(CONFIG_MACH_RK3028_PHONEPAD_780)
	{
		.supply = "vdd_cpu",
	}
	#elif defined(CONFIG_SOC_RK3168) || defined(CONFIG_ARCH_RK3188) || defined(CONFIG_SOC_RK3028) || defined(CONFIG_MACH_RK_FAC)
	{
		.supply = "vdd_core",
	},
	#else
	{
		.supply = "vdd_cpu",
	},
	#endif
};
static struct regulator_consumer_supply tps65910_smps2_supply[] = {
	{
		.supply = "vdd2",
	},
	#if defined(CONFIG_MACH_RK3168_86V)||defined(CONFIG_MACH_RK_FAC)
	{
                .supply = "vdd_cpu",
    },
	#endif
    #if defined(CONFIG_MACH_RK3028_86V) 
    {
                .supply = "vdd_cpu",
        },
	#endif
	#if defined(CONFIG_MACH_RK3028_PHONEPAD_760) || defined(CONFIG_MACH_RK3028_PHONEPAD_780)
	{
				.supply = "vdd_core",
	}
	#endif
};
static struct regulator_consumer_supply tps65910_smps3_supply[] = {
	{
		.supply = "vdd3",
	},
};
static struct regulator_consumer_supply tps65910_smps4_supply[] = {
	{
		.supply = "vio",
	},
};
static struct regulator_consumer_supply tps65910_ldo1_supply[] = {
	{
		.supply = "vdig1",
	},
};
static struct regulator_consumer_supply tps65910_ldo2_supply[] = {
	{
		.supply = "vdig2",
	},
};

static struct regulator_consumer_supply tps65910_ldo3_supply[] = {
	{
		.supply = "vaux1",
	},
};
static struct regulator_consumer_supply tps65910_ldo4_supply[] = {
	{
		.supply = "vaux2",
	},
};
static struct regulator_consumer_supply tps65910_ldo5_supply[] = {
	{
		.supply = "vaux33",
	},
};
static struct regulator_consumer_supply tps65910_ldo6_supply[] = {
	{
		.supply = "vmmc",
	},
};
static struct regulator_consumer_supply tps65910_ldo7_supply[] = {
	{
		.supply = "vdac",
	},
};

static struct regulator_consumer_supply tps65910_ldo8_supply[] = {
	{
		.supply = "vpll",
	},
};

static struct regulator_init_data tps65910_smps1 = {
	.constraints = {
		.name           = "VDD1",
		.min_uV			= 600000,
		.max_uV			= 1500000,
		.apply_uV		= 1,
		.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(tps65910_smps1_supply),
	.consumer_supplies =  tps65910_smps1_supply,
};

/* */
static struct regulator_init_data tps65910_smps2 = {
	.constraints = {
		.name           = "VDD2",
		.min_uV			= 600000,
		.max_uV			= 1500000,
		.apply_uV		= 1,
		.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(tps65910_smps2_supply),
	.consumer_supplies =  tps65910_smps2_supply,
};

/* */
static struct regulator_init_data tps65910_smps3 = {
	.constraints = {
		.name           = "VDD3",
		.min_uV			= 1000000,
		.max_uV			= 1400000,
		.apply_uV		= 1,
		.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(tps65910_smps3_supply),
	.consumer_supplies =  tps65910_smps3_supply,
};

static struct regulator_init_data tps65910_smps4 = {
	.constraints = {
		.name           = "VIO",
		.min_uV			= 1800000,
		.max_uV			= 3300000,
		.apply_uV		= 1,
		.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(tps65910_smps4_supply),
	.consumer_supplies =  tps65910_smps4_supply,
};
static struct regulator_init_data tps65910_ldo1 = {
	.constraints = {
		.name           = "VDIG1",
		.min_uV			= 1200000,
		.max_uV			= 2700000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(tps65910_ldo1_supply),
	.consumer_supplies =  tps65910_ldo1_supply,
};

/* */
static struct regulator_init_data tps65910_ldo2 = {
	.constraints = {
		.name           = "VDIG2",
		.min_uV			= 1000000,
		.max_uV			= 1800000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(tps65910_ldo2_supply),
	.consumer_supplies =  tps65910_ldo2_supply,
};

/* */
static struct regulator_init_data tps65910_ldo3 = {
	.constraints = {
		.name           = "VAUX1",
		.min_uV			= 1800000,
		.max_uV			= 3300000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(tps65910_ldo3_supply),
	.consumer_supplies =  tps65910_ldo3_supply,
};

/* */
static struct regulator_init_data tps65910_ldo4 = {
	.constraints = {
		.name           = "VAUX2",
		.min_uV			= 1800000,
		.max_uV			= 3300000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(tps65910_ldo4_supply),
	.consumer_supplies =  tps65910_ldo4_supply,
};

/* */
static struct regulator_init_data tps65910_ldo5 = {
	.constraints = {
		.name           = "VAUX33",
		.min_uV			= 1800000,
		.max_uV			= 3300000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(tps65910_ldo5_supply),
	.consumer_supplies =  tps65910_ldo5_supply,
};

/* */
static struct regulator_init_data tps65910_ldo6 = {
	.constraints = {
		.name           = "VMMC",
		.min_uV			= 1800000,
		.max_uV			= 3300000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(tps65910_ldo6_supply),
	.consumer_supplies =  tps65910_ldo6_supply,
};

/* */
static struct regulator_init_data tps65910_ldo7 = {
	.constraints = {
		.name           = "VDAC",
		.min_uV			= 1800000,
		.max_uV			= 2850000,
		.apply_uV		= 1,
		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(tps65910_ldo7_supply),
	.consumer_supplies =  tps65910_ldo7_supply,
};

/* */
static struct regulator_init_data tps65910_ldo8 = {
	.constraints = {
		.name           = "VPLL",
		.min_uV			= 1000000,
		.max_uV			= 2500000,
		.apply_uV		= 1,
//		.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,

	},
	.num_consumer_supplies = ARRAY_SIZE(tps65910_ldo8_supply),
	.consumer_supplies =  tps65910_ldo8_supply,
};

void __sramfunc board_pmu_tps65910_suspend(void)
{	
	#ifdef CONFIG_CLK_SWITCH_TO_32K
	 sram_gpio_set_value(pmic_sleep, GPIO_HIGH);  
	#endif
}
void __sramfunc board_pmu_tps65910_resume(void)
{
	#ifdef CONFIG_CLK_SWITCH_TO_32K
 	sram_gpio_set_value(pmic_sleep, GPIO_LOW);  
	sram_32k_udelay(10000);
	#endif
}
static struct tps65910_board tps65910_data = {
	.irq 	= (unsigned)TPS65910_HOST_IRQ,		
	.irq_base = IRQ_BOARD_BASE,
	.gpio_base = TPS65910_GPIO_EXPANDER_BASE,
	
	.pre_init = tps65910_pre_init,
	.post_init = tps65910_post_init,

	//TPS65910_NUM_REGS = 13
	// Regulators
	.tps65910_pmic_init_data[TPS65910_REG_VRTC] = NULL,		
	.tps65910_pmic_init_data[TPS65910_REG_VIO] = &tps65910_smps4,
	.tps65910_pmic_init_data[TPS65910_REG_VDD1] = &tps65910_smps1,
	.tps65910_pmic_init_data[TPS65910_REG_VDD2] = &tps65910_smps2,
	.tps65910_pmic_init_data[TPS65910_REG_VDD3] = &tps65910_smps3,
	.tps65910_pmic_init_data[TPS65910_REG_VDIG1] = &tps65910_ldo1,
	.tps65910_pmic_init_data[TPS65910_REG_VDIG2] = &tps65910_ldo2,
	.tps65910_pmic_init_data[TPS65910_REG_VPLL] = &tps65910_ldo8,
	.tps65910_pmic_init_data[TPS65910_REG_VDAC] = &tps65910_ldo7,
	.tps65910_pmic_init_data[TPS65910_REG_VAUX1] = &tps65910_ldo3,
	.tps65910_pmic_init_data[TPS65910_REG_VAUX2] = &tps65910_ldo4,
	.tps65910_pmic_init_data[TPS65910_REG_VAUX33] = &tps65910_ldo5,
	.tps65910_pmic_init_data[TPS65910_REG_VMMC] = &tps65910_ldo6,

 
};

#endif

