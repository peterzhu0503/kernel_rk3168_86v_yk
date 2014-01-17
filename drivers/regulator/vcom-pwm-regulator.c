/*
 *
 * Copyright (C) 2013 ROCKCHIP, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/bug.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/rk29-pwm-regulator.h>
#include <mach/iomux.h>
#include <linux/gpio.h>
#include <mach/board.h>
#include <plat/pwm.h>

#if 0
#define DBG(x...)	printk(KERN_INFO x)
#else
#define DBG(x...)
#endif

struct rk_pwm_dcdc {
        char name[16];
        struct regulator_desc desc;
        int pwm_id;
        struct clk *pwm_clk;
        const void __iomem *pwm_base;
        u32 suspend_hrc;
        u32 suspend_lrc;
        u32 backup_hrc;
        u32 backup_lrc;
        struct regulator_dev *regulator;
	struct pwm_platform_data *pdata;
};

#if defined(CONFIG_SOC_RK3168) || defined(CONFIG_ARCH_RK3188)
const static int pwm_voltage_map[] = {
	800000,825000,850000, 875000,900000, 925000 ,950000, 975000,1000000, 1025000, 1050000, 1075000, 1100000, 1125000, 1150000, 1175000, 1200000, 1225000, 1250000, 1275000, 1300000, 1325000, 1350000,1375000
};
#else
const static int pwm_voltage_map[] = {
	950000, 975000,1000000, 1025000, 1050000, 1075000, 1100000, 1125000, 1150000, 1175000, 1200000, 1225000, 1250000, 1275000, 1300000, 1325000, 1350000, 1375000, 1400000
};
#endif

static struct rk_pwm_dcdc *g_dcdc;

static int pwm_set_rate(struct pwm_platform_data *pdata,int nHz,u32 rate)
{
	u32 lrc, hrc;
	int id = pdata->pwm_id;
	unsigned long clkrate;

	clkrate = clk_get_rate(g_dcdc->pwm_clk);
	
	DBG("%s:id=%d,rate=%d,clkrate=%d\n",__func__,id,rate,clkrate); 

	if(rate == 0)
	{
		// iomux pwm to gpio
		rk29_mux_api_set(pdata->pwm_iomux_name, pdata->pwm_iomux_gpio);
		//disable pull up or down
		gpio_pull_updown(pdata->pwm_gpio,PullDisable);
		// set gpio to low level
		gpio_direction_output(pdata->pwm_gpio,GPIO_LOW);
	}
	else if (rate < 100)
	{
		lrc = clkrate / nHz;
		lrc = lrc >> (1+(PWM_DIV>>9));
		lrc = lrc ? lrc : 1;
		hrc = lrc * rate / 100;
		hrc = hrc ? hrc : 1;

		// iomux pwm
		rk29_mux_api_set(pdata->pwm_iomux_name, pdata->pwm_iomux_pwm);

		rk_pwm_setup(id, PWM_DIV, hrc, lrc);
	}
	else if (rate == 100)
	{
		// iomux pwm to gpio
		rk29_mux_api_set(pdata->pwm_iomux_name, pdata->pwm_iomux_gpio);
		//disable pull up or down
		gpio_pull_updown(pdata->pwm_gpio,PullDisable);
		// set gpio to low level
		gpio_direction_output(pdata->pwm_gpio,GPIO_HIGH);

	}
	else
	{
		printk("%s:rate error\n",__func__);
		return -1;
	}

	usleep_range(10*1000, 10*1000);

	return (0);
}

static int pwm_regulator_list_voltage(struct regulator_dev *dev,unsigned int index)
{
	struct rk_pwm_dcdc *dcdc = rdev_get_drvdata(dev);
	if (index < dcdc->desc.n_voltages)
	return dcdc->pdata->pwm_voltage_map[index];
	else
		return -1;
}

static int pwm_regulator_is_enabled(struct regulator_dev *dev)
{
	DBG("Enter %s\n",__FUNCTION__);
	return 0;
}

static int pwm_regulator_enable(struct regulator_dev *dev)
{
	DBG("Enter %s\n",__FUNCTION__);
	return 0;
}

static int pwm_regulator_disable(struct regulator_dev *dev)
{
	DBG("Enter %s\n",__FUNCTION__);
	return 0;
}

static int pwm_regulator_get_voltage(struct regulator_dev *dev)
{
	//struct pwm_platform_data *pdata = rdev_get_drvdata(dev);
	
	struct rk_pwm_dcdc *dcdc = rdev_get_drvdata(dev);

	DBG("Enter %s\n",__FUNCTION__);  

	return (dcdc->pdata->pwm_voltage);
}

static int pwm_regulator_set_voltage(struct regulator_dev *dev,
		int min_uV, int max_uV, unsigned *selector)
{	   
	struct rk_pwm_dcdc *dcdc = rdev_get_drvdata(dev);
	const int *voltage_map = dcdc->pdata->pwm_voltage_map;
	int max = dcdc->pdata->max_uV;
	int coefficient = dcdc->pdata->coefficient;
	u32 size = dcdc->desc.n_voltages, i, vol,pwm_value;

	DBG("%s:  min_uV = %d, max_uV = %d\n",__FUNCTION__, min_uV,max_uV);

	if (min_uV < voltage_map[0] ||max_uV > voltage_map[size-1])
	{
		printk("%s:voltage is out of table\n",__func__);
		return -EINVAL;
	}

	for (i = 0; i < size; i++)
	{
		if (voltage_map[i] >= min_uV)
			break;
	}


	vol =  voltage_map[i];

	dcdc->pdata->pwm_voltage = vol;

	#ifdef	CONFIG_INCAR_PNLVCOM_ADJUST ///stephenchan 20130426  for PWM1 for lcd vcom adjust
	/*
	//pwm_value=1; ///2.76v
	//pwm_value=99; ///4.49v
	*/
