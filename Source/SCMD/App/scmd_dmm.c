/*
 * scmd_dmm.c
 *
 * em_dmm — voltage measurement via switch matrix or relay input channels
 *   em_dmm(Xn)                  — route Xn→Y6→T13, read DVM CH2
 *   em_dmm(HIGH_CURR_CHn)       — select relay ch, auto LV/HV via IO79
 *   em_dmm(HIGH_VOLT_CHn)       — select relay ch, auto LV/HV via IO78
 *   LV path: Y→T13→DVM CH2 (needs switch matrix). HV path: DVM CH1 (1/6 divider).
 */

#include "scmd_dmm.h"
#include "Module_SwitchMatrix.h"
#include "Module_DVM_V2.h"
#include "scmd_emio.h"

extern scmd_class scmd_ctrl;
extern M_DVM_V2_Def DVM_V2;

extern int __scmd_help(scmd_class* pCmd, char* pData, unsigned short len);

/* existing X→Y6→T13 measurement */
#define DMM_Y              6
#define DMM_T              13
#define DMM_DVM_CH         2

/* high voltage path (DVM CH1, external 1/6 divider) */
#define DMM_CH1            1
#define DMM_CH1_DIVIDER    6.0f

/* relay channel measurement */
#define HIGH_CURR_Y        8
#define HIGH_VOLT_Y        7
#define HIGH_CURR_CH_COUNT 15
#define HIGH_VOLT_CH_COUNT 28

/* path control IOs */
#define IO_PATH_77         77
#define IO_PATH_78         78
#define IO_PATH_79         79
#define IO_PATH_87         87

static switch_matrix_class* sm = NULL;

void scmd_dmm_set_switch_matrix(switch_matrix_class* p) { sm = p; }

/* ---- channel IO definitions (one-hot, user sets before measurement) ---- */

static const unsigned char high_curr_ch_io[HIGH_CURR_CH_COUNT] = {
    57, 58, 59, 60, 61, 62, 63, 64,   /* CH1-8 */
    89, 90, 91, 92, 93, 94, 95        /* CH9-15 */
};

static const unsigned char high_volt_ch_row[HIGH_VOLT_CH_COUNT] = {
    65, 65, 66, 66, 67, 67, 68, 68, 69, 69, 70, 70, 71, 71,
    52, 52, 53, 53, 54, 54, 55, 55, 56, 56, 85, 85, 86, 86
};
static const unsigned char high_volt_ch_io72[HIGH_VOLT_CH_COUNT] = {
    0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
    0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1
};

/* ---- channel IO validation ---- */

static int check_high_curr_ch(unsigned char ch)
{
    unsigned char lv;
    unsigned char i;
    unsigned char on_count = 0;
    unsigned char target_on = 0;

    /* check all channel IOs: exactly one should be on, matching ch */
    /* all-0 = OFF (no channel selected) */
    for (i = 0; i < HIGH_CURR_CH_COUNT; i++)
    {
        emio_read_io(&emio_instance, high_curr_ch_io[i], &lv);
        if (lv) {
            on_count++;
            if (i + 1 == ch) target_on = 1;
        }
    }

    if (on_count == 0)   return -5;  /* no channel selected (OFF) */
    if (on_count > 1)    return -6;  /* multiple channels selected */
    if (!target_on)      return -7;  /* wrong channel selected */

    return 0;
}

static int check_high_volt_ch(unsigned char ch)
{
    /* unique row IOs: 65-71, 52-56, 85-86 */
    static const unsigned char row_ios[] = {
        65,66,67,68,69,70,71, 52,53,54,55,56, 85,86
    };
    unsigned char lv;
    unsigned char i;
    unsigned char on_count = 0;
    unsigned char target_row = high_volt_ch_row[ch - 1];
    unsigned char target_io72 = high_volt_ch_io72[ch - 1];
    unsigned char found_target = 0;

    for (i = 0; i < sizeof(row_ios); i++)
    {
        emio_read_io(&emio_instance, row_ios[i], &lv);
        if (lv) {
            on_count++;
            if (row_ios[i] == target_row) found_target = 1;
        }
    }

    if (on_count == 0)   return -5;  /* no channel selected */
    if (on_count > 1)    return -6;  /* multiple channels selected */
    if (!found_target)   return -7;  /* wrong row IO selected */

    /* verify IO72 matches the target channel */
    emio_read_io(&emio_instance, 72, &lv);
    if (lv != target_io72) return -8;  /* IO72 mismatch */

    return 0;
}

