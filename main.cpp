#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "socket.h"
#include "proto.h"
#include "sheet.h"
#include "cmd.h"
#include "ssh2.h"

#include <iostream>


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

INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SetIp(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PirTest(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

int state = S_DISCONNECTTED;

int pir_test_resouse_init();

int main(int argc, char *argv[]) {
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
  
  g_sock = socket_client_open(g_gwip, g_gwport);
  //g_sock = socket_client_open("127.0.0.1", 9898);
  if (g_sock == NULL) {
    return -1;
  }
  return 0;
}

int target_disconnect() {
  cout << "target disconnect" << endl;
  socket_close(g_sock);
  g_sock = NULL;
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
  int ret = libssh2_main(g_gwip, 22, "root", "root", "if [ -e /etc/config/dusun/nxp/netinfo ]; then echo 1; else echo 0; fi", val, &size);
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

  MessageBox(g_hwnd, "请按下网关的按钮 ,进行按钮测试, 并迅速点确定按钮!!!", "按钮 Test准备", MB_OK);  
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
  BookHandle bh = sheet_open(path, name, "Sheet1", 0, cols, cnt);

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
static void test_info_append(const char *str) {
  //IDC_INFO
#if 1
  string info_tmp = info_str;
  info_str = std::string(str);
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
  
  BookHandle bh = sheet_open(path, name, "Sheet2", 1, cols, cnt);
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
