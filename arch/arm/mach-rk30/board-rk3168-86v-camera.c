#ifdef CONFIG_VIDEO_RK29

#include <plat/rk_camera.h>
/* Notes:

Simple camera device registration:

       new_camera_device(sensor_name,\       // sensor name, it is equal to CONFIG_SENSOR_X
                          face,\              // sensor face information, it can be back or front
                          pwdn_io,\           // power down gpio configuration, it is equal to CONFIG_SENSOR_POWERDN_PIN_XX
                          flash_attach,\      // sensor is attach flash or not
                          mir,\               // sensor image mirror and flip control information
                          i2c_chl,\           // i2c channel which the sensor attached in hardware, it is equal to CONFIG_SENSOR_IIC_ADAPTER_ID_X
                          cif_chl)  \         // cif channel which the sensor attached in hardware, it is equal to CONFIG_SENSOR_CIF_INDEX_X

Comprehensive camera device registration:

      new_camera_device_ex(sensor_name,\
                             face,\
                             ori,\            // sensor orientation, it is equal to CONFIG_SENSOR_ORIENTATION_X
                             pwr_io,\         // sensor power gpio configuration, it is equal to CONFIG_SENSOR_POWER_PIN_XX
                             pwr_active,\     // sensor power active level, is equal to CONFIG_SENSOR_RESETACTIVE_LEVEL_X
                             rst_io,\         // sensor reset gpio configuration, it is equal to CONFIG_SENSOR_RESET_PIN_XX
                             rst_active,\     // sensor reset active level, is equal to CONFIG_SENSOR_RESETACTIVE_LEVEL_X
                             pwdn_io,\
                             pwdn_active,\    // sensor power down active level, is equal to CONFIG_SENSOR_POWERDNACTIVE_LEVEL_X
                             flash_attach,\
                             res,\            // sensor resolution, this is real resolution or resoltuion after interpolate
                             mir,\
                             i2c_chl,\
                             i2c_spd,\        // i2c speed , 100000 = 100KHz
                             i2c_addr,\       // the i2c slave device address for sensor
                             cif_chl,\
                             mclk)\           // sensor input clock rate, 24 or 48
                          
*/

// add xiangqian fonxin@foxmail.com ***********************************
#define GC2035_B 1
#define GC0308_B 1
#define GC0329_B 1
#define GC0328_B 0
#define OV5640_B 1
#define SP2518_B 1
#define SP0838_B 1

#define GC2035_F 0
#define GC0308_F 1
#define GC0329_F 1
#define GC0328_F 0
#define OV5640_F 0
#define SP2518_F 0
#define SP0838_F 1

//#define FRONT_PIN_TO_BACK_PIN  //ǰ����ƽŻ���
#ifdef FRONT_PIN_TO_BACK_PIN
#define RK_CAMERA_BACK_PIN RK30_PIN3_PB4
#define RK_CAMERA_FRONT_PIN RK30_PIN3_PB5
#else
#define RK_CAMERA_BACK_PIN RK30_PIN3_PB5
#define RK_CAMERA_FRONT_PIN RK30_PIN3_PB4
#endif

static struct rkcamera_platform_data new_camera[] = { 
	#if GC2035_B
		new_camera_device(RK29_CAM_SENSOR_GC2035,
                        back,
                        RK_CAMERA_BACK_PIN,
                        0,
                        0,
                        3,
                        0),
        	#endif
        #if GC0308_B
        	new_camera_device(RK29_CAM_SENSOR_GC0308,
                        back,
                        RK_CAMERA_BACK_PIN,
                        0,
                        0,
                        3,
                        0),
        	#endif
        #if GC0329_B
              new_camera_device(RK29_CAM_SENSOR_GC0329,
                        back,
                        RK_CAMERA_BACK_PIN,
                        0,
                        0,
                        3,
                        0),
        	#endif
        #if GC0328_B
                new_camera_device(RK29_CAM_SENSOR_GC0328,
                        back,
                        RK_CAMERA_BACK_PIN,
                        0,
                        0,
                        3,
                        0), 
        	#endif
        #if OV5640_B
        	new_camera_device(RK29_CAM_SENSOR_OV5640,
                        back,
                        RK_CAMERA_BACK_PIN,
                        0,
                        0,
                        3,
                        0), 
        	#endif
        #if SP2518_B
              new_camera_device(RK29_CAM_SENSOR_SP2518,
                        back,
                        RK_CAMERA_BACK_PIN,
                        0,
                        0,
                        3,
                        0),
        	#endif
        #if SP0838_B
              new_camera_device(RK29_CAM_SENSOR_SP0838,
                        back,
                        RK_CAMERA_BACK_PIN,
                        0,
                        0,
                        3,
                        0),
        	#endif   
                  
 
   
  
    
   
                        
