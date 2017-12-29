#ifndef __CMD_H_
#define __CMD_H_
#ifdef __cplusplus
extern "C" {
#endif

	int cmd_request_mac(char *buff);
		int cmd_request_sysversion(char *buff);
		int cmd_request_zbversion(char *buff);
		int cmd_request_zwversion(char *buff);
		int cmd_request_model(char *buff);
		int cmd_wan_dhcpcli(char *buff);
		int cmd_request_wan_ping_gw(char *buff);
		int cmd_request_wan_dns_ping(char *buff);
		int cmd_request_wifi_sta(char *buff);
		int cmd_request_wifi_ap(char *buff);
		int cmd_request_wifi_smartconfig(char *buff);
		int cmd_request_4g_usb_device(char *buff);
		int cmd_request_4g_at_cmd(char *buff);
		int cmd_request_sim_card_chk(char *buff);
		int cmd_request_zigbee_pair(char *buff);
		int cmd_request_zwave_pair(char *buff);
		int cmd_request_ble_dev_exsit(char *buff);

		int cmd_request_ble_scan(char *buff);
		int cmd_request_btn_pressdown(char *buff);
		int cmd_request_led_powerled(char *buff);
		int cmd_request_led_zwaveled(char *buff);
		int cmd_request_led_zigbeeled(char *buff);
		int cmd_request_led_4gled(char *buff);
		int cmd_request_led_errorled(char *buff);

#ifdef __cplusplus
}
#endif
#endif