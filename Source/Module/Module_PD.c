/*
 * Module_PD.c
 *
 * HUSB238 PD Sink module
 */

#include "Module_PD.h"

extern i2c_bus_class i2c_bus_list[];

int pd_init(pd_module_class* self)
{
	int ret;
	unsigned char dummy;

	self->mux.i2c.bus = &i2c_bus_list[1];
	self->mux.i2c.addr_wide = i2c_8bit_mode;
	self->mux.addr = 0x59;

	self->husb238.bus = &i2c_bus_list[1];
	self->husb238.addr_wide = i2c_8bit_mode;
	self->husb238.addr = HUSB238_I2C_ADDR;

	ret = pca9847_select_channel(&self->mux, PD_MUX_CH);
	if (ret != 0) return -1;

	/* verify chip presence by reading PD_STATUS0 */
	ret = i2c_dev_read_byte(&self->husb238, HUSB238_REG_PD_STATUS0, &dummy);

	pca9847_disable_all(&self->mux);

	return (ret == i2c_ack) ? 0 : -2;
}

int pd_request_voltage(pd_module_class* self, unsigned char voltage_code)
{
	unsigned char cmd;
	int ret;

	/* voltage_code should be HUSB238_VOLTAGE_5V..20V */
	if ((voltage_code & 0xF0) < HUSB238_VOLTAGE_5V ||
	    (voltage_code & 0xF0) > HUSB238_VOLTAGE_20V)
		return -1;

	/* combine voltage with 3A current request */
	cmd = voltage_code | HUSB238_CURRENT_3_0A;

	ret = pca9847_select_channel(&self->mux, PD_MUX_CH);
	if (ret != 0) return -2;

	ret = i2c_dev_write_byte(&self->husb238, HUSB238_REG_GO_COMMAND, cmd);

	pca9847_disable_all(&self->mux);

	return (ret == i2c_ack) ? 0 : -3;
}

int pd_get_status(pd_module_class* self, unsigned char* pVoltage, unsigned char* pCurrent)
{
	unsigned char data;
	int ret;

	ret = pca9847_select_channel(&self->mux, PD_MUX_CH);
	if (ret != 0) return -1;

	ret = i2c_dev_read_byte(&self->husb238, HUSB238_REG_PD_STATUS0, &data);

	pca9847_disable_all(&self->mux);

	if (ret != i2c_ack) return -2;

	if (pVoltage) *pVoltage = (data & HUSB238_STATUS_VOLTAGE_MASK) >> 4;
	if (pCurrent) *pCurrent = (data & HUSB238_STATUS_CURRENT_MASK);

	return 0;
}

/*
 * Scan SRC_PDO registers 0x02-0x07 to find available voltage levels.
 * Returns a bitmask: bit0=5V, bit1=9V, bit2=12V, bit3=15V, bit4=18V, bit5=20V.
 */
int pd_get_available_pdo(pd_module_class* self, unsigned char* pPdoMask)
{
	unsigned char data;
	unsigned char mask = 0;
	int ret;
	int i;

	ret = pca9847_select_channel(&self->mux, PD_MUX_CH);
	if (ret != 0) return -1;

	for (i = 0; i < 6; i++)
	{
		ret = i2c_dev_read_byte(&self->husb238,
			HUSB238_REG_SRC_PDO_5V + i, &data);
		if (ret == i2c_ack && (data & 0x80))
			mask |= (1 << i);
	}

	pca9847_disable_all(&self->mux);

	*pPdoMask = mask;
	return 0;
}
