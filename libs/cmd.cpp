#include "stdafx.h"
#include "cmd.h"
#include "socket.h"
#include "proto.h"

extern SOCKET g_sock;

static int cmd_frame_send(char cmd1, char cmd2, char *data, int len) {

	char buf[256];
	int flen = proto_frame_set((unsigned char *)buf, cmd1, cmd2, len, (unsigned char *)data);
	socket_send(g_sock, buf, flen, 80);

	return 0;
}
static int cmd_frame_wait(char *frame, int len, int ms) {
	return socket_recv(g_sock, frame, len, ms); 
}

int cmd_request_mac(char *buff) {
	char frame[256];
	cmd_frame_send(0x01, 0x02, NULL, 0);

	int len = cmd_frame_wait(frame, sizeof(frame)/sizeof(frame[0]), 4000);

	if (len == 0) {
		return 1;
	}

	strcpy(buff, "0102030405060708");
	return 0;
}
int cmd_request_sysversion(char *buff) {
	strcpy(buff, "V1.0.0.0");
	return 0;
}
int cmd_request_zbversion(char *buff) {
	strcpy(buff, "V1.0.0.0");
	return 0;
}
int cmd_request_zwversion(char *buff) {
	strcpy(buff, "V1.0.0.0");
	return 0;
}
int cmd_request_model(char *buff) {
	strcpy(buff, "DSI-0012");
	return 0;
}
int cmd_wan_dhcpcli(char *buff) {
	return 0;
}
int cmd_request_wan_ping_gw(char *buff) {
	return 0;
}
int cmd_request_wan_dns_ping(char *buff) {
	return 0;
}
int cmd_request_wifi_sta(char *buff) {
	return 0;
}
int cmd_request_wifi_ap(char *buff) {
	return 0;
}
int cmd_request_wifi_smartconfig(char *buff) {
	return 0;
}
int cmd_request_4g_usb_device(char *buff) {
	return 0;
}
int cmd_request_4g_at_cmd(char *buff) {
	return 0;
}
int cmd_request_sim_card_chk(char *buff) {
	return 0;
}
int cmd_request_zigbee_pair(char *buff) {
	return 0;
}
int cmd_request_zwave_pair(char *buff) {
	return 0;
}
int cmd_request_ble_dev_exsit(char *buff) {
	return 0;
}

int cmd_request_ble_scan(char *buff) {
	return 0;
}
int cmd_request_btn_pressdown(char *buff) {
	return 0;
}
int cmd_request_led_powerled(char *buff) {
	return 0;
}
int cmd_request_led_zwaveled(char *buff) {
	return 0;
}
int cmd_request_led_zigbeeled(char *buff) {
	return 0;
}
int cmd_request_led_4gled(char *buff) {
	return 0;
}
int cmd_request_led_errorled(char *buff) {
	return 0;
}