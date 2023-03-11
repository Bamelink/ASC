#pragma once

/* INCLUDES */
// uROS
#include <version.h>

#if ZEPHYR_VERSION_CODE >= ZEPHYR_VERSION(3,1,0)
#include <zephyr/zephyr.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/posix/time.h>
#else
#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <posix/time.h>
#endif

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/string.h>

#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <rmw_microros/rmw_microros.h>
#include <microros_transports.h>

// can
#include <zephyr/kernel.h>
#include <zephyr/drivers/can.h>
#include <zephyr/sys/byteorder.h>

// adc
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <zephyr/drivers/adc.h>




/* DEFINES */
// uROS
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);for(;;){};}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

#define ARRAY_LEN 200

// adc
#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

// can
#define RX_THREAD_STACK_SIZE 2048
#define RX_THREAD_PRIORITY 2
#define SLEEP_TIME K_SECONDS(2)
#define COUNTER_MSG_ID 0x12345



/* FUNCTIONS */
// general
int init(void);

// can
void rx_thread(void *arg1, void *arg2, void *arg3);



/* STATICS */
extern rcl_publisher_t publisher;
extern std_msgs__msg__String msg;

//adc
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};
extern int err;
extern int16_t buf;
extern struct adc_sequence sequence;

// can
extern const struct device *const fcan;
extern const struct device *const scan;
extern const struct device *const dlcan;
extern struct k_thread rx_thread_data;
extern k_tid_t rx_tid;

// gpio outputs
extern const struct gpio_dt_spec asbindicator;
extern const struct gpio_dt_spec valve2on;
extern const struct gpio_dt_spec valve3on;
extern const struct gpio_dt_spec ebsreleaseopen;
extern const struct gpio_dt_spec valve1on;
extern const struct gpio_dt_spec mcuassdcclose;
extern const struct gpio_dt_spec scin;
extern const struct gpio_dt_spec asms;
extern const struct gpio_dt_spec wdi;
extern const struct gpio_dt_spec hpcpon;

// gpio inputs
extern const struct gpio_dt_spec scblatchstate;
extern const struct gpio_dt_spec scalatchstate;
extern const struct gpio_dt_spec resk2;
extern const struct gpio_dt_spec resk3;
extern const struct gpio_dt_spec scafterres;
extern const struct gpio_dt_spec scbeforeres;
extern const struct gpio_dt_spec scpondelay;
extern const struct gpio_dt_spec resetlatch;
extern const struct gpio_dt_spec tsms;
 
// gpio callback data
extern struct gpio_callback res_k2_cb_data;
extern struct gpio_callback res_k3_cb_data;