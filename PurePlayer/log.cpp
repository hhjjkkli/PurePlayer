#include "log.h"
#include <sstream>

std::mutex mymutex;

std::string filter = "";
char* Log::caller;
std::string Log::callerString;

void log(char* logStr)
{
	FILE *fp;
	int len = (int)strlen(logStr);
	fopen_s(&fp, "log.txt", "a+");
	if (fp == NULL) return;
	logStr[len] = '\n';//添加换行符在这里，就不用在其它地方重复添加了
	logStr[len + 1] = '\0';

	if (false == filter.empty()) {
		if (((std::string)logStr).find(filter) == -1) {
			fclose(fp);
			return;
		}
	}

	fwrite(logStr, strlen(logStr), 1, fp);
	//OutputDebugStringA(logStr);
	fclose(fp);
}

void _stdcall log(char* caller, const char *format, ...){
	mymutex.lock();
	char buf[4096], *p = buf;
	va_list args;
	va_start(args, format);
#pragma warning(push)
#pragma warning(disable: 4996) // Deprecation
	p += _vsnprintf(p, sizeof buf - 1, format, args);
#pragma warning(pop)	
	va_end(args);
	char s[5096]; // 设为1024会报错
	std::strcpy(s, caller);
	std::strcat(s, buf);
	log(s);
	mymutex.unlock();
}

void _stdcall log(const char *format, ...) {
	mymutex.lock();
	char buf[4096], *p = buf;
	va_list args;
	va_start(args, format);
#pragma warning(push)
#pragma warning(disable: 4996) // Deprecation
	p += _vsnprintf(p, sizeof buf - 1, format, args);
#pragma warning(pop)	
	va_end(args);
	log(buf);
	mymutex.unlock();
}

void logc(char* ft){
	filter = ft;
	FILE *fp;
	fopen_s(&fp, "log.txt", "w");
	fclose(fp);
}

void logc() {
	filter = "";
	FILE *fp;
	fopen_s(&fp, "log.txt", "w");
	fclose(fp);
}

const std::string int2str(const int &int_temp)
{
	std::stringstream stream;
	stream << int_temp;
	return stream.str();   //此处也可以用 stream>>string_temp  
}
