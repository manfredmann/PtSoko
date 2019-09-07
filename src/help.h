#ifndef HELP_H
#define HELP_H

#include <stdio.h>
#include <stdarg.h>
#include <String.h>

class Help {
	public:
		static String Sprintf(char *fmt, ...);
};

#endif