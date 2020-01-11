#ifndef _ERROR_H_
#define _ERROR_H_

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define STYLE_RESET       "\033[0m"		// default color
#define STYLE_GREEN_BOLD  "\033[1;32m"	// green color with bold text
#define STYLE_BOLD        "\033[1m"		// default color with bold text
#define STYLE_RED_BOLD    "\033[1;31m"	// red color with bold text

#define ERROR(msg) {\
	fprintf(stderr,STYLE_BOLD "%s:%d" STYLE_RESET "" STYLE_RED_BOLD " ERROR:" \
        STYLE_RESET "" STYLE_BOLD " %s " STYLE_RESET "\n", __FILE__, __LINE__, msg);\
         exit(EXIT_FAILURE);\
}

#define SYS_ERROR(msg) {\
	fprintf(stderr, STYLE_BOLD "%s:%d" STYLE_RESET "" STYLE_RED_BOLD " SYSTEM ERROR:" \
        STYLE_RESET "" STYLE_BOLD " %s: %s" STYLE_RESET "\n", __FILE__, __LINE__ ,msg ,strerror(errno));\
         exit(EXIT_FAILURE);\
}

#define LOG(msg) {\
	fprintf(stdout, STYLE_GREEN_BOLD "%s " STYLE_RESET "\n", msg ); \
}

#endif // _ERROR_H_