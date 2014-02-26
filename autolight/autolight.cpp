// autolight.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "dbg.h"
#include <Windows.h>
#include <iostream>
#include <lowlevelmonitorconfigurationapi.h>

using namespace std;

const BYTE VCP_CODE_BRIGHTNESS = 0x10;

HWND GetActualDesktopWindow() {
	HWND hDesktopWnd = GetDesktopWindow();
	HWND hDefView = FindWindowEx(hDesktopWnd, NULL, _T("SHELLDLL_DefView"), NULL);
	if (hDefView != NULL) {
		return FindWindowEx(hDefView, NULL, _T("SysListView32"), NULL);
	} else {
		return FindWindowEx(hDesktopWnd, NULL, _T("WorkerW"), NULL);
	}
}

bool IsFullScreenAppRunning() {
	RECT appBounds;
	RECT rc;

	// get foreground window
	HWND hWnd = GetForegroundWindow();

	// get the actual desktop window
	HWND actualDesktopView = GetActualDesktopWindow();

	// check whether full screen app is running
	if (hWnd != NULL && hWnd != actualDesktopView) {
		// get the size of desktop window as well as the foreground window
		GetWindowRect(GetDesktopWindow(), &rc);
		GetWindowRect(hWnd, &appBounds);
		log_info("The bounds of the foreground window is %d x %d.", appBounds.right - appBounds.left, appBounds.bottom - appBounds.top);
		return appBounds.bottom >= rc.bottom && appBounds.left <= rc.left && appBounds.right >= rc.right && appBounds.top <= rc.top;
	} else {
		log_info("Foreground window is either desktop or shell.");
		return false;
	}
}

bool SetBrightness(DWORD wTargetBrightness) {
	if (wTargetBrightness < 0) {
		return false;
	}

	// get forground window handle
	HWND hWnd = GetForegroundWindow();

	// get monitor handle
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

	// get how many physical monitors are associated with the handle
	DWORD dwPhysicalMonitorArraySize;
	if (!GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &dwPhysicalMonitorArraySize)) {
		return false;
	}

	// get physical monitors
	_PHYSICAL_MONITOR *pPhysicalMonitorArray = new _PHYSICAL_MONITOR[dwPhysicalMonitorArraySize];
	if (!GetPhysicalMonitorsFromHMONITOR(hMonitor, dwPhysicalMonitorArraySize, pPhysicalMonitorArray)) {
		return false;
	}

	// for each physical monitor
	for (int i = 0; i < dwPhysicalMonitorArraySize; i++) {
		// get physical monitor handle
		HANDLE hPhysicalMonitor = pPhysicalMonitorArray[i].hPhysicalMonitor;

		// get capability string
		// do we need to check capability string to make sure we can actually adjust brightness?
		/*DWORD dwCapabilitiesStringLengthInCharacters;
		if (!GetCapabilitiesStringLength(hPhysicalMonitor, &dwCapabilitiesStringLengthInCharacters)) {
			continue;
		}

		CHAR *szASCIICapabilitiesString = new CHAR[dwCapabilitiesStringLengthInCharacters];
		if (!CapabilitiesRequestAndCapabilitiesReply(hPhysicalMonitor, szASCIICapabilitiesString, dwCapabilitiesStringLengthInCharacters)) {
			continue;
		}*/
		// delete[] szASCIICapabilitiesString;

		// get the current brightness
		MC_VCP_CODE_TYPE pvct;
		DWORD wCurrentValue;
		DWORD wMaximumValue;
		if (!GetVCPFeatureAndVCPFeatureReply(hPhysicalMonitor, VCP_CODE_BRIGHTNESS, &pvct, &wCurrentValue, &wMaximumValue)) {
			continue;
		}

		// check if adjustment is needed
		if (wTargetBrightness != wCurrentValue && wTargetBrightness <= wMaximumValue) {
			if (!SetVCPFeature(hPhysicalMonitor, VCP_CODE_BRIGHTNESS, wTargetBrightness)) {
				// log failure
				continue;
			}
		}
	}

	delete[] pPhysicalMonitorArray;

	return true;
}

int _tmain(int argc, _TCHAR* argv[]) {
	// parse parameters
	DWORD dLowBrightness = 25;
	DWORD dHighBrightness = 75;
	DWORD dTestInterval = 60;
	if (argc > 1) {
		dLowBrightness = _wtoi(argv[1]);
		if (argc > 2) {
			dHighBrightness = _wtoi(argv[2]);
			if (argc > 3) {
				dTestInterval = _wtoi(argv[3]);
			}
		}
	}

	// print welcome information
	cout << "Welcome to autolight!" << endl;
	cout << "The low brightness is " << dLowBrightness << endl;
	cout << "The high brightness is " << dHighBrightness << endl;
	cout << "The brightness test interval is " << dTestInterval << endl;
	cout << "The app will going into background in 5 seconds." << endl;
	cout << "Enjoy!" << endl;

	Sleep(5 * 1000);
	FreeConsole();

	// start check loop
	DWORD dCurrentBrightness = -1;
	while (true) {
		if (IsFullScreenAppRunning()) {
			log_info("User is running full screen app.");
			if (dCurrentBrightness != dHighBrightness) {
				log_info("Try to set brightness to %d", dHighBrightness);
				if (SetBrightness(dHighBrightness)) {
					log_info("Successed.");
					dCurrentBrightness = dHighBrightness;
				} else {
					log_info("Failed.");
				}
			}
		} else {
			log_info("User is NOT running full screen app.");
			if (dCurrentBrightness != dLowBrightness) {
				log_info("Try to set brightness to %d", dLowBrightness);
				if (SetBrightness(dLowBrightness)) {
					log_info("Successed.");
					dCurrentBrightness = dLowBrightness;
				} else {
					log_info("Failed");
				}
			}
		}

		log_info("Sleep for %d seconds.", dTestInterval);
		Sleep(dTestInterval * 1000);
	}

	return 0;
}
