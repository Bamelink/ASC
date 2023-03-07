#include "main.h"

/* STATICS */
rcl_publisher_t publisher;
std_msgs__msg__String msg;

int err;
int16_t buf;
struct adc_sequence sequence = {
	.buffer = &buf,
	/* buffer size in bytes, not number of samples */
	.buffer_size = sizeof(buf),
};


const struct device *const fcan = DEVICE_DT_GET(DT_ALIAS(fcan));
const struct device *const scan = DEVICE_DT_GET(DT_ALIAS(scan));
const struct device *const dlcan = DEVICE_DT_GET(DT_ALIAS(dlcan));
struct k_thread rx_thread_data;
k_tid_t rx_tid;

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
	(void) last_call_time;
	if (timer != NULL) {
	    uint16_t ch1 = 0, ch2 = 0, ch3 = 0;

		for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {

			(void)adc_sequence_init_dt(&adc_channels[i], &sequence);

			err = adc_read(adc_channels[i].dev, &sequence);
			if (err < 0) {
				printk("Could not read (%d)\n", err);
				continue;
			} else {
				//printk("%d\n", buf);
				if(i == 0){
					ch1 = buf;
				}
				if(i == 1){
					ch2 = buf;
				}
				if(i == 2){
					ch3 = buf;
				}
			}
		}
		sprintf(msg.data.data, "CH1 = %d, CH2 = %d, CH3 = %d", ch1, ch2, ch3);
		msg.data.size = strlen(msg.data.data);
		RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
	}
}




void main(void)
{
	int ret;
	ret = init(); // Initialze gpio, adc and can
	if (ret < 0) {
		printk("Initialization failed\n");
		return;
	}

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
	RCCHECK(rclc_node_init_default(&node, "zephyr_string_publisher", "", &support));

	// create publisher
	RCCHECK(rclc_publisher_init_default(
		&publisher,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
		"/zephyr_string_publisher"));

	rcl_timer_t timer;
	const unsigned int timer_timeout = 1000;
	RCCHECK(rclc_timer_init_default(
		&timer,
		&support,
		RCL_MS_TO_NS(timer_timeout),
		timer_callback));

	// create executor
	rclc_executor_t executor;
	RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
	RCCHECK(rclc_executor_add_timer(&executor, &timer));

	msg.data.data = (char * ) malloc(ARRAY_LEN * sizeof(char));
	msg.data.size = 0;
	msg.data.capacity = ARRAY_LEN;
	/* END ros setup END */
	

	while(1){
		rclc_executor_spin(&executor);
	}

	// free resources
	RCCHECK(rcl_publisher_fini(&publisher, &node));
	RCCHECK(rcl_node_fini(&node));
}