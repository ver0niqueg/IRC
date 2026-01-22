#ifndef COLORS_HPP
#define COLORS_HPP

#define DEFAULT       "\033[0m"             // Reset color to terminal default

#define PASTEL_VIOLET "\033[1;38;5;183m"    // informational / lifelcycle logs (server start, client connect/disconnect, general INFO)
#define PASTEL_GREEN  "\033[1;38;5;120m"    // Success/confirmation and positive statuses (checks, operations completed, send logs)
#define PASTEL_RED    "\033[1;38;5;203m"    // When server is stopped or errors / critical failures (error messages, fatal conditions)
#define PASTEL_BLUE   "\033[1;38;5;75m"     // Incoming data / receive logs (RECV: bytes received, client input)
#define PASTEL_YELLOW "\033[1;38;5;228m"    // Connection-related notices (CONNECTION, DISCONNECTION messages)
#define PASTEL_ORANGE "\033[1;38;5;215m"    // Warnings and non-fatal alerts (WARN messages)

#endif
