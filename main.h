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
	typedef struct  {
		char singleChar;
		char *fullString;
	}Delimiter;

	int RegisterHotKey_user(char *hotkeys);

	
	Delimiter d;
#ifdef __cplusplus
}
#endif

#endif