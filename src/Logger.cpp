#include "Logger.hpp"

Logger::Logger(void) { file.open("info.log", std::ios::app); }
Logger::~Logger(void) { file.close(); }

Logger *Logger::get() {
	if (ptr == NULL) {
		ptr = new Logger();
	}
	return ptr;
}

void Logger::log(std::string level, std::string msg) {
	time_t	   rawtime;
	struct tm *timeinfo;
	char	   timebuf[100];

	std::time(&rawtime);
	timeinfo = std::localtime(&rawtime);

	std::strftime(timebuf, sizeof(timebuf), "%d-%m-%Y %H:%M:%S", timeinfo);

	file << "[" << timebuf << "]"
		 << "[" << level << "] " << msg << std::endl;
}

Logger *Logger::ptr = NULL;
