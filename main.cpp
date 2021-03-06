#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "socket.h"
#include "proto.h"
#include "sheet.h"
#include "cmd.h"
#include "ssh2.h"
#include "visax.h"

#include <iostream>
#include <string.h>


#include <Commctrl.h>
#include "windows.h"

#include "resource.h"

using namespace std;

#define main_printf printf

#define MAX_LOADSTRING 100

enum {
  S_DISCONNECTTED = 0,
  S_CONNECTTED_IDLE = 1,
  S_TESTING = 2,
};

typedef int (*TESTFUNC)(void *arg);

typedef struct TestOption {
  int id;
  const char *name;
  int checked;
  const char *desc;
  TESTFUNC tfunc;
  int retval;
  char strval[512];
}TestOption_t;

typedef struct BoardInfo {
  char mac[32];
  char sysversion[256];
  char zbversion[32];
  char zwversion[32];
  char model[256];
}BoardInfo_t;

static char id2_data_str[4096];
static int id2_data_str_len = 0;

static HINSTANCE g_hinst = NULL;
static HWND g_hwnd = NULL;
SOCKET g_sock = NULL;
static HWND g_hpirwnd = NULL;
static HMENU g_hmenu = NULL;


static int state_convert(int state);
static int target_connect();
static int target_disconnect();
static void test_option_get();
static void target_test();
static void test_write_result();
static void test_info_append(const char *str);
static void test_info_clear();
static void test_select_all();


static int test_lan(void *arg);
static int test_mac(void *arg);
static int test_sysversion(void *arg);
static int test_zbversion(void *arg);
static int test_zwversion(void *arg);
static int test_model(void *arg);
static int test_time(void *arg);

static int test_wan_dhcpcli(void *arg);
static int test_wan_ping_gw(void *arg);
static int test_wan_dns_ping(void *arg);
static int test_wifi_sta(void *arg);
static int test_wifi_ap(void *arg);
static int test_wifi_smartconfig(void *arg);
static int test_4g_usb_device(void *arg);
static int test_4g_at_cmd(void *arg);
static int test_sim_card_chk(void *arg);
static int test_zigbee_pair(void *arg);
static int test_zwave_pair(void *arg);
static int test_ble_dev_exsit(void *arg);
static int test_ble_scan(void *arg);
static int test_btn_pressdown(void *arg);
static int test_led_powerled(void *arg);
static int test_led_zwaveled(void *arg);
static int test_led_zigbeeled(void *arg);
static int test_led_4gled(void *arg);
static int test_led_errorled(void *arg);
static int test_nxp_pair(void *arg);
static int test_led_allled(void *arg);
static int id2_program(void *arg);


static int test_write_option_result_string(TestOption_t *to, int ret);

static int pir_test();
static void gw_test_pic_init(HWND hwnd);
static void gw_test_pic_set(int type);

static int  id2_sdk_pc_program();

static TestOption_t g_tos[] = {
  {0, "TIME", 0, "", test_time, -1,},
  {TXT_MAC,"MAC", 0, "", test_mac, -1 },
  {TXT_SYSVERSION,"SysVersion", 0, "", test_sysversion, -1 },
  //{TXT_ZBVERSION,"ZBVersion", 0, "", test_zbversion, -1 },
  //{TXT_ZWVERSION,"ZWVersion", 0, "", test_zwversion, -1 },
  {TXT_MODEL,"Model", 0, "", test_model, -1 },

  { CHK_LAN,"LAN 连接性", 0, "", test_lan, -1,{ 0 } },

  {CHK_WAN_DHCPCLI, "WAN DHCP CLI", 0, "", test_wan_dhcpcli, -1},
  {CHK_WAN_PING_GW, "WAN Ping Gw", 0, "", test_wan_ping_gw, -1},
  {CHK_WAN_DNS_PING, "WAN DNS PING", 0, "",test_wan_dns_ping, -1},

  //{CHK_WIFI_STA, "STA", 0, "", test_wifi_sta, -1},
  //{CHK_WIFI_AP, "AP", 0, "", test_wifi_ap, -1},
  {CHK_WIFI_SMARTCONFIG, "SmartConfig", 0, "", test_wifi_smartconfig, -1},

  {CHK_4G_USB_DEVICE, "4G USB Device", 0, "", test_4g_usb_device, -1},
  {CHK_4G_AT_CMD, "4G AT CMD", 0, "", test_4g_at_cmd, -1},
  {CHK_SIM_CARD_CHK, "SIM CARD CHK", 0, "", test_sim_card_chk, -1},

  {CHK_ZIGBEE_PAIR, "Zigbee Work", 0, "", test_zigbee_pair, -1},
  {CHK_ZWAVE_PAIR, "ZWave Work", 0, "", test_zwave_pair, -1},
  
  {CHK_BLE_DEV_EXSIT, "BLE Dev Exsit", 0, "", test_ble_dev_exsit, -1},
  //{CHK_BLE_SCAN, "BLE Scan", 0, "", test_ble_scan, -1},

  {CHK_BTN_PRESSDOWN, "PressDown", 0, "", test_btn_pressdown, -1},

  {CHK_LED_ALLLED, "All Led", 0, "", test_led_allled, -1},
  /*
  {CHK_LED_POWERLED, "Power Led", 0, "", test_led_powerled, -1},
  {CHK_LED_ZWAVELED, "ZWave Led", 0, "", test_led_zwaveled, -1},
  {CHK_LED_ZIGBEELED, "Zigbee Led", 0, "", test_led_zigbeeled, -1},
  {CHK_LED_4GLED, "4G Led", 0, "", test_led_4gled, -1},
  {CHK_LED_ERRORLED, "Err Led", 0, "", test_led_errorled, -1 },
  */

  {CHK_NXP_PAIR, "Nxp Work", 0, "", test_nxp_pair, -1},
  {8000, "ID2 烧录", 0, "", id2_program, -1},
};


static BoardInfo_t bi = {
  "Unknown",
  "Unknown",
  "Unknown",
  "Unknown",
  "Unknown",
};

static char g_gwip[32] = "192.168.66.1";
static int g_gwport = 6666;
static char rootDir[1024];

static int test_amber = 1; // 0->amber, 1->choujian
static int test_nxp = 0; // 0->amber, 1->choujian
static int test_ble = 0;
static int shoujian = 0;
static float amber_power_limit = 5.0f;
static int amber_test_interface = 0;
static int amber_test_com = 0;

INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SetIp(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PirTest(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK STTest(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int state = S_DISCONNECTTED;

int pir_test_resouse_init();
int visa_test();
int visa_find();
int visa_class_test();
int visa_class_test_1();
void print_mac(string mac);
bool visa_amber_rf_power_test(double &power, double &frequence, double &delt_frequence, string &mac);
bool visa_nxp_rf_power_test(double &power, double &frequence, double &delt_frequence, string &mac);
bool visa_ble_rf_power_test(double &power, double &frequence, double &delt_frequence, string &mac);
bool visa_amber_commander_get_tune(unsigned short &tune);
int main(int argc, char *argv[]) {
  GetModuleFileName(NULL, rootDir, sizeof(rootDir));
  char *p = strrchr(rootDir, '\\');
  if (p != NULL) {
    *p = 0;
  }
  cout << "RootDir:" << rootDir << endl;

  /*
  unsigned short x;
    if (!visa_amber_commander_get_tune(x)) {
      cout << "read tune error" << endl;
      return 0;
    }
  cout << "tune is" << hex << x << endl;
  return  0;
  */

/*
int APIENTRY WinMainCRTStartup(_In_ HINSTANCE hInstance,
		      _In_opt_ HINSTANCE hPrevInstance,
		      _In_ LPWSTR    lpCmdLine,
		      _In_ int       nCmdShow) {
*/

  //UNREFERENCED_PARAMETER(hPrevInstance);
  //UNREFERENCED_PARAMETER(lpCmdLine);

  //g_hinst = hInstance;
  g_hinst = GetModuleHandle(NULL);

  //visa_test();
  //visa_find();
  //visa_class_test();
  //visa_class_test_1();  
  //return 0;
  //print_mac("00158d0000f3f4");
  //return 0;

  if (pir_test_resouse_init() != 0) {
    main_printf("Error When load pir Test Resourse!");
    return 0;
  }
  
  int ret = DialogBox(NULL, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc);
  if (ret == -1) {
    main_printf("Error is %d\n", GetLastError());
  }

  return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		g_hwnd = hwndDlg;
		g_hmenu = GetMenu(g_hwnd);
		socket_init();
		//state_convert(S_DISCONNECTTED);
		state_convert(S_CONNECTTED_IDLE);
		SendMessage(GetDlgItem(g_hwnd, PROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, 100)); //设置进度条的范围
		SendMessage(GetDlgItem(g_hwnd, PROGRESS), PBS_MARQUEE, 1, 0); //设置PBS_MARQUEE 是滚动效果 
		test_select_all();

		SetDlgItemText(g_hwnd, IDC_IPP, (LPCTSTR)(g_gwip));
		{
		  char buf[32];
		  sprintf(buf, "%d", g_gwport);
		  SetDlgItemText(g_hwnd, IDC_PORTP, (LPCTSTR)buf);
		}

		{
		  gw_test_pic_init(g_hwnd);
		}

		gw_test_pic_set(0);
		
		return TRUE;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		socket_uninit();
		return TRUE;

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);
		if (wmId == IDM_ABOUT) {
			DialogBox(g_hinst, MAKEINTRESOURCE(IDD_ABOUTBOX), g_hwnd, About);
			return TRUE;
		}
		else if (wmId == IDM_SETIP) {
			DialogBox(g_hinst, MAKEINTRESOURCE(IDD_SETIP), g_hwnd, SetIp);
			return TRUE;
		}
		else if (wmId == IDM_EXIT) {
			EndDialog(hwndDlg, 0);
			socket_uninit();
			return TRUE;
		}
		else if (wmId == IDM_ZWPIR_TEST) {
		  DialogBox(g_hinst, MAKEINTRESOURCE(IDD_PIRTEST), g_hwnd, PirTest);
		  return TRUE;		  
		} else if (wmId == IDM_PROGRAM_ID2) {
		  UINT flag = GetMenuState(g_hmenu,  IDM_PROGRAM_ID2, MF_BYCOMMAND);
		  cout << "ID2 MENU FLAG :" << hex << flag << endl;
		    if (flag & MF_CHECKED) {
		      flag = MF_BYCOMMAND | MF_UNCHECKED;
		    } else {
		      flag = MF_BYCOMMAND | MF_CHECKED;
		    }
		    CheckMenuItem(g_hmenu, IDM_PROGRAM_ID2, flag);
		    return TRUE;
		} else if (wmId == IDM_ST) {
		  //MessageBox(NULL, NULL, NULL, NULL);
		  DialogBox(g_hinst, MAKEINTRESOURCE(IDD_ST), g_hwnd, STTest);		  
		  return TRUE;
		}
		  // EnableMenuItem(hMenu,IDM_PROGRAM_ID2, flag);		  
	

#if 0		
		switch (state) {
		case S_DISCONNECTTED:
			switch (wmId) {
			case BTN_CONNECT:
#if 0
				xls_test();
#else
				if (target_connect() == 0) {
					state_convert(S_CONNECTTED_IDLE);
				}
				else {
					MessageBox(NULL, "Can't connect to server!", "Error", MB_OK);
				}
#endif
				break;
			}
			break;
		case S_CONNECTTED_IDLE:
			switch (wmId) {
			case BTN_DISCONNECT:
				target_disconnect();
				state_convert(S_DISCONNECTTED);
				break;
			case BTN_TEST:
				state_convert(S_TESTING);
				test_option_get();
				target_test();
				state_convert(S_CONNECTTED_IDLE);
				break;
			}
			break;
		case S_TESTING:
			switch (wmId) {
			case BTN_STOP:
				MessageBox(NULL, "Now not support stop!", "Info", MB_OK);
				break;
			}
			break;
		}
#else
		switch (wmId) {
		case BTN_TEST:
		  if (target_connect() == 0) {
		    state_convert(S_TESTING);
		    
		    test_option_get();
		    target_test();
		    //target_disconnect();
		    //state_convert(S_CONNECTTED_IDLE);
		  } else {
		    MessageBox(NULL, "Can't connect to server!", "Error", MB_OK);
		  }
		}
#endif	     
		return TRUE;
	}
	}
	return FALSE;
}



INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (message) {
  case WM_INITDIALOG:
    return (INT_PTR)TRUE;

  case WM_COMMAND:
    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
      EndDialog(hDlg, LOWORD(wParam));
      return (INT_PTR)TRUE;
    }
    break;
  }
  return (INT_PTR)FALSE;
}

INT_PTR CALLBACK SetIp(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == BTN_SETIP) {
			char ip[32];
			GetDlgItemTextA(hDlg, IDC_IP, ip, sizeof(ip));
			char port[32];
			GetDlgItemTextA(hDlg, IDC_PORT, port, sizeof(port));
			strcpy(g_gwip, ip);
			g_gwport = atoi(port);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			//MessageBoxA(NULL, ip, port, 0);
		}
		break;
	}
	return (INT_PTR)FALSE;
}



int target_connect() {
  //g_sock = socket_client_open("192.168.100.103", 6666);
	//g_sock = socket_client_open("192.168.66.1", 6666);
  char ip[32];
  char sport[32];
  GetDlgItemText(g_hwnd, IDC_IPP, ip, sizeof(ip));
  GetDlgItemText(g_hwnd, IDC_PORTP, sport, sizeof(sport));

  strcpy(g_gwip, ip);
  g_gwport = atoi(sport);

  cout << "ip:" << g_gwip << ",port:" << g_gwport << endl;
  g_sock = socket_client_open(g_gwip, g_gwport);
  //g_sock = socket_client_open("127.0.0.1", 9898);
  if (g_sock == NULL) {
    return -1;
  }
  return 0;
}

