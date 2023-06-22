#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

#define INFO "INFO"
#define WARNING "WARNING"
#define ERROR "ERROR"

class Logger {
	private:
	Logger(void);
	static Logger *ptr;

	public:
	static Logger *get();
	void		   log(std::string level, std::string msg);
	std::ofstream  file;
	~Logger(void);
};

#endif
