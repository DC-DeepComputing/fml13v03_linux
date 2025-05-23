
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_data/cros_ec_proto.h>
#include <linux/platform_data/cros_ec_commands.h>

#define DRV_NAME "cros-ec-poweroff"

static struct cros_ec_device *ec_dev;
static void (*orig_pm_power_off)(void);

static int cros_ec_send_shutdown(struct cros_ec_device *ec)
{
	struct spi_device *spi;
	struct spi_message msg;
	struct spi_transfer xfer = { 0 };
	struct spi_transfer trans_delay = { 0 };
	u8 tx_buf[8], rx_buf[8];
	struct cros_ec_command cmd = { 0 };
	int len, ret;

	if (!ec) {
		dev_err(ec->dev, "EC device not available\n");
		return -ENODEV;
	}
	if (!ec->dev) {
		dev_err(ec->dev, "EC device structure is invalid\n");
		return -EINVAL;
	}

	spi = to_spi_device(ec->dev);
	if (!spi) {
		dev_err(ec->dev, "Not an SPI-based EC device\n");
		return -EINVAL;
	}

	cmd.version = 0;
	cmd.command = EC_CMD_HOST_SHUTDOWN;
	cmd.outsize = 0;
	cmd.insize = 0;
	len = cros_ec_prepare_tx(ec, &cmd);
	if (len < 0) {
		dev_err(ec->dev, "Failed to prepare TX data\n");
		return len;
	}

    if (len > sizeof(tx_buf)) {
        dev_err(ec->dev, "TX data length %d exceeds buffer size %zu\n",
                len, sizeof(tx_buf));
        return -EINVAL;
    }
	memcpy(tx_buf, ec->dout, len);
	xfer.tx_buf = tx_buf;
	xfer.rx_buf = rx_buf;
	xfer.len = len;
    spi_message_init(&msg);
    trans_delay.delay.value = 10;
    trans_delay.delay.unit = SPI_DELAY_UNIT_USECS;
    spi_message_add_tail(&trans_delay, &msg);
	spi_message_add_tail(&xfer, &msg);

    dev_dbg(ec->dev, "Sending shutdown command to EC\n");
    ret = spi_sync(spi, &msg);
	if (ret < 0) {
        dev_err(ec->dev, "SPI sync failed: %d\n", ret);
		return ret;
	}

    dev_info(ec->dev, "Shutdown command sent to EC via SPI\n");
	return 0;
}

static void cros_ec_power_off(void)
{
	int ret;

	if (!ec_dev || !ec_dev->dev) {
		if (orig_pm_power_off)
			orig_pm_power_off();
		return;
	}

    ret = cros_ec_send_shutdown(ec_dev);
	if (ret < 0) {
		dev_err(ec_dev->dev,
			"EC power off failed, falling back to original\n");
		if (orig_pm_power_off)
			orig_pm_power_off();
	} else {
		dev_info(ec_dev->dev, "System halted via EC\n");
		mdelay(1000);
		while (1)
			cpu_relax();
	}
}

static int cros_ec_poweroff_probe(struct platform_device *pdev)
{
	struct spi_device *spi;

	dev_dbg(&pdev->dev, "Probing cros_ec_poweroff\n");

	ec_dev = dev_get_drvdata(pdev->dev.parent);
	if (!ec_dev) {
		dev_err(&pdev->dev, "Failed to get cros_ec_device\n");
		return -ENODEV;
	}

	spi = to_spi_device(ec_dev->dev);
	if (!spi) {
		dev_err(&pdev->dev, "EC device is not SPI-based\n");
		return -EINVAL;
	}

	orig_pm_power_off = pm_power_off;
	pm_power_off = cros_ec_power_off;

	dev_info(&pdev->dev, "CROS EC poweroff driver registered (SPI)\n");
	return 0;
}

static int cros_ec_poweroff_remove(struct platform_device *pdev)
{
	if (pm_power_off == cros_ec_power_off)
		pm_power_off = orig_pm_power_off;

	ec_dev = NULL;
	dev_info(&pdev->dev, "CROS EC poweroff driver unregistered\n");
	return 0;
}

static const struct of_device_id cros_ec_poweroff_of_match[] = {
	{ .compatible = "google,cros-ec-poweroff" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, cros_ec_poweroff_of_match);

static struct platform_driver cros_ec_poweroff_driver = {
	.probe = cros_ec_poweroff_probe,
	.remove = cros_ec_poweroff_remove,
	.driver = {
		.name = DRV_NAME,
		.of_match_table = cros_ec_poweroff_of_match,
	},
};

module_platform_driver(cros_ec_poweroff_driver);

MODULE_SOFTDEP("pre: cros_ec_dev cros_ec_spi");
MODULE_DESCRIPTION("Chrome OS EC Poweroff Driver via SPI");
MODULE_AUTHOR("WangYang <yang.wang@deepcomputing.io>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);