int target_disconnect() {
  cout << "target disconnect" << endl;
  if (g_sock != NULL) {
    socket_close(g_sock);
    g_sock = NULL;
  }
  return 0;
}

void test_option_get() {
  int i = 0;
  int cnt = sizeof(g_tos)/sizeof(g_tos[0]);
  int flag = 0;

  for (i = 0; i < cnt; i++) {
    if (g_tos[i].id == CHK_LAN) {
      flag = 1;
    }

    if (g_tos[i].id == 8000) {
      UINT flag = GetMenuState(g_hmenu,  IDM_PROGRAM_ID2, MF_BYCOMMAND);
      if (flag & MF_CHECKED) {
	g_tos[i].checked = TRUE;
      } else {
	g_tos[i].checked = FALSE;
      }
      continue;
    }

    g_tos[i].checked = 1;
    if (flag) {
      g_tos[i].checked = IsDlgButtonChecked(g_hwnd, g_tos[i].id);
    }
  }
}

void _target_test() {
  int i = 0;
  int cnt = sizeof(g_tos) / sizeof(g_tos[0]);

  int dlt = 100 / cnt;

  SendMessage(GetDlgItem(g_hwnd, PROGRESS), PBM_SETPOS, 1, (LPARAM)0);   //设置进度 
  test_info_clear();
  test_info_append("Start...");

  int item_cnt = 0;
  int result_fail_item_cnt = 0;
  gw_test_pic_set(0);  
  
  for (i = 0; i < cnt; i++) {
	char buff[256];

    g_tos[i].retval = -1;
    strcpy(g_tos[i].strval, "--");
    if (g_tos[i].checked == 0) {
      continue;
    }

    item_cnt++;
    if (g_tos[i].tfunc != NULL) {
	  sprintf(buff, "======Testing...【%s】======", g_tos[i].name);
	  test_info_append(buff);
      g_tos[i].retval = g_tos[i].tfunc(&g_tos[i]);
    }

 
     //test_write_option_result_string(&g_tos[i], g_tos[i].retval);
  


	if (g_tos[i].retval == 0) {
		sprintf(buff, "【%s】 √", g_tos[i].name);
	}
	else {
		sprintf(buff, "【%s】 ×", g_tos[i].name);
		result_fail_item_cnt++;
	}
	test_info_append(buff);
    SendMessage(GetDlgItem(g_hwnd, PROGRESS), PBM_SETPOS, i*dlt, (LPARAM)0);   //设置进度 
    //Sleep(100);
	//sprintf(buff, "==========", g_tos[i].name);
	//test_info_append(buff);
  }
  test_info_append("Write Result...");
  test_write_result();
  SendMessage(GetDlgItem(g_hwnd, PROGRESS), PBM_SETPOS, 100, (LPARAM)0);   //设置进度
  test_info_append("Write End!");

  /*
  test_info_append("======写入ID2数据======");
  UINT flag = GetMenuState(g_hmenu,  IDM_PROGRAM_ID2, MF_BYCOMMAND);
  if (flag & MF_CHECKED) {
    test_info_append("烧写ID2数据...");
    int ret = id2_sdk_pc_program();
    if (ret == 0) {
      test_info_append("烧写ID2数据OK");
      test_info_append("【写入ID2数据】√");              
    } else {
      test_info_append("烧写ID2数据失败");
      test_info_append("【写入ID2数据】x");
      result_fail_item_cnt++;      
    }
  }
  SendMessage(GetDlgItem(g_hwnd, PROGRESS), PBM_SETPOS, 100, (LPARAM)0);   //设置进度  
  */
  

  SendMessage(GetDlgItem(g_hwnd, PROGRESS), PBM_SETPOS, 0, (LPARAM)0);   //设置进度
  test_info_append("================================Test Result=============================");
  char xbuff[1024];
  sprintf(xbuff, "End!, Test %d Items, Failed: %d Items! (%d/%d)", item_cnt, result_fail_item_cnt, item_cnt  - result_fail_item_cnt, item_cnt);
  test_info_append(xbuff);
  if (result_fail_item_cnt == 0) {
    test_info_append(
		     " $$$$$$\\ $$\\   $$\\    \r\n"
		     "$$  __$$\\ $$ | $$  |      \r\n"
		     "$$ /  $$ |$$ |$$  /        \r\n"
		     "$$ |  $$ |$$$$$  /        \r\n"
		     "$$ |  $$ |$$  $$<         \r\n"
		     "$$ |  $$ |$$ |\\$$\\      \r\n"
		     " $$$$$$  |$$ | \\$$\\   \r\n"
		     " \\______/ \\__|  \\__|     ");
    gw_test_pic_set(1);
  } else {
    test_info_append(
		     "$$\\   $$\\       \r\n"
		     "$$ |  $$ |      \r\n"
		     "\\$$\\ $$  |      \r\n"
		     "\\$$$$  /       \r\n"
		     "$$  $$<        \r\n"
		     "$$  /\\$$\\       \r\n"
		     "$$ /  $$ |      \r\n"
		     "\\__|  \\__|     ");
    gw_test_pic_set(2);    
  }


}
DWORD WINAPI targt_test_thread(LPVOID lpParam) {
  _target_test();
  target_disconnect();
  state_convert(S_CONNECTTED_IDLE);
  return 0;
}
void target_test() {
  CreateThread(NULL, 0, targt_test_thread, NULL, 0, NULL);
}



static int state_convert(int _state) {
  state = _state;
  switch(state) {
  case S_DISCONNECTTED:
    //EnableWindow(GetDlgItem(g_hwnd, BTN_CONNECT), TRUE);
    //EnableWindow(GetDlgItem(g_hwnd, BTN_DISCONNECT), FALSE);
    //EnableWindow(GetDlgItem(g_hwnd, BTN_TEST), FALSE);
    //EnableWindow(GetDlgItem(g_hwnd, BTN_STOP), FALSE);
    break;
  case S_CONNECTTED_IDLE:
    //EnableWindow(GetDlgItem(g_hwnd, BTN_CONNECT), FALSE);
    //EnableWindow(GetDlgItem(g_hwnd, BTN_DISCONNECT), TRUE);
    EnableWindow(GetDlgItem(g_hwnd, BTN_TEST), TRUE);
    EnableWindow(GetDlgItem(g_hwnd, BTN_STOP), FALSE);
    break;
  case S_TESTING:
    //EnableWindow(GetDlgItem(g_hwnd, BTN_CONNECT), FALSE);
    //EnableWindow(GetDlgItem(g_hwnd, BTN_DISCONNECT), FALSE);
    EnableWindow(GetDlgItem(g_hwnd, BTN_TEST), FALSE);
    EnableWindow(GetDlgItem(g_hwnd, BTN_STOP), FALSE);
    break;
  }
  return 0;
}

static int test_time(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  time_t timer;
  struct tm *tblock;
  timer = time(NULL);
  tblock = localtime(&timer);

  strcpy(to->strval, asctime(tblock));

  return 0;
}
static int test_lan(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;
  int ret = 0;
  return test_write_option_result_string(to, ret);
}

static int test_mac(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  char val[128];
  int ret = cmd_request_mac(val);
  if (ret == 0) {
	  strcpy(to->strval, val);
	  SetDlgItemTextA(g_hwnd, TXT_MAC, to->strval);
  }


  return ret;
}
static int test_sysversion(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  char val[128];
  int ret = cmd_request_sysversion(val);
  if (ret == 0) {
	  strcpy(to->strval, val);
	  SetDlgItemTextA(g_hwnd, TXT_SYSVERSION, to->strval);
  }

  return ret;
}
static int test_zbversion(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  char val[128];
  int ret = cmd_request_zbversion(val);
  if (ret == 0) {
	  strcpy(to->strval, val);
	  SetDlgItemTextA(g_hwnd, TXT_ZBVERSION, to->strval);
  }

  return ret;
}
static int test_zwversion(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  char val[128];
  int ret = cmd_request_zwversion(val);
  if (ret == 0) {
	  strcpy(to->strval, val);
	  SetDlgItemTextA(g_hwnd, TXT_ZWVERSION, to->strval);
  }

  return ret;
}
static int test_model(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  char val[128];
  int ret = cmd_request_model(val);
  if (ret == 0) {
	  strcpy(to->strval, val);
	  SetDlgItemTextA(g_hwnd, TXT_MODEL, to->strval);
  }

  return ret;
}

static int test_wan_dhcpcli(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  char val[128];
  int ret = cmd_wan_dhcpcli(val);

  return test_write_option_result_string(to, ret);
}
static int test_wan_ping_gw(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  char val[128];
  int ret = cmd_request_wan_ping_gw(val);

  return test_write_option_result_string(to, ret);
}
static int test_wan_dns_ping(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  char val[128];
  int ret = cmd_request_wan_dns_ping(val);

  return test_write_option_result_string(to, ret);
}
static int test_wifi_sta(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  char val[128];
  int ret = cmd_request_wifi_sta(val);

  return test_write_option_result_string(to, ret);
}
static int test_wifi_ap(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  char val[128];
  int ret = cmd_request_wifi_ap(val);

  return test_write_option_result_string(to, ret);
}
static int test_wifi_smartconfig(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  MessageBox(g_hwnd, "请启动下手机端的SmartConfig 功能,进行SmartConfig测试, 并迅速点确定按钮!!!", "SmartConfig Test准备", MB_OK);
  char val[128];
  int ret = cmd_request_wifi_smartconfig(val);

  return test_write_option_result_string(to, ret);
}
static int test_4g_usb_device(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  char val[128];
  int ret = cmd_request_4g_usb_device(val);

  return test_write_option_result_string(to, ret);
}
static int test_4g_at_cmd(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  char val[128];
  int ret = cmd_request_4g_at_cmd(val);

  return test_write_option_result_string(to, ret);
}
static int test_sim_card_chk(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  char val[128];
  int ret = cmd_request_sim_card_chk(val);

  return test_write_option_result_string(to, ret);
}

static int test_zigbee_pair(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;
  //MessageBox(g_hwnd, "请按下Zigbee测试设备配网按钮,进行zigbee配网测试, 并迅速点确定按钮!!!", "Zigbee Test准备", MB_OK);

  char val[128];
  #if 0  
  int ret = cmd_request_zigbee_pair(val);
  #else
  int size = 0;
  int ret = libssh2_main(g_gwip, 22, "root", "root", "if [ -e /etc/config/dusun/amber/netinfo ]; then echo 1; else echo 0; fi", val, &size);
  if (ret == 0 && size == 2) {
    val[1] = 0;
    if (strcmp(val, "1") == 0) {
      ret = 0;
    } else {
      ret = 1;
    }
  } else {
    ret = 1;
  }
  #endif
  return test_write_option_result_string(to, ret);
}
static int test_zwave_pair(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;
  //MessageBox(g_hwnd, "请按下ZWave测试设备配网按钮 功能,进行zwave配网测试, 并迅速点确定按钮!!!", "ZWave Test准备", MB_OK);
  char val[128];
#if 0
  int ret = cmd_request_zwave_pair(val);
#else
  int size = 0;
  int ret = libssh2_main(g_gwip, 22, "root", "root", "if [ -e /etc/config/dusun/zwdev/netinfo ]; then echo 1; else echo 0; fi", val, &size);
  if (ret == 0 && size == 2) {
    val[1] = 0;
    if (strcmp(val, "1") == 0) {
      ret = 0;
    } else {
      ret = 1;
    }
  }  else {
    ret = 1;
  }

#endif
  cout << "ret:" << ret << endl;  
  return test_write_option_result_string(to, ret);
}
static int test_nxp_pair(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;
  char val[128];
  #if 0
  //MessageBox(g_hwnd, "请按下NXP配网按钮3秒后松开 ,进行NXP配网测试, 并迅速点确定按钮!!!", "NXP 配网准备", MB_OK);  
  int ret = cmd_request_nxp_pair(val);
#else
  int size = 0;
  //int ret = libssh2_main(g_gwip, 22, "root", "root", "if [ -e /etc/config/dusun/nxp/netinfo ]; then echo 1; else echo 0; fi", val, &size);
  int ret = libssh2_main(g_gwip, 3207, "root", "@_2020_linyux.com", "if [ -e /etc/config/dusun/nxp/netinfo ]; then echo 1; else echo 0; fi", val, &size);  
  if (ret == 0 && size == 2) {
    val[1] = 0;
    if (strcmp(val, "1") == 0) {
      ret = 0;
    } else {
      ret = 1;
    }
  }  else {
    ret = 1;
  }  
#endif
  return test_write_option_result_string(to, ret);
}

static int test_ble_dev_exsit(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;
  char val[128];
  int ret = cmd_request_ble_dev_exsit(val);
  return test_write_option_result_string(to, ret);
}
static int test_ble_scan(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;
  char val[128];
  int ret = cmd_request_ble_scan(val);
  return test_write_option_result_string(to, ret);
}
static int test_btn_pressdown(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;

  MessageBox(g_hwnd, "请点击确定按钮, 并迅速按下网关的按钮 ,进行按钮测试, !!!", "按钮 Test准备", MB_OK);  
  char val[128];
  int ret = cmd_request_btn_pressdown(val);
  return test_write_option_result_string(to, ret);
}
static int test_led_powerled(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;
  char val[128];
  int ret = cmd_request_led_powerled(val);
  return test_write_option_result_string(to, ret);
}

