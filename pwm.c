#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/time.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/slab.h>

#define DEBUG 0

#define NANO_SEC 	1000000000
#define MICRO_SEC	1000000

#define DEFAULT_FREQ		1000
#define DEFAULT_DUTY_CYCLE	50


struct pwm_channel {
	struct hrtimer tm1, tm2;
	ktime_t t1;
	int freq;
	int dc;
	int gpio;

	// linked list of channels
	struct list_head chan_list;
};


// linked list of channels
static LIST_HEAD(_channels);


enum hrtimer_restart cb1(struct hrtimer *t) {
	struct pwm_channel *ch = container_of(t, struct pwm_channel, tm1);
	ktime_t now;
	int ovr;

	now = hrtimer_cb_get_time(t);
	ovr = hrtimer_forward(t, now, ch->t1);
	
#if DEBUG == 1
	printk(KERN_EMERG "up [%d]\n", ovr);
#endif

	gpio_set_value(ch->gpio, 1);

	if (ch->dc < 100) {
		unsigned long t_ns = ((MICRO_SEC * 10 * ch->dc) / (ch->freq));
		ktime_t t2 = ktime_set( 0, t_ns );
		hrtimer_start(&ch->tm2, t2, HRTIMER_MODE_REL);
	}

	return HRTIMER_RESTART;
}


enum hrtimer_restart cb2(struct hrtimer *t) {
	struct pwm_channel *ch = container_of(t, struct pwm_channel, tm2);

#if DEBUG == 1
	printk(KERN_INFO "down\n");
#endif

	gpio_set_value(ch->gpio, 0);
	return HRTIMER_NORESTART;
}


void init_channel(struct pwm_channel *a_ch) {
	unsigned long t_ns = (NANO_SEC)/a_ch->freq;

	hrtimer_init(&a_ch->tm1, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer_init(&a_ch->tm2, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	a_ch->t1 = ktime_set( 0, t_ns );
	a_ch->tm1.function = &cb1;
	a_ch->tm2.function = &cb2;

	gpio_request(a_ch->gpio, "soft_pwm_gpio");
	gpio_direction_output(a_ch->gpio, 1);

	hrtimer_start(&a_ch->tm1, a_ch->t1, HRTIMER_MODE_REL);
}


void deinit_channel(struct pwm_channel *a_ch) {
	if (hrtimer_active(&a_ch->tm1) || hrtimer_is_queued(&a_ch->tm1)) {
		hrtimer_cancel(&a_ch->tm1);
	}

	if (hrtimer_active(&a_ch->tm2) || hrtimer_is_queued(&a_ch->tm2)) {
		hrtimer_cancel(&a_ch->tm2);
	}

	gpio_free(a_ch->gpio);
}

/*
static void deinit_timers(void) {
	deinit_channel(&_channels[0]);
}
*/

static ssize_t export_store(struct class *class, 
		struct class_attribute *attr, 
		const char *buf, 
		size_t len) {

	struct list_head *p = NULL;
	struct pwm_channel *ch = NULL;

	int found = 0;
	int gpio = 0;

	kstrtol(buf, 10, &gpio);
	list_for_each (p, &_channels) {
		ch = list_entry(p, struct pwm_channel, chan_list);
		if (ch->gpio == gpio) {
			found = 1;
			break;
		}
	}

	if (found) {
		// channel for the given gpio already exists
		// ignore the request
		printk(KERN_ERR "channel for the gpio already allocated\n");
		return len;
	}

	
	// create the channel
	if (!(ch = kmalloc(sizeof(struct pwm_channel), GFP_KERNEL))) {
		printk(KERN_ERR "Unable to allocate memory for the channel\n");
		return -ENOMEM;
	}

	// initialize the channel 
	ch->gpio = gpio;
	ch->freq = DEFAULT_FREQ;
	ch->dc = DEFAULT_DUTY_CYCLE;
	INIT_LIST_HEAD(&ch->chan_list);
	list_add(&ch->chan_list, &_channels);

	// create sysfs entries


	// initialize channel's timers
	init_channel(ch);
	
	return len;
}


static ssize_t unexport_store(struct class *class, 
		struct class_attribute *attr, 
		const char *buf, 
		size_t len) {

	struct list_head *p = NULL;
	struct pwm_channel *ch = NULL;

	int found = 0;
	long gpio = 0;

	kstrtol(buf, 10, &gpio);
	list_for_each (p, &_channels) {
		ch = list_entry(p, struct pwm_channel, chan_list);
		if (ch->gpio == gpio) {
			found = 1;
			break;
		}
	}

	if (!found) {
		// channel for the given gpio doesn't exists
		// ignore the request
		return len;
	}

	
	return len;
}


static struct class_attribute soft_pwm_class_attrs[] = {
	__ATTR(export, 0222, NULL, export_store),
	__ATTR(unexport, 0222, NULL, unexport_store),
	__ATTR_NULL,
};


static struct class soft_pwm_class = {
	.name = "soft_pwm",
	.owner = THIS_MODULE,
	.class_attrs = soft_pwm_class_attrs,
};



static int __init pwm_init(void) {
#if DEBUG == 1
	printk(KERN_INFO "installing soft pwm module\n");
#endif

	return class_register(&soft_pwm_class);
}


static void __exit pwm_exit(void) {
#if DEBUG == 1
	printk(KERN_INFO "deinstalling soft pwm module\n");
#endif

	class_unregister(&soft_pwm_class);
}


module_init(pwm_init);
module_exit(pwm_exit);

MODULE_AUTHOR("tw");
MODULE_DESCRIPTION("HR PWM TESTS");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
