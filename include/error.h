#ifndef _ERROR_H_
#define _ERROR_H_

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "menu.h"

#define STYLE_RESET       "\033[0m"    // default color
#define STYLE_GREEN_BOLD  "\033[1;32m" // green color with bold text
#define STYLE_BOLD        "\033[1m"    // default color with bold text
#define STYLE_RED_BOLD    "\033[1;31m" // red color with bold text
#define STYLE_YELLOW_BOLD "\033[1;33m"	// yellow color with bold text
#define STYLE_BLUE_BOLD   "\033[34;1m"	// blue color with bold text

#define ERROR(msg) {\
	fprintf(stderr,STYLE_BOLD "%s:%d" STYLE_RESET "" STYLE_RED_BOLD " ERROR:" \
        STYLE_RESET "" STYLE_BOLD " %s " STYLE_RESET "\n", __FILE__, __LINE__, msg);\
        kill(pid, SIGINT);\
        exit(EXIT_FAILURE);\
}

#define SYS_ERROR(msg) {\
	fprintf(stderr, STYLE_BOLD "%s:%d" STYLE_RESET "" STYLE_RED_BOLD " SYSTEM ERROR:" \
        STYLE_RESET "" STYLE_BOLD " %s: %s" STYLE_RESET "\n", __FILE__, __LINE__ ,msg ,strerror(errno));\
        kill(pid, SIGINT);\
        exit(EXIT_FAILURE);\
}

#endif // _ERROR_H_