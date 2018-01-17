#include "stdafx.h"
#include "cmd.h"
#include "socket.h"
#include "proto.h"

extern SOCKET g_sock;

extern void test_info_append_hex(const char *X, char *buf, int len);

static int cmd_frame_send(short cmd, char *data, int len) {

	char tmpbuf[128];
	while (socket_recv(g_sock, tmpbuf, sizeof(tmpbuf), 1000) > 0) {

	}

	char buf[256];
	char cmd1 = (cmd >>8)&0xff;
	char cmd2 = (cmd>>0)&0xff;
	int flen = proto_frame_set((unsigned char *)buf, cmd1, cmd2, len, (unsigned char *)data);
	socket_send(g_sock, buf, flen, 80);
	test_info_append_hex("SEND:", buf, flen);

	

	return 0;
}
static int cmd_frame_wait(char *frame, int len, int ms) {
	int ret = socket_recv(g_sock, frame, len, ms); 
	if (ret > 0) {
		//proto_frame_set();
		test_info_append_hex("RECV:", frame, ret);
	}
	return ret;
}

int cmd_request_mac(char *buff) {
	char frame[256];
	cmd_frame_send(CMD_REQUEST_MAC, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 8000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_MAC|0x8000))  {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResMac_t *res = (ResMac_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}

	int i = 0;
	int pos = 0;
	for (i = 0; i < sizeof(res->mac); i++) {
	  pos += sprintf(buff + pos, "%02X", res->mac[i]&0xff);
	}
	//strcpy(buff, "0102030405060708");
	
	return 0;
}


