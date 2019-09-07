#include "help.h"

static String Help::Sprintf(char *fmt, ...) {
	char *	buffer = new char[1024];
	String	str;

	va_list args;
	va_start(args, fmt);
	_vbprintf(buffer, 1024, fmt, args);
	va_end(args);

	str = String(buffer);
	delete buffer;

	return str;
}