/* ---- measurement functions (read IO state → determine DVM channel) ---- */

/*
 * HIGH_CURR_CH topology (15ch, one-hot: IO57-64 CH1-8, IO89-95 CH9-15, all-0=OFF):
 *   IO79=1, IO87=0, IO77=x → LV: Y8 → yf set(Y8,T13) → DVM CH2
 *   IO79=0, IO87=1, IO77=0 → HV: DVM CH1 (1/6 divider)
 */
static int meas_high_curr_ch(unsigned char ch, float* pVoltage)
{
    int ret;
    unsigned char io79 = 0, io87 = 0, io77 = 0;

    /* verify channel IO state */
    ret = check_high_curr_ch(ch);
    if (ret != 0) return ret;

    emio_read_io(&emio_instance, IO_PATH_79, &io79);
    emio_read_io(&emio_instance, IO_PATH_87, &io87);
    emio_read_io(&emio_instance, IO_PATH_77, &io77);

    if (io79 == 0 && io87 == 1 && io77 == 0)
    {
        /* HV: DVM CH1 (external 1/6 divider) */
        ret = (int)DVM_V2_GetVolt(&DVM_V2, DMM_CH1,
            Dvm_V2_Rang25V, Dvm_V2_Smp_Time_100MS, pVoltage);
        *pVoltage = *pVoltage * DMM_CH1_DIVIDER;
    }
    else if (io79 == 1)
    {
        /* LV: route Y8→T13→DVM CH2 */
        if (sm == NULL) return -3;

        ret = switch_matrix_connect_yf(sm, HIGH_CURR_Y, DMM_T, 1);
        if (ret != 0) return -2;

        ret = (int)DVM_V2_GetVolt(&DVM_V2, DMM_DVM_CH,
            Dvm_V2_Rang25V, Dvm_V2_Smp_Time_100MS, pVoltage);

        switch_matrix_connect_yf(sm, HIGH_CURR_Y, DMM_T, 0);
    }
    else
    {
        return -1;
    }

    return ret;
}

/*
 * HIGH_VOLT_CH topology:
 *   IO78=1, IO87=0, IO77=x → LV: Y7 → yf set(Y7,T13) → DVM CH2
 *   IO78=0, IO87=0, IO77=0 → HV: DVM CH1 (1/6 divider)
 */
static int meas_high_volt_ch(unsigned char ch, float* pVoltage)
{
    int ret;
    unsigned char io78 = 0, io87 = 0, io77 = 0;

    /* verify channel IO state */
    ret = check_high_volt_ch(ch);
    if (ret != 0) return ret;

    emio_read_io(&emio_instance, IO_PATH_78, &io78);
    emio_read_io(&emio_instance, IO_PATH_87, &io87);
    emio_read_io(&emio_instance, IO_PATH_77, &io77);

    if (io78 == 0 && io87 == 0 && io77 == 0)
    {
        /* HV: DVM CH1 (external 1/6 divider) */
        ret = (int)DVM_V2_GetVolt(&DVM_V2, DMM_CH1,
            Dvm_V2_Rang25V, Dvm_V2_Smp_Time_100MS, pVoltage);
        *pVoltage = *pVoltage * DMM_CH1_DIVIDER;
    }
    else if (io78 == 1)
    {
        /* LV: route Y7→T13→DVM CH2 */
        if (sm == NULL) return -3;

        ret = switch_matrix_connect_yf(sm, HIGH_VOLT_Y, DMM_T, 1);
        if (ret != 0) return -2;

        ret = (int)DVM_V2_GetVolt(&DVM_V2, DMM_DVM_CH,
            Dvm_V2_Rang25V, Dvm_V2_Smp_Time_100MS, pVoltage);

        switch_matrix_connect_yf(sm, HIGH_VOLT_Y, DMM_T, 0);
    }
    else
    {
        return -1;
    }

    return ret;
}

/* ---- SCMD table ---- */

static scmd_errCode_def __help(char *pData, unsigned short len);
static scmd_errCode_def __info(char *pData, unsigned short len);

static scmd_cmd_def dmm_func[] =
{
    {.func = __help, .name = "help", .dest = ">em_dmm help",                                      .isVisible = 1,},
    {.func = __info, .name = "info", .dest = ">em_dmm info",                                      .isVisible = 1,},
    {.func = __info, .name = "meas", .dest = ">em_dmm(Xn|HIGH_CURR_CHn|HIGH_VOLT_CHn)  path auto-detect via IO78/79", .isVisible = 1,},
};

