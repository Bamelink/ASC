#include "main.h"

CAN_MSGQ_DEFINE(counter_msgq, 2);
K_THREAD_STACK_DEFINE(rx_thread_stack, RX_THREAD_STACK_SIZE);


// can interrupt
void tx_irq_callback(const struct device *dev, int error, void *arg)
{
	char *sender = (char *)arg;

	ARG_UNUSED(dev);

	if (error != 0) {
		printf("Callback! error-code: %d\nSender: %s\n",
		       error, sender);
	}
}

// res interrupt
void res_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{

}



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

int init_can(void)
{
	int ret;

	if (!device_is_ready(scan)) {
		printf("CAN: Device %s not ready.\n", scan->name);
		return -1;
	}
	if (!device_is_ready(fcan)) {
		printf("CAN: Device %s not ready.\n", fcan->name);
		return -1;
	}

    if (!device_is_ready(dlcan)) {
		printf("CAN: Device %s not ready.\n", dlcan->name);
		return -1;
	}

	ret = can_start(fcan);
	if (ret != 0) {
		printf("Error starting CAN controller [%d]", ret);
		return -1;
	}
	ret = can_start(scan);
	if (ret != 0) {
		printf("Error starting CAN controller [%d]", ret);
		return -1;
	}
    ret = can_start(dlcan);
	if (ret != 0) {
		printf("Error starting CAN controller [%d]", ret);
		return -1;
	}

	rx_tid = k_thread_create(&rx_thread_data, rx_thread_stack,
				 K_THREAD_STACK_SIZEOF(rx_thread_stack),
				 rx_thread, NULL, NULL, NULL,
				 RX_THREAD_PRIORITY, 0, K_NO_WAIT);
	if (!rx_tid) {
		printf("ERROR spawning rx thread\n");
	}
}

int init_adc(void)
{
    /* Configure channels individually prior to sampling. */
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if (!device_is_ready(adc_channels[i].dev)) {
			printk("ADC controller device not ready\n");
			return -1;
		}

		err = adc_channel_setup_dt(&adc_channels[i]);
		if (err < 0) {
			printk("Could not setup channel #%d (%d)\n", i, err);
			return -1;
		}
	}
}

int init_gpio(void)
{
    int ret;

    // Outputs
	if (!device_is_ready(asbindicator.port)) { return -1; }
	ret = gpio_pin_configure_dt(&asbindicator, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) { return ret; }

    if (!device_is_ready(valve2on.port)) { return -1; }
	ret = gpio_pin_configure_dt(&valve2on, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) { return ret; }

    if (!device_is_ready(valve3on.port)) { return -1; }
	ret = gpio_pin_configure_dt(&valve3on, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) { return ret; }

    if (!device_is_ready(ebsreleaseopen.port)) { return -1; }
	ret = gpio_pin_configure_dt(&ebsreleaseopen, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) { return ret; }

    if (!device_is_ready(valve1on.port)) { return -1; }
	ret = gpio_pin_configure_dt(&valve1on, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) { return ret; }

    if (!device_is_ready(mcuassdcclose.port)) { return -1; }
	ret = gpio_pin_configure_dt(&mcuassdcclose, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) { return ret; }

    if (!device_is_ready(scin.port)) { return -1; }
	ret = gpio_pin_configure_dt(&scin, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) { return ret; }

    if (!device_is_ready(asms.port)) { return -1; }
	ret = gpio_pin_configure_dt(&asms, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) { return ret; }

    if (!device_is_ready(wdi.port)) { return -1; }
	ret = gpio_pin_configure_dt(&wdi, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) { return ret; }
    
    if (!device_is_ready(hpcpon.port)) { return -1; }
	ret = gpio_pin_configure_dt(&hpcpon, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) { return ret; }


    // Inputs
    if (!device_is_ready(scblatchstate.port)) { return -1; }
	ret = gpio_pin_configure_dt(&scblatchstate, GPIO_INPUT);
	if (ret != 0) { return ret; }

    if (!device_is_ready(scalatchstate.port)) { return -1; }
	ret = gpio_pin_configure_dt(&scalatchstate, GPIO_INPUT);
	if (ret != 0) { return ret; }

    if (!device_is_ready(resk2.port)) { return -1; }
	ret = gpio_pin_configure_dt(&resk2, GPIO_INPUT);
	if (ret != 0) { return ret; }

    if (!device_is_ready(resk3.port)) { return -1; }
	ret = gpio_pin_configure_dt(&resk3, GPIO_INPUT);
	if (ret != 0) { return ret; }

    if (!device_is_ready(scafterres.port)) { return -1; }
	ret = gpio_pin_configure_dt(&scafterres, GPIO_INPUT);
	if (ret != 0) { return ret; }

    if (!device_is_ready(scbeforeres.port)) { return -1; }
	ret = gpio_pin_configure_dt(&scbeforeres, GPIO_INPUT);
	if (ret != 0) { return ret; }

    if (!device_is_ready(scpondelay.port)) { return -1; }
	ret = gpio_pin_configure_dt(&scpondelay, GPIO_INPUT);
	if (ret != 0) { return ret; }

    if (!device_is_ready(resetlatch.port)) { return -1; }
	ret = gpio_pin_configure_dt(&resetlatch, GPIO_INPUT);
	if (ret != 0) { return ret; }

    if (!device_is_ready(tsms.port)) { return -1; }
	ret = gpio_pin_configure_dt(&tsms, GPIO_INPUT);
	if (ret != 0) { return ret; }

    // Interrupts
    gpio_pin_interrupt_configure_dt(&resk2, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_pin_interrupt_configure_dt(&resk3, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&res_k2_cb_data, res_pressed, BIT(resk2.pin));
    gpio_init_callback(&res_k3_cb_data, res_pressed, BIT(resk3.pin));
    gpio_add_callback(resk2.port, &res_k2_cb_data);
    gpio_add_callback(resk2.port, &res_k3_cb_data);
}

int init(void)
{
    int ret;

    ret = init_gpio();
    if (ret < 0) {
		return ret;
	}

    ret = init_can();
    if (ret < 0) {
		return ret;
	}

    ret = init_adc();
    if (ret < 0) {
		return ret;
	}
}