	#if GC2035_F
		new_camera_device(RK29_CAM_SENSOR_GC2035,
                        front,
                        RK_CAMERA_FRONT_PIN,
                        0,
                        0,
                        3,
                        0),
        	#endif
        #if GC0308_F
        	new_camera_device(RK29_CAM_SENSOR_GC0308,
                        front,
                        RK_CAMERA_FRONT_PIN,
                        0,
                        0,
                        3,
                        0), 
        	#endif
        #if GC0329_F
        	new_camera_device(RK29_CAM_SENSOR_GC0329,
                        front,
                        RK_CAMERA_FRONT_PIN,
                        0,
                        0,
                        3,
                        0), 
        	#endif
        #if GC0328_F
        	new_camera_device(RK29_CAM_SENSOR_GC0328,
                        front,
                        RK_CAMERA_FRONT_PIN,
                        0,
                        0,
                        3,
                        0), 
        	#endif
        #if OV5640_F
                new_camera_device(RK29_CAM_SENSOR_OV5640,
                        front,
                        RK_CAMERA_FRONT_PIN,
                        0,
                        0,
                        3,
                        0),
        	#endif
        #if SP2518_F
                new_camera_device(RK29_CAM_SENSOR_SP2518,
                        front,
                        RK_CAMERA_FRONT_PIN,
                        0,
                        0,
                        3,
                        0),
        	#endif
        #if SP0838_F
        	new_camera_device(RK29_CAM_SENSOR_SP0838,
                        front,
                        RK_CAMERA_FRONT_PIN,
                        0,
                        0,
                        3,
                        0),
        	#endif




    new_camera_device_end
};

//add xiangqian end***********************************************

#endif  //#ifdef CONFIG_VIDEO_RK29

/*---------------- Camera Sensor Configuration Macro End------------------------*/
#include "../../../drivers/media/video/rk30_camera.c"
/*---------------- Camera Sensor Macro Define End  ---------*/

#define PMEM_CAM_SIZE PMEM_CAM_NECESSARY
/*****************************************************************************************
 * camera  devices
 * author: ddl@rock-chips.com
 *****************************************************************************************/
#ifdef CONFIG_VIDEO_RK29
#define CONFIG_SENSOR_POWER_IOCTL_USR	   1 //define this refer to your board layout
#define CONFIG_SENSOR_RESET_IOCTL_USR	   0
#define CONFIG_SENSOR_POWERDOWN_IOCTL_USR	   0
#define CONFIG_SENSOR_FLASH_IOCTL_USR	   0

#if CONFIG_SENSOR_POWER_IOCTL_USR
static int sensor_power_usr_cb (struct rk29camera_gpio_res *res,int on)
{
	//#error "CONFIG_SENSOR_POWER_IOCTL_USR is 1, sensor_power_usr_cb function must be writed!!";
    struct regulator *ldo_18,*ldo_28;

    ldo_28 = regulator_get(NULL, "ldo7");	// vcc28_cif
    ldo_18 = regulator_get(NULL, "ldo1");	// vcc18_cif
    if (ldo_28 == NULL || IS_ERR(ldo_28) || ldo_18 == NULL || IS_ERR(ldo_18)){
        printk("get cif ldo failed!\n");
        return -1;
    }
    if(on == 0){
        while(regulator_is_enabled(ldo_28)>0)	
            regulator_disable(ldo_28);
        regulator_put(ldo_28);
        while(regulator_is_enabled(ldo_18)>0)
            regulator_disable(ldo_18);
        regulator_put(ldo_18);
        mdelay(10);
    } else {
        regulator_set_voltage(ldo_28, 2800000, 2800000);
        regulator_enable(ldo_28);
        //printk("%s set ldo7 vcc28_cif=%dmV end\n", __func__, regulator_get_voltage(ldo_28));
        regulator_put(ldo_28);

        regulator_set_voltage(ldo_18, 1800000, 1800000);
        //regulator_set_suspend_voltage(ldo, 1800000);
        regulator_enable(ldo_18);
        //printk("%s set ldo1 vcc18_cif=%dmV end\n", __func__, regulator_get_voltage(ldo_18));
        regulator_put(ldo_18);
    }

    return 0;
}
#endif

#if CONFIG_SENSOR_RESET_IOCTL_USR
static int sensor_reset_usr_cb (struct rk29camera_gpio_res *res,int on)
{
	#error "CONFIG_SENSOR_RESET_IOCTL_USR is 1, sensor_reset_usr_cb function must be writed!!";
}
#endif

#if CONFIG_SENSOR_POWERDOWN_IOCTL_USR
static int sensor_powerdown_usr_cb (struct rk29camera_gpio_res *res,int on)
{
	#error "CONFIG_SENSOR_POWERDOWN_IOCTL_USR is 1, sensor_powerdown_usr_cb function must be writed!!";
}
#endif

#if CONFIG_SENSOR_FLASH_IOCTL_USR
static int sensor_flash_usr_cb (struct rk29camera_gpio_res *res,int on)
{
	#error "CONFIG_SENSOR_FLASH_IOCTL_USR is 1, sensor_flash_usr_cb function must be writed!!";
}
#endif

static struct rk29camera_platform_ioctl_cb	sensor_ioctl_cb = {
	#if CONFIG_SENSOR_POWER_IOCTL_USR
	.sensor_power_cb = sensor_power_usr_cb,
	#else
	.sensor_power_cb = NULL,
	#endif

	#if CONFIG_SENSOR_RESET_IOCTL_USR
	.sensor_reset_cb = sensor_reset_usr_cb,
	#else
	.sensor_reset_cb = NULL,
	#endif

	#if CONFIG_SENSOR_POWERDOWN_IOCTL_USR
	.sensor_powerdown_cb = sensor_powerdown_usr_cb,
	#else
	.sensor_powerdown_cb = NULL,
	#endif

	#if CONFIG_SENSOR_FLASH_IOCTL_USR
	.sensor_flash_cb = sensor_flash_usr_cb,
	#else
	.sensor_flash_cb = NULL,
	#endif
};



static rk_sensor_user_init_data_s rk_init_data_sensor[RK_CAM_NUM] ;
#include "../../../drivers/media/video/rk30_camera.c"

#endif /* CONFIG_VIDEO_RK29 */