static int test_led_zwaveled(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;
  char val[128];
  int ret = cmd_request_led_zwaveled(val);
  return test_write_option_result_string(to, ret);
}
static int test_led_zigbeeled(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;
  char val[128];
  int ret = cmd_request_led_zigbeeled(val);
  return test_write_option_result_string(to, ret);
}
static int test_led_4gled(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;
  char val[128];
  int ret = cmd_request_led_4gled(val);
  return test_write_option_result_string(to, ret);
}
static int test_led_errorled(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;
  char val[128];
  int ret = cmd_request_led_errorled(val);
  return test_write_option_result_string(to, ret);
}
static int test_led_allled(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;
  char val[128];
  
  int ret = cmd_request_led_allled_start_blink(val);
  ret = MessageBox(g_hwnd, "是否网关正面所有的LED都在闪烁? 是 / 否?", "Led 测试..",  MB_YESNO);
  if (ret == IDYES) {
    ret = 0;
  } else {
    ret = 1;
  }
  cmd_request_led_allled_restore();
  
  return test_write_option_result_string(to, ret);  
}

static int id2_program(void *arg) {
  TestOption_t *to = (TestOption_t *)arg;
  //char val[128];

  int ret = id2_sdk_pc_program(); 
  return test_write_option_result_string(to, ret);    
}




static int test_write_option_result_string(TestOption_t *to, int ret) {
	//char buf[64];
	//sprintf(buf, "%d", ret);
	//MessageBoxA(g_hwnd, buf, buf, 0);
  if (ret == -1) {
    strcpy(to->strval, "--");
  }
  else if (ret == 0)  {
    strcpy(to->strval, "√");
  }
  else {
    strcpy(to->strval, "×");
  }
  if (ret == 0 && to->id == 8000) {
    sprintf(to->strval + strlen(to->strval), "|%s", id2_data_str);
    cout << to->strval << endl;
  }
  return ret;
}
static void test_year_day_str(char *ydstr, int len) {

  time_t timer;
  struct tm *tblock;
  timer = time(NULL);
  tblock = localtime(&timer);

  char buff[128];
  memset(ydstr, 0, len);
  sprintf(buff, "%d年%d月%d号-By_%s.xls", tblock->tm_year+1900, tblock->tm_mon+1, tblock->tm_mday, "Keven");
  
  strcpy(ydstr, buff);
  //MultiByteToWideChar(CP_ACP, 0, buff, strlen(buff), ydstr, len);
}



static void test_write_result() {
  int i = 0;
  int cnt = sizeof(g_tos) / sizeof(g_tos[0]);
  const char *path = ".";
  char name[128];
  test_year_day_str(name, 128);

  const char* cols[256];
  memset(cols, 0, sizeof(char *) * 256);

  for (i = 0; i < cnt; i++) {
    cols[i] = new char[512];
    memset((void *)cols[i], 0, sizeof(char) * 128);
    strcpy((char *)cols[i], g_tos[i].name);
    //MultiByteToWideChar(CP_ACP, 0, g_tos[i].name, strlen(g_tos[i].name), cols[i], 128);
    //wsprintf(cols[i], L"%s", g_tos[i].name);
  }
  BookHandle bh = sheet_open(path, name, "GwFunctionTest", 0, cols, cnt);

  if (bh == NULL) {
    for (i = 0; i < cnt; i++) {
      if (cols[i] != NULL) {
	delete cols[i];
      }
    }
    MessageBox(NULL, "Error", "open a.xls failed!", MB_OK);
    for (i = 0; i < cnt; i++) {
      if (cols[i] != NULL) {
	delete cols[i];
      }
    }
    return;
  }

  for (i = 0; i < cnt; i++) {
    memset((void *)cols[i], 0, sizeof(char) * 512);
    strcpy((char *)cols[i], g_tos[i].strval);
    //MultiByteToWideChar(CP_ACP, 0, g_tos[i].strval, strlen(g_tos[i].strval), cols[i], 128);
  }
  sheet_append(bh, 0, (const char **)cols, cnt);
  sheet_save(bh, path, name);
  sheet_close(bh);

  for (i = 0; i < cnt; i++) {
    if (cols[i] != NULL) {
      delete cols[i];
    }
  }
}

static string info_str = "";
  static int info_idx = 0;
static void test_info_append(const char *str) {
  //IDC_INFO
#if 1

  char buf[256];
  time_t timer;
  struct tm *tblock;
  timer = time(NULL);
  tblock = localtime(&timer);
  sprintf(buf, "[%02d:%02d:%02d.%d]:", tblock->tm_hour, tblock->tm_min, tblock->tm_sec, info_idx++);
  
  string info_tmp = info_str;
  info_str = std::string(buf);
  info_str += std::string(str);
  info_str += "\r\n";
  info_str += info_tmp;
  //info_str += str;
  //info_str += "\r\n";
  SetDlgItemText(g_hwnd, IDC_INFO, info_str.c_str());
  SendMessage(GetDlgItem(g_hwnd, IDC_INFO), EM_SETSEL, 0, 0);
#else
  char buf[256];
  time_t timer;
  struct tm *tblock;
  timer = time(NULL);
  tblock = localtime(&timer);

  sprintf(buf, "[INFO][%02d:%02d:%02d]:%s\r\n", tblock->tm_hour, tblock->tm_min, tblock->tm_sec, str);
  SendMessage(GetDlgItem(g_hwnd, IDC_INFO), EM_SETSEL, -1, 0);
  SendMessageA(GetDlgItem(g_hwnd, IDC_INFO), EM_REPLACESEL, 0, (LPARAM)(LPCSTR)buf);
#endif
}

extern void test_info_append_hex(const char *X, char *buf, int len) {
	char str[1024];
	int i = 0;
	int pos = 0;
	pos += sprintf(str + pos, "%s", X);
	for (i = 0; i < len; i++) {
		pos += sprintf(str + pos, "[%02X] ", buf[i] & 0xff);
	}
	test_info_append(str);
	return;
}

static void test_info_clear() {
#if 0
  SendMessage(GetDlgItem(g_hwnd, IDC_INFO), EM_SETSEL, -1, 0);
  SendMessageA(GetDlgItem(g_hwnd, IDC_INFO), WM_SETTEXT, 0, (LPARAM)(LPCSTR)"");
#else
  info_str = "";
  info_idx= 0;
  SetDlgItemText(g_hwnd, IDC_INFO, info_str.c_str());  
#endif
}

