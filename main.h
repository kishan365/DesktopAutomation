#pragma once
#ifndef FILE1_H
#define FILE1_H

#ifdef __cplusplus
extern "C" {
#endif
	DWORD mainthreadId;
	typedef struct {
		char *hotkeys;
		int id;
	}HotkeyRequest;

	int RegisterHotKey_user(char *hotkeys, int actionID);
	

#ifdef __cplusplus
}
#endif

#endif