#ifndef __log_H__
#define __log_H__

#include "stdafx.h"
#include <stdio.h>
#include <string>
#include <stdarg.h>
#include "LogClass.h"
#include <mutex>

extern std::mutex mymutex;

#pragma warning(disable:4996)

#define GETCALLERINFO mymutex.lock();Log::callerString = "";Log::callerString.append(__FILE__);Log::callerString=Log::callerString.substr(Log::callerString.find_last_of("\\")+1);Log::callerString.append(",");Log::callerString.append(int2str(__LINE__));Log::callerString.append(",");Log::callerString.append(__FUNCTION__);Log::callerString.append(": ");Log::caller=const_cast<char*>(Log::callerString.c_str());mymutex.unlock();
//#define DEBUG
#ifdef DEBUG
#define logd(caller,format,...) log(caller,format,##__VA_ARGS__)
#else
#define logd(caller,format,...)
#endif // DEBUG

const std::string int2str(const int &int_temp);
extern std::string filter;
void _stdcall log(const char *format, ...);
void _stdcall log(char* caller, const char *format, ...);
void logc(char* ft);
void logc();

#endif // !__log_H__