#ifdef CONFIG_LCD_VCOM_VALUE
			#define VCOM_VALUE          (CONFIG_LCD_VCOM_VALUE)
			pwm_value=VCOM_VALUE;
#else

		#if	defined(CONFIG_LCD_VCOM2V8)  
				pwm_value=1;		//VCOM=2.76V
		#elif	defined(CONFIG_LCD_XY81207X768G_VCOM3V2)  
				pwm_value=29;		//VCOM=3.2V
		#elif defined(CONFIG_LCD_JDF50P_VCOM3V3)    
				pwm_value=35;		//VCOM=3.3V
		#elif defined(CONFIG_LCD_XY8800X600_VCOM3V4)       
				pwm_value=40;		//VCOM=3.38V
		#elif defined(CONFIG_LCD_HC750P_VCOM3V5)     	
				pwm_value=45;		//VCOM=3.47V
		#elif defined(CONFIG_LCD_QC750P_VCOM3V6)     	
				pwm_value=50;		//VCOM=3.58V
		#elif defined(CONFIG_LCD_TM950P_VCOM3V7)           
				pwm_value=55;		//VCOM=3.68V
		#elif defined(CONFIG_LCD_750P_VCOM3V8)           
				pwm_value=60;		//VCOM=3.78V
		#elif defined(CONFIG_LCD_750P_VCOM3V9)    	
				pwm_value=63;		//VCOM=3.9V
		#elif defined(CONFIG_LCD_750P_VCOM4V0)    	
				pwm_value=73;		//VCOM=4.02V
		#elif defined(CONFIG_LCD_750P_VCOM4V1)    	
				pwm_value=79;		//VCOM=4.12V
		#elif defined(CONFIG_LCD_CPT50P_VCOM4V2)    	
				pwm_value=85;		//VCOM=4.22V
		#elif defined(CONFIG_LCD_50P_VCOM4V5)    	
				pwm_value=99;		//VCOM=4.49V			
		#else
				pwm_value=99;		//VCOM=4.55V
		#endif

