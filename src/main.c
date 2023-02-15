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



/* CAN */
void rx_thread(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);
	const struct can_filter my_filter = {
        .id_type = CAN_EXTENDED_IDENTIFIER,
        .rtr = CAN_DATAFRAME,
        .id = 0x1234567,
        .rtr_mask = 1,
        .id_mask = 0
	};
	struct can_frame rx_frame;
	int filter_id;

	filter_id = can_add_rx_filter_msgq(scan, &counter_msgq, &my_filter);
	printf("Counter filter id: %d\n", filter_id);

	while (1) {
		k_msgq_get(&counter_msgq, &rx_frame, K_FOREVER);

		if (rx_frame.dlc != 2U) {
			printf("Wrong data length: %u\n", rx_frame.dlc);
			continue;
		}

		printf("Counter received: %u\n",
		       sys_be16_to_cpu(UNALIGNED_GET((uint16_t *)&rx_frame.data)));
	}
}

void tx_irq_callback(const struct device *dev, int error, void *arg)
{
	char *sender = (char *)arg;

	ARG_UNUSED(dev);

	if (error != 0) {
		printf("Callback! error-code: %d\nSender: %s\n",
		       error, sender);
	}
}



void main(void)
{

	/* BEGIN ros setup BEGIN */
	rmw_uros_set_custom_transport(
		MICRO_ROS_FRAMING_REQUIRED,
		(void *) &default_params,
		zephyr_transport_open,
		zephyr_transport_close,
		zephyr_transport_write,
		zephyr_transport_read
	);

	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;

	// create init_options
	RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

	// create node
	rcl_node_t node;
	RCCHECK(rclc_node_init_default(&node, "zephyr_int32_publisher", "", &support));

	// create publisher
	RCCHECK(rclc_publisher_init_default(
		&publisher,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"zephyr_int32_publisher"));

	// create executor
	rclc_executor_t executor;
	RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));

	msg.data = 0;
	/* END ros setup END */

	/* BEGIN can setup BEGIN*/
	struct can_frame frame = {
                .id_type = CAN_EXTENDED_IDENTIFIER,
                .rtr = CAN_DATAFRAME,
                .id = 0x1234567,
                .dlc = 1
    };
	uint8_t counter = 0;
	k_tid_t rx_tid;
	int ret;

	if (!device_is_ready(scan)) {
		printf("CAN: Device %s not ready.\n", scan->name);
		return;
	}
	if (!device_is_ready(fcan)) {
		printf("CAN: Device %s not ready.\n", fcan->name);
		return;
	}

	ret = can_start(fcan);
	if (ret != 0) {
		printf("Error starting CAN controller [%d]", ret);
		return;
	}
	ret = can_start(scan);
	if (ret != 0) {
		printf("Error starting CAN controller [%d]", ret);
		return;
	}

	rx_tid = k_thread_create(&rx_thread_data, rx_thread_stack,
				 K_THREAD_STACK_SIZEOF(rx_thread_stack),
				 rx_thread, NULL, NULL, NULL,
				 RX_THREAD_PRIORITY, 0, K_NO_WAIT);
	if (!rx_tid) {
		printf("ERROR spawning rx thread\n");
	}
	/* END can setup END */
	

	while(1){
		rclc_executor_spin(&executor);
		frame.data[0] = counter;
		//can_send(fcan, &frame, K_MSEC(100), NULL, NULL);
	}

	// free resources
	RCCHECK(rcl_publisher_fini(&publisher, &node))
	RCCHECK(rcl_node_fini(&node))
}