static void test_select_all() {
	int i = 0;
	int cnt = sizeof(g_tos) / sizeof(g_tos[0]);
	int flag = 0;

	for (i = 0; i < cnt; i++) {
		if (g_tos[i].id == CHK_LAN) {
			flag = 1;
		}

		g_tos[i].checked = 1;

		if (flag) {
			CheckDlgButton(g_hwnd, g_tos[i].id, BST_CHECKED);
		}
	}

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static HBITMAP hBmOk = NULL;
static BITMAP bmOk;
static HBITMAP hBmFd = NULL;
static BITMAP bmFd;
static HBITMAP hBmIl = NULL;
static BITMAP bmIl;
static HBITMAP hBmIl_pir = NULL;
static HDC pir_memdc = NULL;
static HBITMAP hBmDis = NULL;

static WNDPROC pir_fDefProc = NULL;
INT_PTR CALLBACK PirImgProc(HWND hwnd, UINT message,
			    WPARAM wParam, LPARAM lParam);


static void pir_test_clr_mac() {
  SetDlgItemText(g_hpirwnd, PIR_TEST_TXT_MAC, "");
}
static void pir_test_clr_status() {
  SetDlgItemText(g_hpirwnd, PIR_TEST_TXT_STATUS, "");
}
static void pir_test_set_prg(int val) {
  SendMessage(GetDlgItem(g_hpirwnd, PIR_TEST_PRG_TESTING), PBM_SETPOS, val%100, (LPARAM)0);   //设置进度   
}
static void pir_test_set_img(int type) {
  if (type == 0) {
    hBmDis = hBmIl_pir;
    InvalidateRect(GetDlgItem(g_hpirwnd, PIR_TEST_PIC_RESULT), NULL, TRUE);
  } else if (type == 1) {
    hBmDis = hBmOk;
    InvalidateRect(GetDlgItem(g_hpirwnd, PIR_TEST_PIC_RESULT), NULL, TRUE);    
  } else if (type == 2) {
    hBmDis = hBmFd;
    InvalidateRect(GetDlgItem(g_hpirwnd, PIR_TEST_PIC_RESULT), NULL, TRUE);    
  } else {
    hBmDis = 0;
  }
}
static string pir_info_str = "";
static void pir_test_clr_info() {
  pir_info_str = "";
  SetDlgItemText(g_hpirwnd, PIR_TEST_TXT_INFO, pir_info_str.c_str());
}

static void pir_test_disable_btn() {
  EnableWindow(GetDlgItem(g_hpirwnd, PIR_TEST_BTN_START), FALSE);
}
static void pir_test_enable_btn() {
  EnableWindow(GetDlgItem(g_hpirwnd, PIR_TEST_BTN_START), TRUE);  
}

static void pir_test_info(const char *str) {
  #if 0
  string x = pir_info_str;
  pir_info_str = std::string(str);
  pir_info_str += "\r\n";
  pir_info_str += x;
  #else
  pir_info_str += str;
  pir_info_str += "\r\n";
  #endif
  SetDlgItemText(g_hpirwnd, PIR_TEST_TXT_INFO, pir_info_str.c_str());
  SendMessage(GetDlgItem(g_hpirwnd, PIR_TEST_TXT_INFO), EM_SETSEL, 0, 0);  
}

static void pir_test_parse_mac(const char *buf, char *mac) {
  int llen = strlen("0000000000000000|00-00.00-00.00|00|0");  
  int len = strlen(buf);
  if (len == llen) {
    strncpy(mac, buf, 16);    
  } else {
    cout << "llen:" << llen << ",len:" << len << endl;
    strcpy(mac, "");
  }
}

static void pir_test_parse_status(const char *buf, char *status) {
  int llen = strlen("0000000000000000|00-00.00-00.00|00|0");
  int len = strlen(buf);  
  if (len == llen) {
    strncpy(status, buf + 16 + 1 + 14 + 1 + 2 + 1, 1);
  } else {
    cout << "llen:" << llen << ",len:" << len << endl;    
    strcpy(status, "");
  }
}
static void pir_test_set_mac(char *mac) {
  SetDlgItemText(g_hpirwnd, PIR_TEST_TXT_MAC, mac);
}
static void pir_test_set_status(char *status) {
  SetDlgItemText(g_hpirwnd, PIR_TEST_TXT_STATUS, status);  
}
static int pir_test_include_and_query_result(char *retbuf) {
  char val[4096];
  val[0] = 0;
  int ret = cmd_request_zwave_include_and_query(val);

  strcpy(retbuf, val);  
  return ret;
}
static int pir_test_exclude(char *retbuf) { 
  char val[4096];
  val[0] = 0;
  int ret = cmd_request_zwave_exclude(val);

  strcpy(retbuf, val);
  return ret;
}

DWORD WINAPI pir_test_thread(LPVOID lpParam) {
  if (target_connect() != 0) {
    return 0;
  }
  pir_test();  
  target_disconnect();
  return 0;
}
void pir_target_test() {
  CreateThread(NULL, 0, pir_test_thread, NULL, 0, NULL);
}

static void pir_test_write_result(char *mac, char *status, int result) {
  int i = 0;
  char *col_names[3] = {
    "MAC",
    "PIR STATUS",
    "测试结果",
  };
  int cnt = sizeof(col_names)/sizeof(col_names[0]);
  
  const char *path = ".";
  char name[128];
  test_year_day_str(name, 128);

  const char* cols[256];
  memset(cols, 0, sizeof(char *) * 256);

  for (i = 0; i < cnt; i++) {
    cols[i] = new char[128];
    memset((void *)cols[i], 0, sizeof(char) * 128);
    strcpy((char *)cols[i], col_names[i]);
  }
  
  BookHandle bh = sheet_open(path, name, "ZWavePirTest", 1, cols, cnt);
  if (bh == NULL) {
    for (i = 0; i < cnt; i++) {
      if (cols[i] != NULL) {
	delete cols[i];
      }
    }
    MessageBox(NULL, "Error", "open a.xls failed!", MB_OK);
    for (i = 0; i < cnt; i++) {
      if (cols[i] != NULL) {
	delete cols[i];
      }
    }
    return;
  }

  for (i = 0; i < cnt; i++) {
    memset((void *)cols[i], 0, sizeof(char) * 128);
    if (i == 0) {
      sprintf((char *)cols[i], "%s", mac);
    } else if (i == 1) {
      sprintf((char *)cols[i], "%s", status);      
    } else if (i == 2) {
      sprintf((char *)cols[i], "%d", result);            
    }
  }
  sheet_append(bh, 1, (const char **)cols, cnt);
  sheet_save(bh, path, name);
  sheet_close(bh);

  for (i = 0; i < cnt; i++) {
    if (cols[i] != NULL) {
      delete cols[i];
    }
  }
}

static int pir_test_prepare() {
  pir_test_clr_mac();
  pir_test_clr_status();
  pir_test_set_prg(0);
  pir_test_set_img(0);
  pir_test_clr_info();
  return 0;
}



static int pir_test() {
  pir_test_disable_btn();
  pir_test_prepare();

  pir_test_info("开启入网模式,请按下Pir的配网按钮并等待10秒左右获取结果...");
  MessageBox(g_hpirwnd, "开启入网模式,请按下Pir的配网按钮并等待10秒左右获取结果...", "Pir Test 信息", MB_OK);
  pir_test_set_prg(3);
  
  char buf[4096];
  int ret = pir_test_include_and_query_result(buf);
  //strcpy(buf, "{\"value\": \"0\"}");
  pir_test_info(buf);
  pir_test_set_prg(50);

  char mac[64];
  char status[64];    
  if (ret != 0) {
    pir_test_info("测试 失败!");
    strcpy(mac, "");
    strcpy(status, "");
    pir_test_set_mac(mac);
    pir_test_set_status(status);
    pir_test_set_img(2);
    pir_test_write_result(mac, status, ret);  
    pir_test_set_prg(99);  
    pir_test_enable_btn();
    return 0;
  }

  char buf1[128];
  pir_test_info("开启退网模式...");
  MessageBox(g_hpirwnd, "开启退网模式,请按下Pir的配网按钮", "Pir Test 信息", MB_OK);
  pir_test_set_prg(60);      
  ret = pir_test_exclude(buf1);  


  if (ret == 0) {
    pir_test_info("测试OK!");
    pir_test_parse_mac(buf, mac);
    pir_test_parse_status(buf, status);
    pir_test_set_mac(mac);
    pir_test_set_status(status);
    pir_test_set_img(1);
  } else {
    pir_test_info("测试 失败!");
    strcpy(mac, "");
    strcpy(status, "");
    pir_test_set_mac(mac);
    pir_test_set_status(status);
    pir_test_set_img(2);
  }
  pir_test_write_result(mac, status, ret);  
  pir_test_set_prg(99);  
  pir_test_enable_btn();
  return 0;
}

int pir_test_resouse_init() {
  hBmOk= LoadBitmap(g_hinst, MAKEINTRESOURCE(1));
  hBmFd = LoadBitmap(g_hinst, MAKEINTRESOURCE(2));
  hBmIl = LoadBitmap(g_hinst, MAKEINTRESOURCE(3));
  hBmIl_pir = LoadBitmap(g_hinst, MAKEINTRESOURCE(3));
  if (hBmOk == NULL || hBmFd == NULL || hBmIl == NULL) {
    MessageBox(g_hwnd, "无法加载Pir Test资源", "加载Error", MB_OK);
    return -1;
  }
  GetObject(hBmFd,sizeof(BITMAP),(LPVOID)&bmFd);
  GetObject(hBmOk,sizeof(BITMAP),(LPVOID)&bmOk);
  GetObject(hBmIl,sizeof(BITMAP),(LPVOID)&bmIl);  
  return 0;
}


INT_PTR CALLBACK PirTest(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (message) {
  case WM_INITDIALOG:
    {
      g_hpirwnd = hDlg;


      HWND hwnd_img = GetDlgItem(hDlg, PIR_TEST_PIC_RESULT);

      if (pir_memdc != NULL) {
	DeleteObject(pir_memdc);
	pir_memdc = NULL;
      }
      HDC hdc = GetDC(hwnd_img);
      pir_memdc = CreateCompatibleDC(hdc);
      ReleaseDC(hwnd_img,hdc);


      pir_fDefProc =  (WNDPROC)GetWindowLong(hwnd_img, GWL_WNDPROC);    
      SetWindowLong(hwnd_img, GWL_WNDPROC, (LONG)PirImgProc);

      pir_test_set_img(0);
    }
    return (INT_PTR)TRUE;

  case WM_COMMAND:
    if (LOWORD(wParam) == IDCANCEL) {
      EndDialog(hDlg, LOWORD(wParam));
      return (INT_PTR)TRUE;
    }
    else if (LOWORD(wParam) == PIR_TEST_BTN_START) {
      pir_target_test();
      return (INT_PTR)TRUE;
    }
    break;
  }
  return (INT_PTR)FALSE;
}



INT_PTR CALLBACK PirImgProc(HWND hwnd, UINT message,
			   WPARAM wParam, LPARAM lParam) {
   if (message != WM_PAINT) {
     return CallWindowProc(pir_fDefProc, hwnd, message, wParam, lParam);     
   }

   PAINTSTRUCT pt;
   HDC hdc = BeginPaint(hwnd, &pt);

   if (hBmDis != NULL && (hBmDis == hBmOk || hBmDis == hBmFd || hBmDis == hBmIl_pir)) {
     SelectObject(pir_memdc,hBmDis);
     RECT rc;
     GetClientRect(hwnd, &rc);

     if (hBmDis != NULL) {
       if (hBmDis == hBmOk) {
	 StretchBlt(pt.hdc,
		    0,0, rc.right-rc.left, rc.bottom - rc.top,
		    pir_memdc,
		    0,0,bmOk.bmWidth, bmOk.bmHeight,
		    SRCCOPY);
       } else if (hBmDis == hBmFd) {
	 StretchBlt(pt.hdc,
		    0,0, rc.right-rc.left, rc.bottom - rc.top,
		    pir_memdc,
		    0,0,bmFd.bmWidth, bmFd.bmHeight,
		    SRCCOPY);       
       } else if (hBmDis == hBmIl_pir) {
	 StretchBlt(pt.hdc,
		    0,0, rc.right-rc.left, rc.bottom - rc.top,
		    pir_memdc,
		    0,0,bmIl.bmWidth, bmIl.bmHeight,
		    SRCCOPY);       	 
       }
     }
   }

   EndPaint(hwnd, &pt);

   return 0;
 }




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static WNDPROC gwpic_fDefProc = NULL;
static HBITMAP hBmGwDis = NULL;
static HDC gwpic_memdc = NULL;
INT_PTR CALLBACK GwPicProc(HWND hwnd, UINT message,
			   WPARAM wParam, LPARAM lParam);

static void gw_test_pic_init(HWND hwnd) {
  HWND gwpic_hwnd = GetDlgItem(hwnd, GW_TEST_PIC);
  HDC hdc = GetDC(gwpic_hwnd);
  gwpic_memdc = CreateCompatibleDC(hdc);
  ReleaseDC(gwpic_hwnd,hdc);

  gwpic_fDefProc =  (WNDPROC)GetWindowLong(gwpic_hwnd, GWL_WNDPROC);    
  SetWindowLong(gwpic_hwnd, GWL_WNDPROC, (LONG)GwPicProc);  
}

static void gw_test_pic_set(int type) {
  if (type == 0) {
    hBmGwDis = hBmIl;
    InvalidateRect(GetDlgItem(g_hwnd, GW_TEST_PIC), NULL, TRUE);
  } else if (type == 1) {
    hBmGwDis = hBmOk;
    InvalidateRect(GetDlgItem(g_hwnd, GW_TEST_PIC), NULL, TRUE);    
  } else if (type == 2) {
    hBmGwDis = hBmFd;
    InvalidateRect(GetDlgItem(g_hwnd, GW_TEST_PIC), NULL, TRUE);    
  } else {
    hBmGwDis = 0;
  }  
}


INT_PTR CALLBACK GwPicProc(HWND hwnd, UINT message,
			   WPARAM wParam, LPARAM lParam) {
   if (message != WM_PAINT) {
     return CallWindowProc(gwpic_fDefProc, hwnd, message, wParam, lParam);     
   }

   PAINTSTRUCT pt;
   HDC hdc = BeginPaint(hwnd, &pt);

   if (hBmGwDis != NULL && (hBmGwDis == hBmOk || hBmGwDis == hBmFd || hBmGwDis == hBmIl)) {
     SelectObject(gwpic_memdc,hBmGwDis);
   
     RECT rc;
     GetClientRect(hwnd, &rc);

     if (hBmGwDis != NULL) {
       if (hBmGwDis == hBmOk) {
	 StretchBlt(pt.hdc,
		    0,0, rc.right-rc.left, rc.bottom - rc.top,
		    gwpic_memdc,
		    0,0,bmOk.bmWidth, bmOk.bmHeight,
		    SRCCOPY);
       } else if (hBmGwDis == hBmFd) {
	 StretchBlt(pt.hdc,
		    0,0, rc.right-rc.left, rc.bottom - rc.top,
		    gwpic_memdc,
		    0,0,bmFd.bmWidth, bmFd.bmHeight,
		    SRCCOPY);       
       } else if (hBmGwDis == hBmIl) {
	 StretchBlt(pt.hdc,
		    0,0, rc.right-rc.left, rc.bottom - rc.top,
		    gwpic_memdc,
		    0,0,bmIl.bmWidth, bmIl.bmHeight,
		    SRCCOPY);       	 	 
       }
     }
   }

   EndPaint(hwnd, &pt);
   return 0;
}

static void view_data(char *data, int size) {
  int i;

  for (i = 0; i < size; i++) {
    char buf[8];
    sprintf(buf, "%02X", data[i]&0xff);
    //cout << "[" << hex << buf << "] ";
    cout <<  hex  << buf   ;
  }
  cout << endl;
}

#include "basetype.h"
#include "datasource.h"
#include "errCode.h"

typedef struct _tee_id2_prov_head_t {
    uint32_t magic;   /* 0x764f7250 */
    uint32_t version; /* 0x00030000 */
    uint32_t rsvd[4]; /* 0x00000000 */
    uint32_t rec_num;
    uint8_t recs[];
} tee_id2_prov_head_t;

int  id2_sdk_pc_program() {
  #define ID2_SDK_DEBUG 0
  
  ds_cfg_t cfg;
  cfg.port_num = 1;
  strcpy((char *)cfg.lic_path, "./licenseConfig.ini");
  int ret  = ds_init(&cfg);
  if (ret != DS_STATUS_OK) {
    cout << "ds_init error" << ret << endl;
    return -1;
  }
  cout << "ds_init ok" << endl;

#if ID2_SDK_DEBUG == 0
  ds_dev_t dev = NULL;
  ret = ds_create_dev(&dev);
  if(ret != DS_STATUS_OK) {
    cout << "ds_create_dev error" << ret << endl;
    return -2;
  }
  cout << "ds_create_dev ok" << endl;


  uint32_t rec_num = 0;  
  if(ds_get_rec_num(dev, &rec_num) != DS_STATUS_OK) {
    cout << "destroy dev.." << endl;
    ds_destroy_dev(dev);    
    cout << "ds_get_rec_num" << endl;
    return -3;
  }
  cout << "ds_get_rec_num ok: " << rec_num << endl;

  uint32_t rec_idx;
  uint32_t rec_size = 0;
  char rec_data[256];

  char data[1024];
  uint32_t data_len = 0;
  
  for (rec_idx = 0; rec_idx < rec_num; rec_idx++) {
    ret = ds_get_rec(dev, rec_idx, NULL, &rec_size);
    if (ret != DS_STATUS_OK) {
      cout << "destroy dev.." << endl;      
      ds_destroy_dev(dev);      
      cout << "ds_get_rec size failed: " << ret << endl;
      return -4;
    }
    cout << "ds_get_rec(idx: " << rec_idx << "size :"  << rec_size  << endl;      

    ret = ds_get_rec(dev, rec_idx, rec_data, &rec_size);
    if (ret != DS_STATUS_OK) {
      cout << "destroy dev.." << endl;      
      ds_destroy_dev(dev);      
      cout << "ds_get_rec(idx: " << rec_idx << "size error (" << ret << ")" << endl;
      return -5;
    }
    
    cout << "ds_get_rec idx:" << rec_idx << " data ok" << endl;
    memcpy(data + sizeof(tee_id2_prov_head_t) + data_len, rec_data, rec_size);
    data_len += rec_size;
  }

  tee_id2_prov_head_t * pti = (tee_id2_prov_head_t*)data;
  memset(pti, 0x0, sizeof(tee_id2_prov_head_t));
  pti->magic = 0x764f7250;
  pti->version = 0x00020000;
  pti->rec_num = rec_num;
  data_len += sizeof(tee_id2_prov_head_t);
  
  cout << "ds_get_rec ok" << endl;
  
  cout << "ds data (" << data_len <<")  bytes:" << endl;
  view_data(data, data_len);
  #else

  #endif
  
  /*
    /usr/bin/writeid2 -i "50724F760000020000000000000000000000000000000000030000005265436D010000000000000000000000000000000000000000000100000000007F71020218000000303
    0323635313243434341433037443239313736303530305265436D0100000001000000000000000000000000000000010001001000000078F5DDC610000000D7ED0705D8A32D9EAF67F655AA93258B5265436D0100000001
    0000000000000000000000000000000200010000000000D049690320000000164B2D4FEB02C365EF7101682DCDEDEE965AF57E2C3624199C9B3ED13C566573"
  */
  int size = 0;
  char buf[4096];
  int len = 0;
#if ID2_SDK_DEBUG == 0
  len += sprintf(buf+len,
		 "if [ -e \"/usr/bin/writeid2\" ]; then\n"
		 "\t/usr/bin/writeid2 -i ");
  int i = 0;
  id2_data_str_len = 0;
  for (i = 0; i < data_len; i++) {
    id2_data_str_len += sprintf(id2_data_str + id2_data_str_len, "%02X", data[i]&0xff);
    len += sprintf(buf+len, "%02X", data[i]&0xff);
  }
  len += sprintf(buf + len , " > /dev/null \n");
  len += sprintf(buf + len,
		 "\tret=$?\n"
		 "\techo $ret\n"
		 "\treturn $ret\n"
		 "else\n"
		 "\techo 1\n"
		 "\treturn 1;\nfi");
#else
  strcpy(buf,
    "if [ -e \"/usr/bin/writeid2\" ]; then\n"
    "\t/usr/bin/writeid2 -i 50724F760000020000000000000000000000000000000000030000005265436D010000000000000000000000000000000000000000000100000000007F710202180000003030323635313243434341433037443239313736303530305265436D0100000001000000000000000000000000000000010001001000000078F5DDC610000000D7ED0705D8A32D9EAF67F655AA93258B5265436D01000000010000000000000000000000000000000200010000000000D049690320000000164B2D4FEB02C365EF7101682DCDEDEE965AF57E2C3624199C9B3ED13C566573 > /dev/null ;\n"
    "\tret=$?\n"
    "\techo $ret\n"
    "\treturn $ret\n"
    "else\n"
    "\techo 1\n"
    "\treturn 1;\nfi");
  strcpy(id2_data_str, "50724F760000020000000000000000000000000000000000030000005265436D010000000000000000000000000000000000000000000100000000007F710202180000003030323635313243434341433037443239313736303530305265436D0100000001000000000000000000000000000000010001001000000078F5DDC610000000D7ED0705D8A32D9EAF67F655AA93258B5265436D01000000010000000000000000000000000000000200010000000000D049690320000000164B2D4FEB02C365EF7101682DCDEDEE965AF57E2C3624199C9B3ED13C566573");
  id2_data_str_len = strlen(id2_data_str);
#endif
  cout << "cmd : " << buf << endl;  



  char out[1024*10];
  memset(out, 0, sizeof(out));
  out[0] = '2';
  int outsize = sizeof(out);

  ret = libssh2_main(g_gwip, 22, "root", "root", buf, out, &outsize);
  
  //strcpy(g_gwip, "192.168.66.134");  
  //ret = libssh2_main(g_gwip, 22, "root", "123456", buf, out, &outsize);
  
  cout << "[" << out  << "]" << endl;
  if (ret == 0 && out[0] == '0') {
    cout << "id data program ok" << endl;
  } else {
    cout << "destroy dev.." << endl;    
    //ds_destroy_dev(dev);  
   cout << "id data program failed : " << out[0] << endl;
   return -6;
  }
  
#if ID2_SDK_DEBUG == 0    
  ds_dev_prov_stat_t prov_stat = DEV_PROV_STAT_SUCCESS;  
  if (ds_set_dev_prov_stat(dev, prov_stat) != DS_STATUS_OK) {
    cout << "ds_set_dev_prov_stat(" << prov_stat << ") fail" << endl;
    cout << "destroy dev.." << endl;  
    ds_destroy_dev(dev);
    return -7;
  }
  cout << "ds_set_dev_prov_stat ok" << endl;

  cout << "destroy dev.." << endl;  
  ds_destroy_dev(dev);  
#endif

  ds_cleanup();
  return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//单载波测试

static HDC gwpic_memdc_dt = NULL;
INT_PTR CALLBACK GwPicProc_dt(HWND hwnd, UINT message,
			   WPARAM wParam, LPARAM lParam);
static WNDPROC gwpic_fDefProc_dt = NULL;

static HBITMAP hBmGwDis_dt = NULL;
static HWND hwndST = NULL;

static void gw_test_pic_init_dt(HWND hwnd) {
  if (gwpic_memdc_dt  != NULL) {
    DeleteObject(gwpic_memdc_dt);
    gwpic_memdc_dt = NULL;
  }
  
  HWND gwpic_hwnd = GetDlgItem(hwnd, IDC_IMG1);
  HDC hdc = GetDC(gwpic_hwnd);
  gwpic_memdc_dt = CreateCompatibleDC(hdc);
  ReleaseDC(gwpic_hwnd,hdc);

  gwpic_fDefProc_dt =  (WNDPROC)GetWindowLong(gwpic_hwnd, GWL_WNDPROC);    
  SetWindowLong(gwpic_hwnd, GWL_WNDPROC, (LONG)GwPicProc_dt);  
}

static void gw_test_pic_set_dt(int type) {
  if (type == 0) {
    hBmGwDis_dt = hBmIl;
    InvalidateRect(GetDlgItem(hwndST, IDC_IMG1), NULL, TRUE);
  } else if (type == 1) {
    hBmGwDis_dt = hBmOk;
    InvalidateRect(GetDlgItem(hwndST, IDC_IMG1), NULL, TRUE);    
  } else if (type == 2) {
    hBmGwDis_dt = hBmFd;
    InvalidateRect(GetDlgItem(hwndST, IDC_IMG1), NULL, TRUE);    
  } else {
    hBmGwDis = 0;
  }  
}

INT_PTR CALLBACK GwPicProc_dt(HWND hwnd, UINT message,
			   WPARAM wParam, LPARAM lParam) {
   if (message != WM_PAINT) {
     return CallWindowProc(gwpic_fDefProc_dt, hwnd, message, wParam, lParam);     
   }

   PAINTSTRUCT pt;
   HDC hdc = BeginPaint(hwnd, &pt);
   

   if (hBmGwDis_dt != NULL && (hBmGwDis_dt == hBmOk || hBmGwDis_dt == hBmFd || hBmGwDis_dt == hBmIl)) {
     SelectObject(gwpic_memdc_dt,hBmGwDis_dt);
   
     RECT rc;
     //GetClientRect(hwnd, &rc);
     GetClientRect(GetDlgItem(hwndST, IDC_IMG1), &rc);     

     if (hBmGwDis_dt != NULL) {
       if (hBmGwDis_dt == hBmOk) {
	 StretchBlt(pt.hdc,
		    0,0, rc.right-rc.left, rc.bottom - rc.top,
		    gwpic_memdc_dt,
		    0,0,bmOk.bmWidth, bmOk.bmHeight,
		    SRCCOPY);
       } else if (hBmGwDis_dt == hBmFd) {
	 StretchBlt(pt.hdc,
		    0,0, rc.right-rc.left, rc.bottom - rc.top,
		    gwpic_memdc_dt,
		    0,0,bmFd.bmWidth, bmFd.bmHeight,
		    SRCCOPY);       
       } else if (hBmGwDis_dt == hBmIl) {
	 StretchBlt(pt.hdc,
		    0,0, rc.right-rc.left, rc.bottom - rc.top,
		    gwpic_memdc_dt,
		    0, 0, bmIl.bmWidth, bmIl.bmHeight,
		    SRCCOPY);
       }
     }
   }

   EndPaint(hwnd, &pt);
   return 0;
}

#if 0
using namespace BarTender;
using namespace System;
void print_mac_cpp() {
  BarTender::Application ^btApp=gcnew BarTender::Application;
  BarTender::Format ^btFormat=gcnew BarTender::Format;
  // btFormat = btApp->Formats->Open("D:\\1100.btw", false, "");//标签模板
  System::String^ str= gcnew String(m_DocFile);//此处DocFile为CString类型，用于存储标签模板路径
  btFormat = btApp->Formats->Open(str, false, "");
  btFormat->PrintSetup->IdenticalCopiesOfLabel=1;//份数
  btFormat->PrintSetup->NumberSerializedLabels = 1;//序列标签数
  GetDlgItem(IDC_EDIT1)->GetWindowTextA(cs_code);//此处为界面编辑框中的信息
  System::String^ str1= gcnew String(cs_code);//将想要打印的标签信息送给CSN
  btFormat->SetNamedSubStringValue("CSN", str1);//CSN为在bartender软件中创建的具名数据源
  btFormat->PrintOut(false, false);
  btFormat->Close(BarTender::BtSaveOptions::btDoNotSaveChanges); //退出时是否保存标签
  btApp->Quit(BarTender::BtSaveOptions::btDoNotSaveChanges);// 界面退出时同步退出bartender进程  
}
#endif

static int print_init = 0;  
void print_mac_init() {
  return;
  if (print_init == 0) {
    system("cd C:Program Files (x86)\\Seagull\\BarTender Suite");
    
    //system("start bartend.exe /F=f:\\work\\project\\project\\LTE4G\\src\\gwtest\\TESTZ3.btw");
    char cmd[1024];    
    sprintf(cmd, "start bartend.exe /F=%s\\TESTZ3.btw", rootDir);
    cout << cmd << endl;
    system(cmd);

    print_init = 1;    
  }
}

void print_mac(string mac) {
  cout << "MAC:[" << mac.c_str() << "]" << endl;
  return;
  /**> set print conetent */
  char file[1024];
  sprintf(file, "%s\\PrintData.txt", rootDir);
  //FILE *fp = fopen(".\\PrintData.txt", "w");
  FILE *fp = fopen(file, "w");  
  if (fp == NULL) {
    cout << "Can't  Open File: " << file << endl;
    return ;
  }
  fwrite("MAC,MACTYPE\r\n", strlen("MAC,MACTYPE\r\n"), 1, fp);
  char ctx_buf[128];
					    sprintf(ctx_buf, "%s,%s|1203", mac.c_str(), mac.c_str());
  fwrite(ctx_buf, strlen(ctx_buf), 1, fp);
  fclose(fp);
  cout << ctx_buf << endl;

  char cmd[1024];  
  //system("start bartend.exe /D=F:\\work\\project\\project\\LTE4G\\src\\gwtest\\PrintData.txt");
  sprintf(cmd, "start bartend.exe /D=%s\\PrintData.txt", rootDir);
  system(cmd);
  cout << cmd << endl;
  
  system("start bartend.exe /P");    
}

void print_mac_free() {
  return;
  system("start bartend.exe /x");//close bartender
  print_init = 0;  
}

static void test_cacl_year_day_str(char *ydstr, int len) {

  time_t timer;
  struct tm *tblock;
  timer = time(NULL);
  tblock = localtime(&timer);

  char buff[128];
  memset(ydstr, 0, len);
  sprintf(buff, "%d年%d月%d号-By_%s.xls", tblock->tm_year+1900, tblock->tm_mon+1, tblock->tm_mday, "Keven");
  
  strcpy(ydstr, buff);
  //MultiByteToWideChar(CP_ACP, 0, buff, strlen(buff), ydstr, len);
}
void cacl_test_write_result(char *mac, double power, double frequence, double delt) {
  static char *tos[] = {
    "mac", "功率(dBm)", "频率(KHZ)", "频偏(KHZ)", "Ref频率(KHZ)"
  };
  int i = 0;
  int cnt = sizeof(tos) / sizeof(tos[0]);
  const char* cols[256];
  memset(cols, 0, sizeof(char *) * 256);
  for (i = 0; i < cnt; i++) {
    cols[i] = new char[512];
    memset((void *)cols[i], 0, sizeof(char) * 128);
    strcpy((char *)cols[i], tos[i]);
    //MultiByteToWideChar(CP_ACP, 0, g_tos[i].name, strlen(g_tos[i].name), cols[i], 128);
    //wsprintf(cols[i], L"%s", g_tos[i].name);
  }
  char name[128];
  test_cacl_year_day_str(name, 128);
  const char *path = rootDir;  
  BookHandle bh = sheet_open(path, name, "FrequenceTest", 0, cols, cnt);

  if (bh == NULL) {
    for (i = 0; i < cnt; i++) {
      if (cols[i] != NULL) {
	delete cols[i];
      }
    }
    MessageBox(NULL, "Error", "open a.xls failed!", MB_OK);
    return;
  }


  char svals[5][64];
  sprintf(svals[0], "%s", mac);
  sprintf(svals[1], "%f dBm", power);
  sprintf(svals[2], "%f KHZ", frequence/1000.0f);
  sprintf(svals[3], "%f KHZ", delt);    
  sprintf(svals[4], "%f KHZ", 2.425 * 1000.0f * 1000.0f/1000.0f);

  for (i = 0; i < cnt; i++) {
    memset((void *)cols[i], 0, sizeof(char) * 512);
    strcpy((char *)cols[i], svals[i]);
    //MultiByteToWideChar(CP_ACP, 0, g_tos[i].strval, strlen(g_tos[i].strval), cols[i], 128);
  }
  sheet_append(bh, 0, (const char **)cols, cnt);
  sheet_save(bh, path, name);
  sheet_close(bh);
  for (i = 0; i < cnt; i++) {
    if (cols[i] != NULL) {
      delete cols[i];
    }
  }
}


void StTest_do() {
#if 0
  if (target_connect() == 0) {
    char val[128];
    int ret = cmd_request_st_amber(val);
    if (ret == 0) {
      cout << "OK" << endl;
      gw_test_pic_set_dt(1);
    } else {
      cout << "Failed" << endl;
      gw_test_pic_set_dt(2);      
    }
    target_disconnect();
  } else {
      gw_test_pic_set_dt(2);          
      //MessageBox(NULL, "Can't connect to server!", "Error", MB_OK);
  }
#else
  char outinfo[256];

  cout << "============================" << endl;
  test_info_clear();
  
  if (amber_test_interface == 0 && target_connect() != 0) {
    cout << "connect error!" << endl;
    gw_test_pic_set_dt(2);
    target_disconnect();
    test_info_append("连接测试网关失败!!");
    return;
  }

  double power = 0.0f;
  double frequence = 0.0f;
  double delt_freqence = 0.0f;
  string mac = "";
  int nxp_checked     = IsDlgButtonChecked(hwndST, ID_CHK_NXP);
  int ember_checked = IsDlgButtonChecked(hwndST, ID_CHK_EMBER);
  int ble_checked = IsDlgButtonChecked(hwndST, ID_CHK_BLE);  
  cout << "nxp_check:" << nxp_checked << ",ember_checked:" << ember_checked << ",ble_checked:" << ble_checked << endl;
  if (ember_checked) {
    if (!visa_amber_rf_power_test(power, frequence, delt_freqence, mac)) {
      cout << "rf test failed--" << endl;
      gw_test_pic_set_dt(2);
      if (amber_test_interface == 0) {
	target_disconnect();
      }
      test_info_append("RF 频率矫正及测试失败!");

      if (mac.compare("") != 0)  {
	sprintf(outinfo, "打印mac...");
	test_info_append(outinfo);
	print_mac(mac);
      }
      cout  << "-----" << endl;
      return;
    }
  
    sprintf(outinfo, "mac:%s, 功率:%f dBm, 频率:%f KHZ, 频偏:%f KHZ", mac.c_str(), power, frequence/1000.0f, delt_freqence);
    test_info_append(outinfo);
    cout << "OK" << endl;
    gw_test_pic_set_dt(1);
    if (amber_test_interface == 0) {
      target_disconnect();
    }

    sprintf(outinfo, "写入测试结果到xls..");
    test_info_append(outinfo);    
    cacl_test_write_result((char *)mac.c_str(), power, frequence, delt_freqence);    

    sprintf(outinfo, "打印mac...");
    test_info_append(outinfo);  
    print_mac(mac);
  }
  
  if (nxp_checked) {
    if (!visa_nxp_rf_power_test(power, frequence, delt_freqence, mac)) {
      cout << "rf test failed" << endl;
      gw_test_pic_set_dt(2);
      if (amber_test_interface == 0) {
	target_disconnect();
      }
      test_info_append("RF 频率测试失败!");

      if (mac.compare("") != 0)  {
	sprintf(outinfo, "打印mac...");
	test_info_append(outinfo);
	print_mac(mac);
      }
      return;
    }
  
    sprintf(outinfo, "mac:%s, 功率:%f dBm, 频率:%f KHZ, 频偏:%f KHZ", mac.c_str(), power, frequence/1000.0f, delt_freqence);
    test_info_append(outinfo);
    cout << "OK" << endl;
    gw_test_pic_set_dt(1);
    if (amber_test_interface == 0) {
      target_disconnect();
    }

    sprintf(outinfo, "写入测试结果到xls..");
    test_info_append(outinfo);    
    cacl_test_write_result((char *)mac.c_str(), power, frequence, delt_freqence);    

    sprintf(outinfo, "打印mac...");
    test_info_append(outinfo);  
    print_mac(mac);    
  }

  if (ble_checked) {
    if (!visa_ble_rf_power_test(power, frequence, delt_freqence, mac)) {
      cout << "rf test failed" << endl;
      gw_test_pic_set_dt(2);
      if (amber_test_interface == 0) {
	target_disconnect();
      }
      test_info_append("RF 频率测试失败!");

      if (mac.compare("") != 0)  {
	sprintf(outinfo, "打印mac...");
	test_info_append(outinfo);
	print_mac(mac);
      }
      return;
    }
  
    sprintf(outinfo, "mac:%s, 功率:%f dBm, 频率:%f KHZ, 频偏:%f KHZ", mac.c_str(), power, frequence/1000.0f, delt_freqence);
    test_info_append(outinfo);
    cout << "OK" << endl;
    gw_test_pic_set_dt(1);
    if (amber_test_interface == 0) {
      target_disconnect();
    }

    sprintf(outinfo, "写入测试结果到xls..");
    test_info_append(outinfo);    
    cacl_test_write_result((char *)mac.c_str(), power, frequence, delt_freqence);    

    sprintf(outinfo, "打印mac...");
    test_info_append(outinfo);  
    print_mac(mac);    
  }  
#endif
}




DWORD WINAPI st_test_thread(LPVOID lpParam) {
  EnableWindow(GetDlgItem(hwndST, BTN_ST_TEST), FALSE);
  StTest_do();

  Sleep (5000);
  EnableWindow(GetDlgItem(hwndST, BTN_ST_TEST), TRUE);        
  return 0;
}






int load_rf_test_config_from_file(const char *file) {
  FILE *fp = fopen(file, "r");
  if (fp == NULL) {
    return -1;
  }
  char buf[1024];
  char *p = fgets(buf, sizeof(buf), fp);
  if (p == NULL) {
    fclose(fp);
    return -2;
  }

  cout << buf << endl;

  int apw = 0;
  if (sscanf(buf, "%d,%d,%d,%d,%d,%d,%d", &test_amber,&test_nxp, &shoujian, &apw,
	     &amber_test_interface, &amber_test_com, &test_ble) != 6) {
    fclose(fp);
    return -3;
  }
  fclose(fp);
  amber_power_limit = (float)apw;
  
  return 0;
}

int save_rf_test_config_from_file(const char *file) {
  FILE *fp = fopen(file, "w");
  if (fp == NULL) {
    return -1;
  }

  char buf[1024];
  int apw = (int)amber_power_limit;
  sprintf(buf, "%d,%d,%d,%d,%d,%d,%d", test_amber, test_nxp, shoujian, apw, amber_test_interface, amber_test_com, test_ble);
  cout << buf << endl;
  fwrite( buf, strlen(buf), 1, fp);
  fclose(fp);
  
  return 0;
}
INT_PTR CALLBACK STTest(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (message) {
  case WM_INITDIALOG:
    {
      hwndST = hDlg;
      gw_test_pic_init_dt(hwndST);      
      gw_test_pic_set_dt(0);

      print_mac_init();

      int i = 0;
      for (i = -15; i < 20; i++) {
	char buf[20];
	sprintf(buf, "%d", i);
	SendMessage(GetDlgItem(hDlg, CBX_XS),CB_INSERTSTRING,0,(LPARAM)buf);
      }

      SendMessage(GetDlgItem(hDlg, CBX_ZBFF),CB_INSERTSTRING,0,(LPARAM)"串口");	            
      SendMessage(GetDlgItem(hDlg, CBX_ZBFF),CB_INSERTSTRING,0,(LPARAM)"网络");            
      
      char file[1024];
      sprintf(file, "%s/amber_power_limit", rootDir);
      load_rf_test_config_from_file(file);

      /**> init */
      #if 1
      if (test_nxp) {	
	CheckDlgButton(hDlg, ID_CHK_NXP, TRUE);
      } else {
	CheckDlgButton(hDlg, ID_CHK_NXP, FALSE);
      }
      
      if (test_amber) {
	CheckDlgButton(hDlg, ID_CHK_EMBER, TRUE);
      } else {
	CheckDlgButton(hDlg, ID_CHK_EMBER, FALSE);	
      }

      if (test_ble) {
	CheckDlgButton(hDlg, ID_CHK_BLE, TRUE);
      } else {
	CheckDlgButton(hDlg, ID_CHK_BLE, FALSE);	
      }      
      
      if (shoujian) {
	CheckDlgButton(hDlg,ID_CHOUJIAN_AMER, TRUE);
      } else {
	CheckDlgButton(hDlg,ID_CHOUJIAN_AMER, FALSE);	
      }

      int apl = 19 - (int)amber_power_limit;
      SendMessage(GetDlgItem(hDlg, CBX_XS), CB_SETCURSEL, apl, 0);
      
      SendMessage(GetDlgItem(hDlg, CBX_ZBFF), CB_SETCURSEL,amber_test_interface, 0);

      SetDlgItemInt(hDlg, IDC_ZBDEV, amber_test_com, FALSE);

      if (amber_test_interface == 0) {// net
	EnableWindow(GetDlgItem(hDlg, IDC_ZBDEV), FALSE);
      } else {
	EnableWindow(GetDlgItem(hDlg, IDC_ZBDEV), TRUE);	  
      }      
      #endif
    }
    return (INT_PTR)TRUE;

  case WM_COMMAND:
    if (LOWORD(wParam) == IDCANCEL) {
      EndDialog(hDlg, LOWORD(wParam));
      print_mac_free();
      return (INT_PTR)TRUE;
    }
    else if (LOWORD(wParam) == BTN_ST_TEST) {
      gw_test_pic_set_dt(0);
      //amber_power_limit = 5.0f;
      //amber_test_interface = 0; // network

      int cursel = SendMessage(GetDlgItem(hDlg, CBX_XS), CB_GETCURSEL, 0, 0);
      amber_power_limit = 19.0f - cursel;
      cout << "amber_power_limit is " << amber_power_limit << endl;
      char buf[256];
      sprintf(buf, "发射功率阀值 : %f", amber_power_limit);
      test_info_append(buf);

      cursel = SendMessage(GetDlgItem(hDlg, CBX_ZBFF), CB_GETCURSEL, 0, 0);
      amber_test_interface = cursel%2;
      cout << "amber_test_interface is " << amber_test_interface << endl;
      sprintf(buf, "Amber 测试接口 : %d", (amber_test_interface == 0) ? "Eth" : "Uart");
      test_info_append(buf);
      
      //MessageBox(NULL, 0, buf, 0);
      CreateThread(NULL, 0, st_test_thread, NULL, 0, NULL);      
      return (INT_PTR)TRUE;
    } else if (LOWORD(wParam) == CBX_ZBFF || LOWORD(wParam) == CBX_XS) {
      if (HIWORD(wParam) == CBN_SELCHANGE) {
	test_amber = IsDlgButtonChecked(hDlg, ID_CHK_EMBER);
	test_nxp = IsDlgButtonChecked(hDlg, ID_CHK_NXP);
	test_ble = IsDlgButtonChecked(hDlg, ID_CHK_BLE);	
	shoujian = IsDlgButtonChecked(hDlg, ID_CHOUJIAN_AMER);
	int cursel = SendMessage(GetDlgItem(hDlg, CBX_XS), CB_GETCURSEL, 0, 0);
	amber_power_limit = 19.0f - cursel;
	cursel = SendMessage(GetDlgItem(hDlg, CBX_ZBFF), CB_GETCURSEL, 0, 0);
	amber_test_interface = cursel%2;
	char file[1024];
	sprintf(file, "%s/amber_power_limit", rootDir);
	save_rf_test_config_from_file(file);
	
	char buf[128];
	sprintf(buf, "amber:%d, nxp:%d, ble:%d, choujian:%d, fs:%d, com:%d",
		test_amber,test_nxp,shoujian, amber_test_interface, amber_test_com);
	test_info_append(buf);	
	if (amber_test_interface == 0) {// net
	  EnableWindow(GetDlgItem(hDlg, IDC_ZBDEV), FALSE);
	} else {
	  EnableWindow(GetDlgItem(hDlg, IDC_ZBDEV), TRUE);	  
	}
      }
    } else if (LOWORD(wParam) == IDC_ZBDEV) {
	test_amber = IsDlgButtonChecked(hDlg, ID_CHK_EMBER);
	test_nxp = IsDlgButtonChecked(hDlg, ID_CHK_NXP);
	shoujian = IsDlgButtonChecked(hDlg, ID_CHOUJIAN_AMER);
	int cursel = SendMessage(GetDlgItem(hDlg, CBX_XS), CB_GETCURSEL, 0, 0);
	amber_power_limit = 19.0f - cursel;
	cursel = SendMessage(GetDlgItem(hDlg, CBX_ZBFF), CB_GETCURSEL, 0, 0);
	amber_test_interface = cursel%2;
	amber_test_com = GetDlgItemInt(hDlg, IDC_ZBDEV, 0, 0);
	char file[1024];
	sprintf(file, "%s/amber_power_limit", rootDir);
	save_rf_test_config_from_file(file);      
    } else if (LOWORD(wParam) == ID_CHK_EMBER || LOWORD(wParam) == ID_CHK_NXP || LOWORD(wParam) == ID_CHOUJIAN_AMER || LOWORD(wParam) == ID_CHK_BLE ) {
      if (HIWORD(wParam) == BN_CLICKED) {
	test_amber = IsDlgButtonChecked(hDlg, ID_CHK_EMBER);
	test_nxp = IsDlgButtonChecked(hDlg, ID_CHK_NXP);
	test_ble = IsDlgButtonChecked(hDlg, ID_CHK_BLE);	
	shoujian = IsDlgButtonChecked(hDlg, ID_CHOUJIAN_AMER);
	int cursel = SendMessage(GetDlgItem(hDlg, CBX_XS), CB_GETCURSEL, 0, 0);
	amber_power_limit = 19.0f - cursel;
	cursel = SendMessage(GetDlgItem(hDlg, CBX_ZBFF), CB_GETCURSEL, 0, 0);
	amber_test_interface = cursel%2;
	char file[1024];
	sprintf(file, "%s/amber_power_limit", rootDir);
	save_rf_test_config_from_file(file);      	
      }
    }
    break;
  }
  return (INT_PTR)FALSE;
}

#if 1
#include "visa.h"
#include "visatype.h"

int visa_find() {
  ViSession rm = VI_NULL, vi = VI_NULL;
  ViStatus status;
  ViChar desc[256], id[256], buffer[256];
  ViUInt32 retCnt, itemCnt;
  ViFindList list;
  ViUInt32 i;
  
  // Open a default session
  status = viOpenDefaultRM(&rm);
  if (status < VI_SUCCESS) {
    cout << "viOpenDefaultRM" << endl;
    goto error;
  }
  
  // Find all GPIB devices
  status = viFindRsrc(rm, "USB?*INSTR", &list, &itemCnt,desc);
  if (status < VI_SUCCESS) {
    cout << "viFindRsrc" << endl;
    goto error;
  }
  
  for (i = 0; i < itemCnt; i++) {
    // Open resource found in rsrc list
    status = viOpen(rm, desc, VI_NULL, VI_NULL, &vi);
    if (status < VI_SUCCESS) {
      cout << "viOpen" << endl;
      goto error;
    }
    
    // Send an ID query.
    status = viWrite(vi, (ViBuf) "*idn?", 5, &retCnt);
    if (status < VI_SUCCESS) {
      cout << "viWrite" << endl;
      goto error;
    }
    
    // Clear the buffer and read the response
    memset(id, 0, sizeof(id));
    status = viRead(vi, (ViBuf) id, sizeof(id), &retCnt);
    if (status < VI_SUCCESS) {
      cout << "viRead" << endl;
      goto error;
    }
    
    // Print the response
    printf("id: %s: %s\n", desc, id);
    // We’re done with this device so close it
    viClose(vi);
    // Get the next item
    viFindNext(list, desc);
  }
  
  // Clean up
  viClose(rm);
  return 0;
 error:
  // Report error and clean up
  viStatusDesc(vi, status, buffer);
  fprintf(stderr, "failure: %s\n", buffer);
  if (rm != VI_NULL) {
    viClose(rm);
  }
  return 1; 
}
int visa_test() {
  ViSession rm = VI_NULL;
  // Open a default session
  ViStatus status = viOpenDefaultRM(&rm);
  if (status < VI_SUCCESS) {
    cout << "viOpenDefaultRM" << endl;
    goto error;
  }

  ViChar desc[256];
  ViFindList list;
  ViUInt32 itemCnt;  
  // Find all USB devices
  status = viFindRsrc(rm, "USB?*INSTR", &list, &itemCnt,desc);
  if (status < VI_SUCCESS) {
    cout << "viFindRsrc" << endl;
    goto error;
  }

  if (itemCnt <= 0) {
    cout << "no use device !" << endl;
    goto error;
  }
  
  ViSession vi = VI_NULL;
  // Open the GPIB device at primary address 1, GPIB board 8
  //status = viOpen(rm, "GPIB8::1::INSTR", VI_NULL, VI_NULL, &vi);
  status = viOpen(rm, desc, VI_NULL, VI_NULL, &vi);  
  if (status < VI_SUCCESS) {
    cout << "viOpen" << endl;
    goto error;
  }

  ViUInt32 retCnt;  
  // Send an ID query.
  status = viWrite(vi, (ViBuf) "*idn?", 5, &retCnt);
  if (status < VI_SUCCESS) {
    cout << "viWrite" << endl;
    goto error;
  }

  ViChar id[256];
  // Clear the buffer and read the response
  memset(id, 0, sizeof(id));
  status = viRead(vi, (ViBuf) id, sizeof(id), &retCnt);
  if (status < VI_SUCCESS) {
    cout << "viRead" << endl;
    goto error;
  }  
  
  // Print the response
  printf("id: %s\n", id);
  
  // Clean up
  viClose(vi); // Not needed, but makes things a bit more
  
  // understandable
  viClose(rm); // Closes resource manager and any sessions
  
  // opened with it
  return 0;
  
 error:
  // Report error and clean up
  viStatusDesc(vi, status, id);
  fprintf(stderr, "failure: %s\n", id);
  if (rm != VI_NULL) {
    viClose(rm);
  }
  return 1; 
}

#include "PCOMM.H"
int com_uart = -1;
int com_uart_open = 0;
DWORD WINAPI uart_close_thread(LPVOID lpParam) {
  //Sleep(5000);
  system("ping 111.111.111.111 -n 1 -w 4000");
  
  // int ret = sio_open (com_uart);
  //sio_ioctl (com_uart, B115200, P_NONE | BIT_8 | STOP_1 );

  char *cmd = "plugin mfg tone stop\x0d\x0a";
  int ret = sio_write (com_uart, cmd, strlen(cmd));
  system("ping 111.111.111.111 -n 1 -w 50");        
  
  cmd = "plugin mfg stop\x0d\x0a";
  ret = sio_write (com_uart, cmd, strlen(cmd));
  system("ping 111.111.111.111 -n 1 -w 50");        

  ret = sio_close(com_uart);
  com_uart_open = 0;
  return 0;
}
int uart_start_tone_for_st_amber() {
  /**> TODO */
  char buf[4096];
  GetDlgItemText(hwndST, IDC_ZBDEV, buf, sizeof(buf));
  int ret = sscanf(buf, "%d",  &com_uart);
  if (ret != 1) {
    sprintf(buf, "请设置正确的串口号, Error:[%s]", buf);
    test_info_append(buf);    
    return false;
  }

  while (com_uart_open != 0) {
    system("ping 111.111.111.111 -n 1 -w 1000");      
  }
  
  ret = sio_open (com_uart);
  if (ret != SIO_OK) {
    sprintf(buf, "无法打开串口:COM%d", com_uart);
    test_info_append(buf);    
    return false;    
  }
  com_uart_open = 1;
  sio_ioctl (com_uart, B115200, P_NONE | BIT_8 | STOP_1 );


  char *cmd = "plugin mfg start 1\x0d\x0a";
  cout << cmd << endl;
  ret = sio_write (com_uart, cmd, strlen(cmd));
  system("ping 111.111.111.111 -n 1 -w 100");  

  cmd = "plugin mfg set-channel 15\x0d\x0a";
  cout << cmd << endl;  
  ret = sio_write (com_uart, cmd, strlen(cmd));
  system("ping 111.111.111.111 -n 1 -w 100");    

  cmd = "plugin mfg set-power 20 0\x0d\x0a";
  cout << cmd << endl;  
  ret = sio_write (com_uart, cmd, strlen(cmd));
  system("ping 111.111.111.111 -n 1 -w 100");      

  cmd = "plugin mfg tone start\x0d\x0a";
  cout << cmd << endl;  
  ret = sio_write (com_uart, cmd, strlen(cmd));
  system("ping 111.111.111.111 -n 1 -w 300");        

  CreateThread(NULL, 0, uart_close_thread, NULL, 0, NULL);  
  //5. plugin mfg tone stop
  //6. plugin mfg stop  
  
  //ret = sio_close(com_uart);  
  return 0;
}
bool visa_amber_tone_start(int channel, int power) {
  if (amber_test_interface == 0) {
    char val[128];
    int ret = cmd_request_st_amber(val);
    if (ret != 0) {
      return false;
    }      
  } else {
    int ret = uart_start_tone_for_st_amber();
    if (ret != 0) {
      return false;
    }          
  }

  return true;
}

bool visa_amber_commander_get_id(string &mac) {
 char cmd[1024];
 sprintf(cmd, "cmd.exe /c .\\commander\\commander.exe device info --device efr32mg1 > .\\info_out.txt");
 cout << cmd << endl;
 WinExec(cmd, SW_HIDE);     

 FILE *fp = fopen(".\\info_out.txt", "r");
 if (fp == NULL) {
   cout << "can't open info_out.txt" << endl;
   return false;
 }

 do {
   char line[256];
   char *p = fgets(line, sizeof(line), fp);
   if (p == NULL) {
     cout << "End of File" <<endl;
     fclose(fp);
     return false;
   }

   cout << line << endl;
   if (strstr(line, "Unique ID") == NULL) {
     continue;
   }

   p = strstr(line, ":");
   if (p == NULL) {
     continue;
   }

   p +=  1;
   p[16] = 0;
   cout <<"mac is " << p << endl;
   mac = p;
   fclose(fp);   
   return true;
 } while (1);

 fclose(fp);
 return false;
}

bool visa_amber_commander_set_tune(unsigned short tune) {
  int cnt = 3;
  do {
    char cmd[1024];
    sprintf(cmd, "cmd.exe /c .\\commander\\commander.exe flash --tokengroup znet --token TOKEN_MFG_CTUNE:0x%x --device efr32mg1 > .\\tune_out.txt", tune);
    cout << cmd << endl;

    //WinExec("cmd.exe /c dir > d:\\abc.txt", SW_HIDE);
    WinExec(cmd, SW_HIDE);    
    //system(cmd);

    FILE *fp = fopen(".\\tune_out.txt", "r");
    if (fp == NULL) {
      test_info_append("打开tune_out.txt failed");
      cout << "can't open file tune_out.txt" <<endl;
      return false;
    }
    
    int flag = 0;
    while (1) {
      char line[1024];
      if (fgets(line, sizeof(line), fp) == NULL) {
	break;
      }
      cout << line << endl;
      if ((strstr(line, "WARNING:") != NULL || strstr(line, "ERROR:") != NULL))  {
	cout << "WARNING + ERROR find in outtext" <<endl;
	test_info_append("WARNING OR ERROR");
	flag = 1;
	break;
      }
    }
    
    fclose(fp);
    fp = NULL;

    cnt--;    

    if (flag == 1) {
      if (cnt > 0) {
	cout << "continue to set tune" << endl;
	continue;
      } else {
	cout << "set tone false" << endl;
	test_info_append("尝试SetTone失败!");
	return false;
      }
    } else {
      cout << "set tone ok" << endl;
      break;
    }
  } while (cnt > 0);

  return true;
}


bool visa_amber_commander_get_tune(unsigned short &tune) {
 //cmd.exe /c .\\commander\\commander.exe tokendump --tokengroup znet  --token TOKEN_MFG_CTUNE --device efr32mg1
  
  char cmd[1024];
  sprintf(cmd, "cmd.exe /c .\\commander\\commander.exe tokendump --tokengroup znet  --token TOKEN_MFG_CTUNE --device efr32mg1 > .\\get_tune_out.txt");
  cout << cmd << endl;

  //WinExec("cmd.exe /c dir > d:\\abc.txt", SW_HIDE);
  WinExec(cmd, SW_HIDE);
  //system(cmd);
  Sleep(2000);

  FILE *fp = fopen(".\\get_tune_out.txt", "r");
  if (fp == NULL) {
    test_info_append("打开get_tune_out.txt 失败!");
    cout << "can't open file get_tune_out.txt" <<endl;
    return false;
  }

  while (1) {
    char line[1024];
    if (fgets(line, sizeof(line), fp) == NULL) {
      fclose(fp);
      fp = NULL;
      test_info_append("fgets 失败!");
      return false;
    }
    cout << "[" << line  << "]" << endl;
    if (strstr(line, "MFG_CTUNE:") == NULL) {
      cout  << "no MFG_CTUNE:" << endl;
      continue;
    }
    unsigned short x = 0;
    if (sscanf(line, "MFG_CTUNE: 0x%x", &x) != 1) {
      cout << "parse tune failed" << endl;
      fclose(fp);
      fp = NULL;
      test_info_append("获取MFG_CTUNE解析失败!");
      return false;
    }
    if (x == 0xFFFF) {
      x = 0x150;
    }
    if (x >= 1 && x <= 0x200) {
      fclose(fp);
      fp = NULL;
      tune = x;
      return true;
    }
  }
  if (fp != NULL) {
    fclose(fp);
    fp = NULL;
  }
  test_info_append("获取Tone 不应该到这里!");
  return false;
}

int visa_class_test() {
  RigolSpectrum *rs = new RigolSpectrum();

  //Sleep(20* 1000);    

  //if (!rs->InitSpectrum("2425MHZ", "100KHZ", "20DBM")) {
  if (!rs->InitSpectrum("2425MHZ", "2500KHZ", "20DBM")) {    
    cout << "can't connect to Rigol Device!" << endl;
    return -1;
  }

  unsigned short tune = 0x1;
 

  socket_init();

  do {
    cout << "======================设置 Tune [0x" << hex << (tune&0xffff) << "]======================" << endl;
    if (!visa_amber_commander_set_tune(tune)) {
      cout <<  "设置Tone 失败, 请检查jlink连接" << endl;
      return -2;
    }
    Sleep(10 *1000);
  
    g_sock = socket_client_open("192.168.100.103", 6666);
    if (g_sock == NULL) {
      cout << "connect to gateway failed!" << endl;
      return -2;
    }

    if (!visa_amber_tone_start(15, 20)) {
      cout <<  "启动载波发射失败" << endl;
      socket_close(g_sock);
      g_sock = NULL;    
      return -2;
    }  

    socket_close(g_sock);
    g_sock = NULL;

    Sleep(1 * 1000);

    double power;
    double frequency;
    if (!rs->CapturePeak("1", power, frequency)) {
      cout << "Capture Peak Value Failed" << endl;
      return -2;
    }
    cout << "power : " << power << ",frequency:" << frequency << endl;
    double delt = (frequency - 2425*1000*1000.0f)/1000.0f;
    cout << "误差 :" << delt << " KHZ" << endl;

    tune ++;
  } while (tune < 0x1ff);
  
  return 0;
}

int visa_class_test_1() {
  RigolSpectrum *rs = new RigolSpectrum();

  if (!rs->InitSpectrum("2425MHZ", "100KHZ", "20DBM")) {
  //if (!rs->InitSpectrum("2425MHZ", "2500KHZ", "20DBM")) {    
    cout << "can't connect to Rigol Device!" << endl;
    return -1;
  }

  double power;
  double frequency;
  if (!rs->CapturePeak("1", power, frequency)) {
    cout << "Capture Peak Value Failed" << endl;
    return -2;
  }
  cout << "power : " << power << ",frequency:" << frequency << endl;
  double delt = (frequency - 2425*1000*1000.0f)/1000.0f;
  cout << "误差 :" << delt << " KHZ" << endl;

  return 0;
}

bool visa_amber_cal(  RigolSpectrum *rs, double &_power, double &_frequency) {
  //unsigned short tune = 0x12c;
  //unsigned short tune = 0x130;
  //unsigned short tune = 0xda;
  unsigned short tune = 0x150;
  int cnt = 0;
  char cal_buf[256];

  int span = 500;
  if (!rs->InitSpectrum("2425MHZ", "500KHZ", "20DBM")) {    
    sprintf(cal_buf, "设置SPAN to %dKHZ", span);
    cout << cal_buf << endl;
    test_info_append(cal_buf);          
    return false;
  }

  if (!visa_amber_commander_get_tune(tune)) {
    sprintf(cal_buf, "Get Tune Failed!");
    cout << cal_buf << endl;
    test_info_append(cal_buf);          
    return false;    
  } 
  sprintf(cal_buf, "Get Tune OK: 0x%04X", tune&0xffff);
  cout << cal_buf << endl;
  test_info_append(cal_buf);
  Sleep(1000);
  
  do {

    int capcnt = 0;
    double power;
    double frequence;
    do {
      sprintf(cal_buf, "======================设置 Tune [ 0x%x]======================", tune&0xffff);
      cout << cal_buf << endl;
      test_info_append(cal_buf);
      
      if (!visa_amber_commander_set_tune(tune)) {
	sprintf(cal_buf, "设置Tone 失败， 请检查 JLink连接是否正常");
	cout << cal_buf << endl;
	test_info_append(cal_buf);      
	return false;
      }


      sprintf(cal_buf, "等待设备重新启动...");
      cout << cal_buf << endl;
      test_info_append(cal_buf);    
      int cnt = 10;
      while (cnt > 0) {
	Sleep(1000);
	cnt--;
      
	char buf[32];
	sprintf(buf, "%d 秒", cnt);
	test_info_append(buf);            
      }

    recap:                
      sprintf(cal_buf, "启动载波发射(持续3秒)..");
      cout << cal_buf << endl;
      test_info_append(cal_buf);    
      if (!visa_amber_tone_start(15, 20)) {
	sprintf(cal_buf, "启动载波发射失败");
	cout << cal_buf << endl;
	test_info_append(cal_buf);            
	return false;
      }
    
      cnt = 1;
      while (cnt > 0) {
	Sleep(1000);
	cnt--;
      
	char buf[32];
	sprintf(buf, "%d 秒", cnt);
	test_info_append(buf);            
      }    

      sprintf(cal_buf,"捕获Peak...");
      cout << cal_buf << endl;
      test_info_append(cal_buf);        

      if (!rs->CapturePeak("1", power, frequence) ) {
	sprintf(cal_buf, "捕获Peak 失败!");
	cout << cal_buf << endl;
	test_info_append(cal_buf);      
	return false;
      }

      cnt = 2;
      while (cnt > 0) {
	Sleep(1000);
	cnt--;
	char buf[32];
	sprintf(buf, "%d 秒", cnt);
	test_info_append(buf);            
      }      

      //if (power < 0.0f) {
      //if (power < -20.0f) {
      //if (power < 10.0f) {
      if (power < amber_power_limit) {				      
       // if (power < 5.0) {			
	sprintf(cal_buf, "捕获　功率:%f dBm, 功率太小, 等待３秒重新尝试..", power);
	cout << cal_buf << endl;
	test_info_append(cal_buf);
	capcnt++;
	goto recap;
      }
      break;
    } while (capcnt < 3);

    if (capcnt >= 3) {
      return false;
    }

    double delt = (frequence - 2425*1000*1000.0f)/1000.0f;
    sprintf(cal_buf, "发射功率:%f dBm, 频率:%f KHZ, 频偏:%f KHZ", power, frequence/1000.0f, delt);
    cout << cal_buf << endl;
    test_info_append(cal_buf);

    if (span > 100) {
      if (delt > -20.0f  && delt < 20.0f /* KHZ */) {
	sprintf(cal_buf, "发射功率:%f dBm, 频率:%f KHZ, 频偏:%f KHZ, 调整SPAN  到 100KHZ, 重新测量 ...", power, frequence/1000.0f, delt);
	cout << cal_buf << endl;
	test_info_append(cal_buf);	
	span = 100;
	if (!rs->InitSpectrum("2425MHZ", "100KHZ", "20DBM")) {    
	  sprintf(cal_buf, "设置SPAN to %dKHZ", span);
	  cout << cal_buf << endl;
	  test_info_append(cal_buf);          
	  return false;
	}
	goto recap;	
      }
    } else {
      if (delt > -5.0f  && delt < 5.0f /* KHZ */) {
	sprintf(cal_buf, "校准OK");
	cout << cal_buf << endl;
	test_info_append(cal_buf);
	_power = power;
	_frequency = frequence;
	return true;
      }
    }

    if (delt > 0) {
      //int xx = (int)(delt/4.167f);
      int xx = (int)(delt/0.625f);      
      if (xx == 0) {
	xx = 1;
      }
      tune += xx;
      
    } else {
      //int xx = (int)((-delt)/4.167f);
      int xx = (int)((-delt)/0.625f);      
      if (xx == 0) {
	xx = 1;
      }      
      tune -= xx;
    }
    cnt++;
    
  } while (cnt <= 20 && (tune <= 0x1ff && tune > 1));

  sprintf(cal_buf, "校准失败");
  cout << cal_buf << endl;
  test_info_append(cal_buf);  
  return false;
}

bool visa_amber_rf_power_test(double &power, double &frequence, double &delt_frequence, string &mac) {
  power = 0.0f;
  frequence = 0.0f;
  delt_frequence = 0.0f;
  char cal_buf[512];

  sprintf(cal_buf, "初始化频谱仪...");
  cout << cal_buf << endl;
  test_info_append(cal_buf);      
  RigolSpectrum *rs = new RigolSpectrum();
  //if (!rs->InitSpectrum("2425MHZ", "100KHZ", "20DBM")) {
  if (!rs->InitSpectrum("2425MHZ", "500KHZ", "20DBM")) {    
    sprintf(cal_buf, "初始化频谱仪失败!");
    cout << cal_buf << endl;
    test_info_append(cal_buf);          
    return false;
  }

  sprintf(cal_buf, "获取设备信息..");
  cout << cal_buf << endl;
  test_info_append(cal_buf);
  int choujian_checked = IsDlgButtonChecked(hwndST, ID_CHOUJIAN_AMER);  
  if (!visa_amber_commander_get_id(mac) && !choujian_checked) {
    sprintf(cal_buf, "获取设备信息失败, 请检查Jlink是否插入电脑并和设备连接!");
    cout << cal_buf << endl;
    test_info_append(cal_buf);                	
    return false;
  }

  Sleep(2000);
  if (!choujian_checked)  {
    sprintf(cal_buf, "校准频偏..");
    cout << cal_buf << endl;
    test_info_append(cal_buf);              
    if (!visa_amber_cal(rs, power, frequence)) {
      sprintf(cal_buf, "校准频偏失败!");
      cout << cal_buf << endl;
      test_info_append(cal_buf);                  
      return false;
    }
  } else {
    int try_cnt = 4;
    while (try_cnt > 0) {
      sprintf(cal_buf, "启动载波发射(持续3秒)..");
      cout << cal_buf << endl;
      test_info_append(cal_buf);
      
      if (!visa_amber_tone_start(15, 20)) {
   	try_cnt--;
	if (try_cnt <= 0) {
	  return false;
	}
	sprintf(cal_buf, "启动载波发射失败, 重试");
	cout << cal_buf << endl;
	test_info_append(cal_buf);
	Sleep(1000);
	continue;
      }
    
      int cnt = 1;
      while (cnt > 0) {
	Sleep(1000);
	cnt--;
      
	char buf[32];
	sprintf(buf, "%d 秒", cnt);
	test_info_append(buf);            
      }    
  
      sprintf(cal_buf, "捕获Peak...");
      cout << cal_buf << endl;
      test_info_append(cal_buf);                
      if (!rs->CapturePeak("1", power, frequence)) {
	try_cnt--;
	if (try_cnt <= 0) {
	  return false;
	}
	sprintf(cal_buf, "捕获Peak 失败!, 重试");
	cout << cal_buf << endl;
	test_info_append(cal_buf);
	Sleep(2500);
	continue;
      }

      //if (power < 5.0) {
      if (power < amber_power_limit) {				
	try_cnt--;
	if (try_cnt <= 0) {
	  return false;
	}
	sprintf(cal_buf, "捕获　功率:%f dBm, 功率太小, 等待３秒重新尝试..", power);
	cout << cal_buf << endl;
	test_info_append(cal_buf);
	Sleep(2500);	
	continue;
      }
      
      
      double delt = (frequence - 2425*1000*1000.0f)/1000.0f;
      //if (!(delt > -5.0f  && delt < 5.0f /* KHZ */)) {
      if (!(delt > -20.0f  && delt < 20.0f /* KHZ */)) {	
	try_cnt--;
	if (try_cnt <= 0) {
	  return false;
	}	
	sprintf(cal_buf, "频骗过大, frequence:%f KHZ, delt:%f KHZ，重新测试..", frequence/1000.0f, delt);
	cout << cal_buf << endl;
	test_info_append(cal_buf);
	Sleep(2500);	
	continue;
      }      
      break;
    } // while (try_cnt > 0）
  }
  
  delt_frequence = (frequence - 2425*1000*1000.0f)/1000.0f;
  return true;
}

bool visa_nxp_tone_start(int channel, int power) {
  char val[128];
  int ret = cmd_request_st(val);
  if (ret != 0) {
    return false;
  }  
  return true;
}
bool visa_nxp_rf_power_test(double &power, double &frequence, double &delt_frequence, string &mac) {
  cout << "NXP rf.." << endl;
  mac="0000000000000000";
  power = 0.0f;
  frequence = 0.0f;
  delt_frequence = 0.0f;
  char cal_buf[512];

  sprintf(cal_buf, "初始化频谱仪...");
  cout << cal_buf << endl;
  test_info_append(cal_buf);      
  RigolSpectrum *rs = new RigolSpectrum();
  //if (!rs->InitSpectrum("2425MHZ", "100KHZ", "20DBM")) {
  if (!rs->InitSpectrum("2480MHZ", "100KHZ", "20DBM")) {    
    sprintf(cal_buf, "初始化频谱仪失败!");
    cout << cal_buf << endl;
    test_info_append(cal_buf);          
    return false;
  }
  
  if (!visa_nxp_tone_start(15, 20)) {
    sprintf(cal_buf, "启动载波发射失败");
    cout << cal_buf << endl;
    test_info_append(cal_buf);            
    return false;    
  }



  int cnt = 1;
  while (cnt > 0) {
    Sleep(1000);
    cnt--;
    char buf[32];
    sprintf(buf, "%d 秒", cnt);
    test_info_append(buf);            
  }        
  
  sprintf(cal_buf, "捕获Peak...");
  cout << cal_buf << endl;
  test_info_append(cal_buf);                
  if (!rs->CapturePeak("1", power, frequence)) {
    sprintf(cal_buf, "捕获Peak 失败!");
    cout << cal_buf << endl;
    test_info_append(cal_buf);                    
    return false;
  }

  delt_frequence = (frequence - 2480*1000*1000.0f)/1000.0f;
  sprintf(cal_buf, "发射功率:%f dBm, 频率:%f KHZ, 频偏:%f KHZ", power, frequence/1000.0f, delt_frequence);
  cout << cal_buf << endl;
  test_info_append(cal_buf);


  if (power < amber_power_limit) {
    return false;
  }
  //if (delt_frequence <=30.0f * 1000.0f && delt_frequence >= -30.0f * 1000.0f) {
  if (delt_frequence <=30.0f && delt_frequence >= -30.0f) {    
    return true;
  }
  
  return false;
}

bool visa_ble_tone_start(int channel, int power) {
  char val[128];
  //2402
  //(f - chan)/2 = chan
  //f = chan* 2 + 2402 = 21 * 2 + 2402 = 2402 + 42 = 2444
  int ret = cmd_request_st_ble(val);
  if (ret != 0) {
    return false;
  }  
  return true;
}
bool visa_ble_rf_power_test(double &power, double &frequence, double &delt_frequence, string &mac) {
  cout << "BLE rf.." << endl;
  mac="0000000000000000";
  power = 0.0f;
  frequence = 0.0f;
  delt_frequence = 0.0f;
  char cal_buf[512];

  sprintf(cal_buf, "初始化频谱仪...");
  cout << cal_buf << endl;
  test_info_append(cal_buf);      
  RigolSpectrum *rs = new RigolSpectrum();
  //if (!rs->InitSpectrum("2425MHZ", "100KHZ", "20DBM")) {
  if (!rs->InitSpectrum("2444MHZ", "100KHZ", "20DBM")) {    
    sprintf(cal_buf, "初始化频谱仪失败!");
    cout << cal_buf << endl;
    test_info_append(cal_buf);          
    return false;
  }
  
  if (!visa_ble_tone_start(15, 20)) {
    sprintf(cal_buf, "启动载波发射失败");
    cout << cal_buf << endl;
    test_info_append(cal_buf);            
    return false;    
  }



  int cnt = 1;
  while (cnt > 0) {
    Sleep(1000);
    cnt--;
    char buf[32];
    sprintf(buf, "%d 秒", cnt);
    test_info_append(buf);            
  }        
  
  sprintf(cal_buf, "捕获Peak...");
  cout << cal_buf << endl;
  test_info_append(cal_buf);                
  if (!rs->CapturePeak("1", power, frequence)) {
    sprintf(cal_buf, "捕获Peak 失败!");
    cout << cal_buf << endl;
    test_info_append(cal_buf);                    
    return false;
  }

  delt_frequence = (frequence - 2444*1000*1000.0f)/1000.0f;
  sprintf(cal_buf, "发射功率:%f dBm, 频率:%f KHZ, 频偏:%f KHZ", power, frequence/1000.0f, delt_frequence);
  cout << cal_buf << endl;
  test_info_append(cal_buf);


  if (power < amber_power_limit) {
    return false;
  }
  //if (delt_frequence <=30.0f * 1000.0f && delt_frequence >= -30.0f * 1000.0f) {
  if (delt_frequence <=30.0f && delt_frequence >= -30.0f) {    
    return true;
  }
  
  return false;
}

#else
bool visa_amber_rf_power_test(double &power, double &frequence, double &delt_frequence, string &mac) {
  return false;
}
bool visa_nxp_rf_power_test(double &power, double &frequence, double &delt_frequence, string &mac) {
  return false;
}
#endif