int cmd_request_sysversion(char *buff) {
	char frame[256];
	cmd_frame_send(CMD_REQUEST_SYSVERSION, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_SYSVERSION|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResSysVersion_t *res = (ResSysVersion_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}

	strncpy(buff, res->version, sizeof(res->version));
	//strcpy(buff, "V1.0.0.0");
	
	return 0;
}
int cmd_request_zbversion(char *buff) {

	char frame[256];
	
	cmd_frame_send(CMD_REQUEST_ZBVERSION, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_ZBVERSION|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResZbVersion_t *res = (ResZbVersion_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}

	strncpy(buff, res->version, sizeof(res->version));
	//strcpy(buff, "V1.0.0.0");
	return 0;
}
int cmd_request_zwversion(char *buff) {
	char frame[256];
	
	cmd_frame_send(CMD_REQUEST_ZWVERSION, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_ZWVERSION|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResZwVersion_t *res = (ResZwVersion_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}

	strncpy(buff, res->version, sizeof(res->version));
	//strcpy(buff, "V1.0.0.0");
	return 0;
}
int cmd_request_model(char *buff) {
  	char frame[256];
	
	cmd_frame_send(CMD_REQUEST_MODEL, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_MODEL|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResModel_t *res = (ResModel_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}

	strncpy(buff, res->model, sizeof(res->model));
	//strcpy(buff, "DSI-0012");
	return 0;
}
int cmd_wan_dhcpcli(char *buff) {
    	char frame[256];
	
	cmd_frame_send(CMD_REQUEST_WAN_DHCPCLI, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_WAN_DHCPCLI|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResWanDhcpCli_t *res = (ResWanDhcpCli_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}

	return 0;
}
int cmd_request_wan_ping_gw(char *buff) {
      	char frame[256];
	
	cmd_frame_send(CMD_REQUEST_WAN_PING_GW, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_WAN_PING_GW|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResWanPingGw_t *res = (ResWanPingGw_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}

	return 0;
}
int cmd_request_wan_dns_ping(char *buff) {
        	char frame[256];
	
	cmd_frame_send(CMD_REQUEST_WAN_DNS_PING, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_WAN_DNS_PING|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResWanDnsPing_t *res = (ResWanDnsPing_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}

	return 0;
}
int cmd_request_wifi_sta(char *buff) {
          	char frame[256];
	
	cmd_frame_send(CMD_REQUEST_WIFI_STA, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_WIFI_STA|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResWifiSta_t *res = (ResWifiSta_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}

	return 0;
}
int cmd_request_wifi_ap(char *buff) {
  char frame[256];
	
	cmd_frame_send(CMD_REQUEST_WIFI_AP, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_WIFI_AP|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResWifiAp_t *res = (ResWifiAp_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}

	return 0;
}

int cmd_request_wifi_smartconfig(char *buff) {
    char frame[256];
	
	cmd_frame_send(CMD_REQUEST_WIFI_SMARTCONFIG, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 35000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_WIFI_SMARTCONFIG|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResWifiSmartConfig_t *res = (ResWifiSmartConfig_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}
	return 0;
}
int cmd_request_4g_usb_device(char *buff) {
      char frame[256];
	
	cmd_frame_send(CMD_REQUEST_4G_USB_DEVICE, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 15000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_4G_USB_DEVICE|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	Res4GUsbDevice_t *res = (Res4GUsbDevice_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}
	return 0;
}
int cmd_request_4g_at_cmd(char *buff) {
        char frame[256];
	
	cmd_frame_send(CMD_REQUEST_4G_AT_CMD, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 20000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_4G_AT_CMD|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	Res4GAtCmd_t *res = (Res4GAtCmd_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}
	return 0;
}
int cmd_request_sim_card_chk(char *buff) {
          char frame[256];
	
	cmd_frame_send(CMD_REQUEST_SIM_CARD_CHK, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 20000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_SIM_CARD_CHK |0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResSimCardChk_t *res = (ResSimCardChk_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}
	return 0;
}
int cmd_request_zigbee_pair(char *buff) {
            char frame[256];
	
	cmd_frame_send(CMD_REQUEST_ZIGBEE_PAIR, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_ZIGBEE_PAIR|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResZigbeePair_t *res = (ResZigbeePair_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}
	return 0;
}
int cmd_request_zwave_pair(char *buff) {
              char frame[256];
	
	cmd_frame_send(CMD_REQUEST_ZWAVE_PAIR, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_ZWAVE_PAIR|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResZWavePair_t *res = (ResZWavePair_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}
	return 0;
}
int cmd_request_ble_dev_exsit(char *buff) {
  char frame[256];
	
	cmd_frame_send(CMD_REQUEST_BLE_DEV_EXSIT, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_BLE_DEV_EXSIT|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResBleDevExsit_t *res = (ResBleDevExsit_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}
	return 0;
}

int cmd_request_ble_scan(char *buff) {
    char frame[256];
	
	cmd_frame_send(CMD_REQUEST_BLE_SCAN, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_BLE_SCAN|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResBleScan_t *res = (ResBleScan_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}
	return 0;
}
int cmd_request_btn_pressdown(char *buff) {
      char frame[256];
	
	cmd_frame_send(CMD_REQUEST_BTN_PRESSDOWN, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 15000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_BTN_PRESSDOWN|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResBtnPressDown_t *res = (ResBtnPressDown_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}
	return 0;
}
int cmd_request_led_powerled(char *buff) {
        char frame[256];
	
	cmd_frame_send(CMD_REQUEST_LED_POWERLED, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_LED_POWERLED|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResLedPowerLed_t *res = (ResLedPowerLed_t *)data;

	if (res->ret != E_OK) {
	  return 3;
	}
	return 0;
}
int cmd_request_led_zwaveled(char *buff) {
          char frame[256];
	
	cmd_frame_send(CMD_REQUEST_LED_ZWAVELED, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_LED_ZWAVELED|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResLedZWaveLed_t *res = (ResLedZWaveLed_t*)data;

	if (res->ret != E_OK) {
	  return 3;
	}
	return 0;
}
int cmd_request_led_zigbeeled(char *buff) {
            char frame[256];
	
	cmd_frame_send(CMD_REQUEST_LED_ZIGBEELED, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_LED_ZIGBEELED|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResLedZigbeeLed_t *res = (ResLedZigbeeLed_t*)data;

	if (res->ret != E_OK) {
	  return 3;
	}
	return 0;
}
int cmd_request_led_4gled(char *buff) {
              char frame[256];
	
	cmd_frame_send(CMD_REQUEST_LED_4GLED, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_LED_4GLED|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResLed4GLed_t *res = (ResLed4GLed_t*)data;

	if (res->ret != E_OK) {
	  return 3;
	}
	return 0;
}
int cmd_request_led_errorled(char *buff) {
                char frame[256];
	
	cmd_frame_send(CMD_REQUEST_LED_ERRORLED, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len <= 0) {
	  return 1;
	}

	if (proto_frame_get_cmd((u8*)frame) != (CMD_REQUEST_LED_ERRORLED|0x8000) ) {
	  return 2;
	}
	char *data = (char*)proto_frame_get_data((u8*)frame);
	ResLedErrorLed_t *res = (ResLedErrorLed_t*)data;

	if (res->ret != E_OK) {
	  return 3;
	}
	return 0;
}
