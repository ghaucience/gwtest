#ifndef __CMD_H_
#define __CMD_H_
#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push)
#pragma pack(1)
  typedef struct ReqMac {
    char yummy;
  }ReqMac_t;
  typedef struct ResMac {
    char ret;
    char mac[6];
  }ResMac_t;

  typedef struct ReqSysVersion {
    char yummy;
  }ReqSysVersion_t;
  typedef struct ResSysVersion {
    char ret;
	char version[64];
  }ResSysVersion_t;

  typedef struct ReqZbVersion {
    char yummy;
  }ReqZbVersion_t;
  typedef struct ResZbVersion {
    char ret;
    char version[64];
  }ResZbVersion_t;

  typedef struct ReqZwVersion {
    char yummy;
  }ReqZwVersion_t;
  typedef struct ResZwVersion {
    char ret;
    char version[64];
  }ResZwVersion_t;


  typedef struct ReqModel {
    char yummy;
  }ReqModel;
  typedef struct ResModel {
    char ret;
    char model[64];
  }ResModel_t;

  typedef struct ReqWanDhcpCli {
    char yummy;
  }ReqWanDhcpCli_t;
  typedef struct ResWanDhcpCli {
    char ret;
  }ResWanDhcpCli_t;

  typedef struct ReqWanPingGw {
    char yummy;
  }ReqWanPingGw_t;
  typedef struct ResWanPingGw {
    char ret;
  }ResWanPingGw_t;
  
  typedef struct ReqWanDnsPing {
    char yummy;
  }ReqWanDnsPing_t;
  typedef struct ResWanDnsPing {
    char ret;
  }ResWanDnsPing_t;
  

  typedef struct ReqWifiSta {
    char yummy;
  }ReqWifiSta_t;
  typedef struct ResWifiSta {
    char ret;
  }ResWifiSta_t;

  typedef struct ReqWifiAp {
    char yummy;
  }ReqWifiAp_t;
  typedef struct ResWifiAp {
    char ret;
  }ResWifiAp_t;

  typedef struct ReqWifiSmartConfig {
    char yummy;
  }ReqWifiSmartConfig_t;
  typedef struct ResWifiSmartConfig {
    char ret;
  }ResWifiSmartConfig_t;

  
  typedef struct Req4GUsbDevice {
    char yummy;
  }Req4GUsbDevice_t;
  typedef struct Res4GUsbDevice {
    char ret;
  }Res4GUsbDevice_t;

  typedef struct Req4GAtCmd {
    char yummy;
  }Req4GAtCmd_t;
  typedef struct Res4GAtCmd {
    char ret;
  }Res4GAtCmd_t;

  typedef struct ReqSimCardChk {
    char yummy;
  }ReqSimCardChk_t;
  typedef struct ResSimCardChk {
    char ret;
  }ResSimCardChk_t;


  typedef struct ReqZigbeePair {
    char yummy;
  }ReqZigbeePair_t;
  typedef struct ResZigbeePair {
    char ret;
  }ResZigbeePair_t;

  typedef struct ReqZWavePair {
    char yummy;
  }ReqZWavePair_t;
  typedef struct ResZWavePair {
    char ret;
  }ResZWavePair_t;

  typedef struct ReqBleDevExsit {
    char yummy;
  }ReqBleDevExsit_t;
  typedef struct ResBleDevExsit {
    char ret;
  }ResBleDevExsit_t;

  typedef struct ReqBleScan {
    char yummy;
  }ReqBleScan_t;
  typedef struct ResBleScan {
    char ret;
  }ResBleScan_t;

  typedef struct ReqBtnPressDown {
    char yummy;
  }ReqBtnPressDown_t;
  typedef struct ResBtnPressDown {
    char ret;
  }ResBtnPressDown_t;

  typedef struct ReqLedPowerLed {
    char yummy;
  }ReqLedPowerLed_t;
  typedef struct ResLedPowerLed {
    char ret;
  }ResLedPowerLed_t;

  typedef struct ReqLedZWaveLed {
    char yummy;
  }ReqLedZWaveLed_t;
  typedef struct ResLedZWaveLed {
    char ret;
  }ResLedZWaveLed_t;

  typedef struct ReqLedZigbeeLed {
    char yummy;
  }ReqLedZigbeeLed_t;
  typedef struct ResLedZigbeeLed {
    char ret;
  }ResLedZigbeeLed_t;
  
  typedef struct ReqLed4GLed {
    char yummy;
  }ReqLed4GLed_t;
  typedef struct ResLed4GLed {
    char ret;
  }ResLed4GLed_t;

  typedef struct ReqLedErrorLed {
    char yummy;
  }ReqLedErrorLed_t;
  typedef struct ResLedErrorLed {
    char ret;
  }ResLedErrorLed_t;

  typedef struct ReqNxp {
    char yummy;
  } ReqNxp_t;
  typedef struct ResNxp {
    char ret;
  }ResNxp_t;
  
#pragma pack(pop)

  enum {
    CMD_REQUEST_MAC = 0x0001,
    CMD_REQUEST_SYSVERSION = 0x0002,
    CMD_REQUEST_ZBVERSION = 0x0003,
    CMD_REQUEST_ZWVERSION = 0x0004,
    CMD_REQUEST_MODEL = 0x0005,
    
    CMD_REQUEST_WAN_DHCPCLI = 0x0101,
    CMD_REQUEST_WAN_PING_GW = 0x0102,
    CMD_REQUEST_WAN_DNS_PING = 0x0103,
		
    CMD_REQUEST_WIFI_STA = 0x0201,
    CMD_REQUEST_WIFI_AP = 0x0202,
    CMD_REQUEST_WIFI_SMARTCONFIG = 0x0203,
		
    CMD_REQUEST_4G_USB_DEVICE = 0x0301,
    CMD_REQUEST_4G_AT_CMD = 0x0302,
    CMD_REQUEST_SIM_CARD_CHK = 0x0303,

    CMD_REQUEST_ZIGBEE_PAIR = 0x0401,

    CMD_REQUEST_ZWAVE_PAIR = 0x0501,

    CMD_REQUEST_BLE_DEV_EXSIT = 0x0601,
    CMD_REQUEST_BLE_SCAN = 0x0602,
    CMD_REQUEST_BTN_PRESSDOWN = 0x0603,

    CMD_REQUEST_LED_POWERLED = 0x0701,
    CMD_REQUEST_LED_ZWAVELED = 0x0702,
    CMD_REQUEST_LED_ZIGBEELED = 0x0703,
    CMD_REQUEST_LED_4GLED = 0x0704,
    CMD_REQUEST_LED_ERRORLED = 0x0705,

    CMD_REQUEST_NXP = 0x0801,
  };

  enum {
    E_OK = 0x0,
    E_ERROR = 0x1,
  };

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

  int cmd_request_nxp_pair(char *buff);

#ifdef __cplusplus
}
#endif
#endif