//		#if !defined(CONFIG_INCAR_PNLAVDD_OUTPUT_HIGH)	
//			if((pwm_value!=1)||(pwm_value!=99))
//				pwm_value-=24;
//		#endif
#endif
	

	#else
	// VDD12 = 1.40 - 0.455*D , 其中D为PWM占空比, 
	pwm_value = (max-vol)/coefficient/10;  // pwm_value %, coefficient *1000
	#endif

//			pwm_value=50;		//VCOM=4.55V


	if (pwm_set_rate(dcdc->pdata,1000*1000,pwm_value)!=0)
	{
		printk("%s:fail to set pwm rate,pwm_value=%d\n",__func__,pwm_value);
		return -1;

	}

	*selector = i;

	printk("stephen for pwm1 output ****** %s:ok,vol=%d,pwm_value=%d\n",__FUNCTION__,vol,pwm_value);

	return 0;

}

static struct regulator_ops pwm_voltage_ops = {
	.list_voltage	= pwm_regulator_list_voltage,
	.set_voltage	=pwm_regulator_set_voltage,
	.get_voltage	= pwm_regulator_get_voltage,
	.enable		= pwm_regulator_enable,
	.disable	= pwm_regulator_disable,
	.is_enabled	= pwm_regulator_is_enabled,
};

static int __devinit pwm_regulator_probe(struct platform_device *pdev)
{
	struct pwm_platform_data *pdata = pdev->dev.platform_data;
	struct rk_pwm_dcdc *dcdc;
	int pwm_id  =  pdata->pwm_id;
	int id = pdev->id;
	int ret ;
    	char gpio_name[20];
	unsigned selector = 0;
	
	if (!pdata)
		return -ENODEV;

	#ifdef	CONFIG_INCAR_PNLVCOM_ADJUST ///stephenchan 20130508  for P2B1 as AVDD control
		#ifdef	CONFIG_INCAR_PNLAVDD_OUTPUT_HIGH
		if(gpio_request(RK30_PIN0_PD4,NULL) != 0){
			gpio_free(RK30_PIN0_PD4);
			printk("RK30_PIN0_PD4  gpio_request error\n");
			return -EIO;
		}
		gpio_direction_output(RK30_PIN0_PD4, GPIO_LOW);
		gpio_set_value(RK30_PIN0_PD4,GPIO_HIGH);
		printk("RK30_PIN0_PD4  stephen output HIGH ***************************************\n");
		#else
		if(gpio_request(RK30_PIN0_PD4,NULL) != 0){
			gpio_free(RK30_PIN0_PD4);
			printk("RK30_PIN0_PD4  gpio_request error\n");
			return -EIO;
		}
		gpio_direction_output(RK30_PIN0_PD4, GPIO_LOW);
		gpio_set_value(RK30_PIN0_PD4,GPIO_LOW);
		printk("RK30_PIN0_PD4  stephen output LOW ***************************************\n");
		#endif

	#endif

	if (!pdata->pwm_voltage)
		pdata->pwm_voltage = 1100000;	// default 1.1v

	if(!pdata->pwm_voltage_map)
		pdata->pwm_voltage_map = pwm_voltage_map;

	if(!pdata->max_uV)
		pdata->max_uV = 1400000;

	if(!pdata->min_uV)
		pdata->min_uV = 1000000;
	
	if(pdata->suspend_voltage < pdata->min_uV)
		pdata->suspend_voltage = pdata->min_uV;
	
	if(pdata->suspend_voltage > pdata->max_uV)	
		pdata->suspend_voltage = pdata->max_uV;
	
	dcdc = kzalloc(sizeof(struct rk_pwm_dcdc), GFP_KERNEL);
	if (dcdc == NULL) {
		dev_err(&pdev->dev, "Unable to allocate private data\n");
		return -ENOMEM;
	}

	snprintf(dcdc->name, sizeof(dcdc->name), "PWM_DCDC%d", id + 1);
	dcdc->desc.name = dcdc->name;
	dcdc->desc.id = id;
	dcdc->desc.type = REGULATOR_VOLTAGE;
	dcdc->desc.n_voltages = ARRAY_SIZE(pwm_voltage_map);
	dcdc->desc.ops = &pwm_voltage_ops;
	dcdc->desc.owner = THIS_MODULE;
	dcdc->pdata = pdata;
	printk("%s:n_voltages=%d\n",__func__,dcdc->desc.n_voltages);
	dcdc->regulator = regulator_register(&dcdc->desc, &pdev->dev,
					     pdata->init_data, dcdc);
	if (IS_ERR(dcdc->regulator)) {
		ret = PTR_ERR(dcdc->regulator);
		dev_err(&pdev->dev, "Failed to register PWM_DCDC%d: %d\n",
			id + 1, ret);
		goto err;
	}

	snprintf(gpio_name, sizeof(gpio_name), "PWM_DCDC%d", id + 1);
	ret = gpio_request(pdata->pwm_gpio,gpio_name);
	if (ret) {
		dev_err(&pdev->dev,"failed to request pwm gpio\n");
		goto err_gpio;
	}

	dcdc->pwm_clk = rk_pwm_get_clk(pwm_id);
	dcdc->pwm_base = rk_pwm_get_base(pwm_id);
	if (IS_ERR(dcdc->pwm_clk)) {
		printk("pwm_clk get error %p\n", dcdc->pwm_clk);
		return -EINVAL;
	}
	clk_enable(dcdc->pwm_clk);

	dcdc->suspend_lrc = 0x12;
	switch (pdata->suspend_voltage)
	{
	case 1000000:
	default:
		dcdc->suspend_hrc = 0x10;
		break;
	case 1050000:
		dcdc->suspend_hrc = 0x0e;
		break;
	case 1100000:
		dcdc->suspend_hrc = 0x0c;
		break;
	case 1150000:
		dcdc->suspend_hrc = 0x0a;
		break;
	}

	g_dcdc	= dcdc;
	platform_set_drvdata(pdev, dcdc);	
	printk(KERN_INFO "pwm_regulator.%d: driver initialized\n",id);
	pwm_regulator_set_voltage(dcdc->regulator,pdata->pwm_voltage,pdata->pwm_voltage,&selector);
	
	return 0;


err_gpio:
	gpio_free(pdata->pwm_gpio);
err:
	printk("%s:error\n",__func__);
	return ret;

}

