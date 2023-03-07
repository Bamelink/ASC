#include "main.h"

int err;
int16_t buf;
struct adc_sequence sequence = {
	.buffer = &buf,
	/* buffer size in bytes, not number of samples */
	.buffer_size = sizeof(buf),
};

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

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
	(void) last_call_time;
	if (timer != NULL) {
	    uint16_t ch1 = 0, ch2 = 0, ch3 = 0;

		for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {

			//printk("- %s, channel %d: ",
			//       adc_channels[i].dev->name,
			//       adc_channels[i].channel_id);

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

	/* BEGIN can setup BEGIN*/
	/*
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
	}*/
	/* END can setup END */

	/* BEGIN adc setup BEGIN */
	
	/* Configure channels individually prior to sampling. */
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if (!device_is_ready(adc_channels[i].dev)) {
			printk("ADC controller device not ready\n");
			return;
		}

		err = adc_channel_setup_dt(&adc_channels[i]);
		if (err < 0) {
			printk("Could not setup channel #%d (%d)\n", i, err);
			return;
		}
	}
	/* END adc setup END */
	

	while(1){
		rclc_executor_spin(&executor);
	}

	// free resources
	RCCHECK(rcl_publisher_fini(&publisher, &node))
	RCCHECK(rcl_node_fini(&node))
}