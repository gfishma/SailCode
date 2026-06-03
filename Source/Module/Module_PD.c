/*
 * Module_PD.c
 *
 * HUSB238 PD Sink module
 */

#include "Module_PD.h"

extern i2c_bus_class i2c_bus_list[];

/*
 * Voltage code lookup: pdo_code → string index (0=??, 1=5V..6=20V)
 */
static unsigned char pdo_to_index(unsigned char pdo)
{
	switch (pdo) {
	case HUSB238_PDO_5V:  return 1;
	case HUSB238_PDO_9V:  return 2;
	case HUSB238_PDO_12V: return 3;
	case HUSB238_PDO_15V: return 4;
	case HUSB238_PDO_18V: return 5;
	case HUSB238_PDO_20V: return 6;
	default:              return 0;
	}
}

static const unsigned char index_to_pdo[7] = {
	0,                        /* 0 = invalid */
	HUSB238_PDO_5V,           /* 1 */
	HUSB238_PDO_9V,           /* 2 */
	HUSB238_PDO_12V,          /* 3 */
	HUSB238_PDO_15V,          /* 4 */
	HUSB238_PDO_18V,          /* 5 */
	HUSB238_PDO_20V,          /* 6 */
};

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

	/* verify chip presence */
	ret = i2c_dev_read_byte(&self->husb238, HUSB238_REG_PD_STATUS0, &dummy);

	pca9847_disable_all(&self->mux);

	return (ret == i2c_ack) ? 0 : -2;
}

/*
 * Request PD voltage.
 *   1. Check source supports this PDO
 *   2. Write SRC_PDO with PDO_SELECT
 *   3. Write GO_COMMAND with REQUEST_PDO
 */
int pd_request_voltage(pd_module_class* self, unsigned char pdo_code)
{
	unsigned char pdo_reg, pdo_data;
	unsigned char v_idx;
	int ret;

	v_idx = pdo_to_index(pdo_code);
	if (v_idx == 0) return -1;

	/* check source capability */
	ret = pca9847_select_channel(&self->mux, PD_MUX_CH);
	if (ret != 0) return -2;

	pdo_reg = HUSB238_REG_SRC_PDO_5V + (v_idx - 1);
	ret = i2c_dev_read_byte(&self->husb238, pdo_reg, &pdo_data);
	if (ret != i2c_ack) { pca9847_disable_all(&self->mux); return -3; }
	if (!(pdo_data & HUSB238_PDO_DETECT_MASK))
		{ pca9847_disable_all(&self->mux); return -4; }

	/* Step 1: select PDO */
	ret = i2c_dev_write_byte(&self->husb238, HUSB238_REG_SRC_PDO, pdo_code);
	if (ret != i2c_ack) { pca9847_disable_all(&self->mux); return -5; }

	/* Step 2: trigger request */
	ret = i2c_dev_write_byte(&self->husb238, HUSB238_REG_GO_COMMAND,
		HUSB238_GO_REQUEST_PDO);

	pca9847_disable_all(&self->mux);

	return (ret == i2c_ack) ? 0 : -6;
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

	if (pVoltage) *pVoltage = (data & 0xF0);
	if (pCurrent) *pCurrent = (data & 0x0F);

	return 0;
}

/*
 * Scan SRC_PDO registers 0x02-0x07.
 * Returns a bitmask of available voltage indices (1=5V..6=20V).
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
		if (ret == i2c_ack && (data & HUSB238_PDO_DETECT_MASK))
			mask |= (1 << i);
	}

	pca9847_disable_all(&self->mux);

	*pPdoMask = mask;
	return 0;
}
