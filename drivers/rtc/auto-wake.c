#include <linux/android_alarm.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/sysdev.h>
#include <linux/wakelock.h>
#include <linux/power_supply.h>
#include <linux/suspend.h>

#include <asm/mach/time.h>

#define WAKEUP_CLOSE 0
#define WAKEUP_OPEN  1
#define WAKEUP_DEFAULT_TIME 30

extern void rk28_send_wakeup_key(void);

struct auto_wake
{
	struct alarm alarm;
	struct timespec timer;
	//struct notifier_block pm_nb;
};

static struct auto_wake auto_wake;
static void auto_wake_update(struct auto_wake *auto_wake);

static int wakeup_time = WAKEUP_DEFAULT_TIME;
static int auto_wakeup_switch = WAKEUP_CLOSE;
static ssize_t auto_wakeup_timer_show(struct class *cls,struct class_attribute *attr, char *_buf)
{
    return sprintf(_buf, "%d\n", wakeup_time);
}

static ssize_t auto_wakeup_timer_store(struct class *cls, struct class_attribute *attr,const char *_buf, size_t _count)
{
#if 0	
    wakeup_time = simple_strtoul(_buf, NULL, 16);
    printk("wakeup_time = %d\n",wakeup_time);
#else
    int timer=0;
    sscanf(_buf, "%d", &timer);	
    if(timer != 0)
    {
    	if(timer < 30)
		timer = 30;
        wakeup_time = timer;
        auto_wake.timer.tv_sec = timer;
        printk("wakeup_time=%d\n",wakeup_time);
    }
#endif
    return _count;
} 

static ssize_t auto_wakeup_switch_show(struct class *cls,struct class_attribute *attr, char *_buf)
{
    return sprintf(_buf, "%d\n", auto_wakeup_switch);
}
int in_rtc_auto_wakeup_mode=0;
static ssize_t auto_wakeup_switch_store(struct class *cls, struct class_attribute *attr,const char *_buf, size_t _count)
{
#if 0	
    wakeup_time = simple_strtoul(_buf, NULL, 16);
    printk("wakeup_time = %d\n",wakeup_time);
#else
    int timer_switch=0;
    sscanf(_buf, "%d", &timer_switch);	
    if(timer_switch != 0)
   {
	auto_wakeup_switch = WAKEUP_OPEN;
	in_rtc_auto_wakeup_mode = 1;
	auto_wake_update(&auto_wake);
	printk("auto_wakeup_switch=%d\n",auto_wakeup_switch);
    }else{
	auto_wakeup_switch = WAKEUP_CLOSE;
	in_rtc_auto_wakeup_mode = 0;
    }
#endif
    return _count;
} 
static struct class *auto_wakeup_time = NULL;
static CLASS_ATTR(auto_wakeup_timer, 0666, auto_wakeup_timer_show, auto_wakeup_timer_store);
static CLASS_ATTR(auto_wakeup_switch, 0666, auto_wakeup_switch_show, auto_wakeup_switch_store);

struct timespec set_atuo_wake_time( struct timespec timer)
{
	struct timespec new_time;
	struct timespec tmp_time;	
	tmp_time =ktime_to_timespec(alarm_get_elapsed_realtime());

	printk("nowtime = %ld \n",tmp_time.tv_sec);
	
	new_time.tv_nsec = timer.tv_nsec+ tmp_time.tv_nsec;
	new_time.tv_sec =  timer.tv_sec+ tmp_time.tv_sec;
	//new_time.tv_sec =  wakeup_time+ tmp_time.tv_sec;

	return new_time;
}

static void auto_wake_update(struct auto_wake *auto_wake)
{
	printk("hjc:%s>>>>>>>>>>>>>\n",__func__);

	struct timespec new_alarm_time;
	
//	printk("auto_wake_update\n");
	new_alarm_time = set_atuo_wake_time(auto_wake->timer);
	alarm_start_range(&auto_wake->alarm,
			timespec_to_ktime(new_alarm_time),
			timespec_to_ktime(new_alarm_time));
}

static void atuo_wake_trigger(struct alarm *alarm)
{
	struct auto_wake *auto_wake = container_of(alarm, struct auto_wake,
						    alarm);
	if(auto_wakeup_switch == WAKEUP_CLOSE){
		printk("hjc:%s,wakeup_close\n",__func__);
	}else{
		auto_wake_update(auto_wake);
		rk28_send_wakeup_key();
	}
}


#if 0
static void auto_wake_cancel(struct auto_wake *auto_wake)
{
	alarm_cancel(&auto_wake->alarm);
}



static int auto_wake_callback(struct notifier_block *nfb,
					unsigned long action,
					void *ignored)
{

	struct auto_wake *auto_wake = container_of(nfb, struct auto_wake,
						    pm_nb);

	switch (action)
	{
		case PM_SUSPEND_PREPARE:
		{
			printk("PM_SUSPEND_PREPARExsf \n");
			auto_wake_update(auto_wake);
			return NOTIFY_OK;
		}
		case PM_POST_SUSPEND:
		{
			printk("PM_POST_SUSPENDxsf \n");
		//	auto_wake_cancel(auto_wake);
			return NOTIFY_OK;
		}
	}

	return NOTIFY_DONE;
}


static struct notifier_block auto_wake_pm_notifier = {
	.notifier_call = auto_wake_callback,
	.priority = 0,
};

#endif

void auto_wake_init(void)
{
	struct timespec timer;
//	auto_wake->pm_nb = auto_wake_pm_notifier;

	timer.tv_sec =  wakeup_time;//CONFIG_AUTO_WAKE_UP_PERIOD;
	timer.tv_nsec = 0;

	auto_wake.timer = timer;
	
	alarm_init(&auto_wake.alarm,ANDROID_ALARM_ELAPSED_REALTIME_WAKEUP,atuo_wake_trigger);
	//auto_wake_update(&auto_wake);
	//register_pm_notifier(&auto_wake->pm_nb);// xsf
}

static int  __init start_auto_wake(void)
{
   
	//struct timespec timer;
	//printk("CONFIG_AUTO_WAKE_UP_PERIOD = %d\n", CONFIG_AUTO_WAKE_UP_PERIOD);
	//timer.tv_sec =   wakeup_time;//CONFIG_AUTO_WAKE_UP_PERIOD;
	//timer.tv_nsec = 0;

	auto_wakeup_time = class_create(THIS_MODULE, "auto_wakeup");//HJC
	class_create_file(auto_wakeup_time,&class_attr_auto_wakeup_timer);
	class_create_file(auto_wakeup_time,&class_attr_auto_wakeup_switch);

	auto_wake_init();
	//auto_wake_update(&auto_wake);
	return 0;
} 

late_initcall_sync(start_auto_wake);
