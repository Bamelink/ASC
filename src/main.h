// For uROS
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
#include <std_msgs/msg/int32.h>

#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <rmw_microros/rmw_microros.h>
#include <microros_transports.h>

// For CAN
#include <zephyr/kernel.h>
#include <zephyr/drivers/can.h>
#include <zephyr/sys/byteorder.h>


// uROS defines and statics
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);for(;;){};}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

rcl_publisher_t publisher;
std_msgs__msg__Int32 msg;

/* CAN DEFINES */
#define RX_THREAD_STACK_SIZE 512
#define RX_THREAD_PRIORITY 2
#define SLEEP_TIME K_SECONDS(2)
#define COUNTER_MSG_ID 0x12345

/* CAN global structs */
K_THREAD_STACK_DEFINE(rx_thread_stack, RX_THREAD_STACK_SIZE);
const struct device *const fcan = DEVICE_DT_GET(DT_ALIAS(fcan));
const struct device *const scan = DEVICE_DT_GET(DT_ALIAS(scan));
struct k_thread rx_thread_data;
CAN_MSGQ_DEFINE(counter_msgq, 2);