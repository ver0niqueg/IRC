#ifndef COLORS_HPP
#define COLORS_HPP

#define DEFAULT       "\033[0m"          // Reset color to terminal default

#define PASTEL_VIOLET "\033[1;38;5;183m" // General information (server state, setup)
#define PASTEL_GREEN  "\033[1;38;5;120m" // Send messages (successful operations, confirmations)
#define PASTEL_RED    "\033[1;38;5;203m" // Info when server stops or errors (invalid commands, failures)
#define PASTEL_BLUE   "\033[1;38;5;75m"  // Receive messages (incoming data, client actions)
#define PASTEL_YELLOW "\033[1;38;5;228m" // Warnings (non-fatal issues, edge cases)
#define PASTEL_ORANGE "\033[1;38;5;215m" // Important notices (disconnects, critical events)

#endif