void pwm_suspend_voltage(void)
{
	struct rk_pwm_dcdc *dcdc = g_dcdc;
	
	if(!dcdc)
		return;
	
	dcdc->backup_hrc = readl_relaxed(dcdc->pwm_base + PWM_REG_HRC);
	dcdc->backup_lrc = readl_relaxed(dcdc->pwm_base + PWM_REG_LRC);

	__rk_pwm_setup(dcdc->pwm_base, PWM_DIV, dcdc->suspend_hrc, dcdc->suspend_lrc);
}

void pwm_resume_voltage(void)
{
	struct rk_pwm_dcdc *dcdc = g_dcdc;	
	
	if(!dcdc)
		return;
	__rk_pwm_setup(dcdc->pwm_base, PWM_DIV, dcdc->backup_hrc, dcdc->backup_lrc);
}

static int __devexit pwm_regulator_remove(struct platform_device *pdev)
{
	struct pwm_platform_data *pdata = pdev->dev.platform_data;
	struct regulator_dev *rdev = platform_get_drvdata(pdev);

	regulator_unregister(rdev);
	gpio_free(pdata->pwm_gpio);

	return 0;
}

static struct platform_driver pwm_regulator_driver = {
	.driver = {
		.name = "pwm-voltage-regulator",
	},
	.remove = __devexit_p(pwm_regulator_remove),
};


static int __init pwm_regulator_module_init(void)
{
	return platform_driver_probe(&pwm_regulator_driver, pwm_regulator_probe);
}

static void __exit pwm_regulator_module_exit(void)
{
	platform_driver_unregister(&pwm_regulator_driver);
}

fs_initcall(pwm_regulator_module_init);
module_exit(pwm_regulator_module_exit);
MODULE_LICENSE("GPL");
