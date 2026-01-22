#ifndef COLORS_HPP
#define COLORS_HPP

#define DEFAULT       "\033[0m"             // reset color to terminal default

#define PASTEL_VIOLET "\033[1;38;5;183m"    // info / lifelcycle logs
#define PASTEL_GREEN  "\033[1;38;5;120m"    // success + positive statuses
#define PASTEL_RED    "\033[1;38;5;203m"    // when server is stopped or errors / critical failures
#define PASTEL_BLUE   "\033[1;38;5;75m"     // incoming data / receive logs
#define PASTEL_YELLOW "\033[1;38;5;228m"    // connection messages
#define PASTEL_ORANGE "\033[1;38;5;215m"    // warnings and non-fatal alerts

#endif