static scmd_class dmm_ctrler =
{
    .cmdList = dmm_func, .cmdQty = (sizeof(dmm_func) / sizeof(dmm_func[0])),
    .stringLenthMax = 32, .sfunc_flag = 1,
};

/* ---- entry ---- */

scmd_errCode_def scmd_em_dmm(char* pData, unsigned short len)
{
    char *pNet = pData, *pEnd;
    unsigned short slen = 0;
    long ch_val;
    float voltage;
    int ret;

    str_deSpace(pData);

    /* sub-commands */
    if (strncmp(pData, "help", 4) == 0)
        return __help(pData, len);
    if (strncmp(pData, "info", 4) == 0)
        return __info(pData, len);

    pEnd = strstr(pNet, ")");
    if (pEnd == NULL)
        return __scmd_ErrMsg("<em_dmm(error), ')' not found.\r\n");
    pNet = strstr(pNet, "(");
    if (pNet == NULL)
        return __scmd_ErrMsg("<em_dmm(error), '(' not found.\r\n");
    pNet += 1; str_deSpace(pNet);

    /* ---- Xn path ---- */
    if (*pNet == 'X' || *pNet == 'x')
    {
        pNet++;
        pNet = str_GetHexDec(pNet, pEnd, &ch_val);
        if (pNet == NULL)
            return __scmd_ErrMsg("<em_dmm(error), X value not found.\r\n");
        if (ch_val < 1 || ch_val > SM_INPUT_TOTAL)
            return __scmd_ErrMsg("<em_dmm(error), X over range (1-300).\r\n");

        if (sm == NULL)
            return __scmd_ErrMsg("<em_dmm(error), switch matrix not initialized.\r\n");

        ret = switch_matrix_connect(sm, (unsigned short)ch_val, DMM_Y, DMM_T, 1);
        if (ret != 0) {
            slen += sprintf(scmd_msgBuf + slen,
                "<em_dmm(error) connect failed code=%d\r\n", ret);
            scmd_callback(scmd_msgBuf, slen);
            return scmd_normal;
        }

        ret = (int)DVM_V2_GetVolt(&DVM_V2, DMM_DVM_CH,
            Dvm_V2_Rang25V, Dvm_V2_Smp_Time_100MS, &voltage);

        switch_matrix_connect(sm, (unsigned short)ch_val, DMM_Y, DMM_T, 0);

        if (ret != 0) {
            slen += sprintf(scmd_msgBuf + slen,
                "<em_dmm(error) dvm read failed code=%d\r\n", ret);
            scmd_callback(scmd_msgBuf, slen);
            return scmd_normal;
        }

        {
            int mv = (int)(voltage * 1000.0f);
            slen += sprintf(scmd_msgBuf + slen,
                "<em_dmm(ok) X%d %dmV\r\n", (int)ch_val, mv);
        }
        scmd_callback(scmd_msgBuf, slen);
        return scmd_normal;
    }

    /* ---- HIGH_CURR_CHn / HIGH_VOLT_CHn path ---- */
    if (*pNet == 'H' || *pNet == 'h')
    {
        /* skip "HIGH" */
        char *p = pNet + 4;
        if (*p == '_') p++;

        /* HIGH_CURR_CHn */
        if ((p[0] == 'C' || p[0] == 'c') &&
            (p[1] == 'U' || p[1] == 'u') &&
            (p[2] == 'R' || p[2] == 'r') &&
            (p[3] == 'R' || p[3] == 'r'))
        {
            p += 4;
            /* skip optional "_CH" */
            if (*p == '_') p++;
            if (*p == 'C' || *p == 'c') p++;
            if (*p == 'H' || *p == 'h') p++;

            p = str_GetHexDec(p, pEnd, &ch_val);
            if (p == NULL)
                return __scmd_ErrMsg("<em_dmm(error), HIGH_CURR_CH number not found.\r\n");
            if (ch_val < 1 || ch_val > HIGH_CURR_CH_COUNT)
                return __scmd_ErrMsg("<em_dmm(error), HIGH_CURR_CH over range (1-15).\r\n");

            ret = meas_high_curr_ch((unsigned char)ch_val, &voltage);
            if (ret == -4) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) HIGH_CURR_CH unexpected\r\n");
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            if (ret == -5) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) HIGH_CURR_CH no channel selected (all IO=0)\r\n");
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            if (ret == -6) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) HIGH_CURR_CH multiple channels selected\r\n");
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            if (ret == -7) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) HIGH_CURR_CH wrong channel selected\r\n");
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            if (ret == -1) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) HIGH_CURR_CH IO state unknown (LV/HV?)\r\n");
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            if (ret == -2) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) HIGH_CURR_CH yf connect failed\r\n");
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            if (ret == -3) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) HIGH_CURR_CH LV path needs switch matrix\r\n");
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            if (ret != 0) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) dvm read failed code=%d\r\n", ret);
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            {
                int mv = (int)(voltage * 1000.0f);
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(ok) HIGH_CURR_CH%d %dmV\r\n", (int)ch_val, mv);
            }
            scmd_callback(scmd_msgBuf, slen);
            return scmd_normal;
        }

        /* HIGH_VOLT_CHn */
        if ((p[0] == 'V' || p[0] == 'v') &&
            (p[1] == 'O' || p[1] == 'o') &&
            (p[2] == 'L' || p[2] == 'l') &&
            (p[3] == 'T' || p[3] == 't'))
        {
            p += 4;
            /* skip optional "_CH" */
            if (*p == '_') p++;
            if (*p == 'C' || *p == 'c') p++;
            if (*p == 'H' || *p == 'h') p++;

            p = str_GetHexDec(p, pEnd, &ch_val);
            if (p == NULL)
                return __scmd_ErrMsg("<em_dmm(error), HIGH_VOLT_CH number not found.\r\n");
            if (ch_val < 1 || ch_val > HIGH_VOLT_CH_COUNT)
                return __scmd_ErrMsg("<em_dmm(error), HIGH_VOLT_CH over range (1-28).\r\n");

            ret = meas_high_volt_ch((unsigned char)ch_val, &voltage);
            if (ret == -4) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) HIGH_VOLT_CH IO95=1 (unexpected for VOLT)\r\n");
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            if (ret == -5) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) HIGH_VOLT_CH no channel selected (all row IO=0)\r\n");
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            if (ret == -6) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) HIGH_VOLT_CH multiple channels selected\r\n");
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            if (ret == -7) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) HIGH_VOLT_CH wrong channel selected\r\n");
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            if (ret == -8) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) HIGH_VOLT_CH IO72 mismatch for this ch\r\n");
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            if (ret == -1) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) HIGH_VOLT_CH IO state unknown (LV/HV?)\r\n");
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            if (ret == -2) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) HIGH_VOLT_CH yf connect failed\r\n");
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            if (ret == -3) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) HIGH_VOLT_CH LV path needs switch matrix\r\n");
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            if (ret != 0) {
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(error) dvm read failed code=%d\r\n", ret);
                scmd_callback(scmd_msgBuf, slen);
                return scmd_normal;
            }
            {
                int mv = (int)(voltage * 1000.0f);
                slen += sprintf(scmd_msgBuf + slen,
                    "<em_dmm(ok) HIGH_VOLT_CH%d %dmV\r\n", (int)ch_val, mv);
            }
            scmd_callback(scmd_msgBuf, slen);
            return scmd_normal;
        }
    }

    return __scmd_ErrMsg("<em_dmm(error), unknown format. Use Xn, HIGH_CURR_CHn, or HIGH_VOLT_CHn.\r\n");
}

/* ---- help / info ---- */

static scmd_errCode_def __help(char *pData, unsigned short len)
{
    dmm_ctrler.msgSource = scmd_ctrl.msgSource;
    __scmd_help(&dmm_ctrler, pData, len);
    return scmd_normal;
}

static scmd_errCode_def __info(char *pData, unsigned short len)
{
    unsigned short slen = 0;
    slen += sprintf(scmd_msgBuf + slen, "<em_dmm info:\r\n");
    slen += sprintf(scmd_msgBuf + slen, "  Xn             — X:1-300, route X->Y6->T13->DVM CH2\r\n");
    slen += sprintf(scmd_msgBuf + slen, "  HIGH_CURR_CHn  — n:1-15, set ch IOs first. IO79=1→LV(Y8→DVM CH2) IO79=0,87=1,77=0→HV(DVM CH1)\r\n");
    slen += sprintf(scmd_msgBuf + slen, "  HIGH_VOLT_CHn  — n:1-28, set ch IOs first. IO78=1→LV(Y7→DVM CH2) IO78=0,87=0,77=0→HV(DVM CH1)\r\n");
    scmd_callback(scmd_msgBuf, slen);
    return scmd_normal;
}
