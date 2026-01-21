################################################################################
#                                     COLORS                                   #
################################################################################

DEFAULT			= \033[0m
WHITE			= \033[1;37m
PASTEL_VIOLET 	= \033[1;38;5;183m
PASTEL_GREEN 	= \033[1;38;5;120m
PASTEL_RED 		= \033[1;38;5;203m
PASTEL_BLUE		= \033[1;38;5;75m


################################################################################
#                                 PROGRESS BAR                                 #
################################################################################

define PROGRESS_BAR
	@TOTAL_STEPS=20; CURRENT_STEP=0; \
	echo -n "["; \
	while [ $$CURRENT_STEP -lt $$TOTAL_STEPS ]; do \
		CURRENT_STEP=$$(($$CURRENT_STEP + 1)); \
		echo -n "$(PASTEL_GREEN)▰$(DEFAULT)"; \
		sleep 0.01; \
	done; \
	echo "] $(PASTEL_GREEN)100%$(DEFAULT)"
endef

################################################################################
#                                     CONFIG                                   #
################################################################################

NAME =			ircserv
CC =			c++
CFLAGS =		-Wall -Wextra -Werror -std=c++98
RM =			rm -f

SRC =			./srcs/
OBJ =			./objs/
INC =			./includes

# Source files
SRCS =			$(SRC)main.cpp \
				$(SRC)Server.cpp \
				$(SRC)Client.cpp \
				$(SRC)Channel.cpp \
				$(SRC)CommandHandler.cpp \
				$(SRC)commands/AuthCommands.cpp \
				$(SRC)commands/ChannelCommands.cpp \
				$(SRC)commands/MessageCommands.cpp \
				$(SRC)commands/OperatorCommands.cpp

# Converts source file paths to object file paths
OBJS =			$(patsubst $(SRC)%, $(OBJ)%, $(SRCS:.cpp=.o))

################################################################################
#                                     RULES                                    #
################################################################################

# Rule for compiling source files into object files
$(OBJ)%.o:		$(SRC)%.cpp
				@mkdir -p $(dir $@)
				@$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

# Rule for creating the executable
$(NAME):		$(OBJS)
				@echo "\n🔮 $(WHITE)Linking $(PASTEL_VIOLET)$(NAME)$(DEFAULT) executable\t\t\t"
				@$(CC) $(CFLAGS) -I$(INC) $(OBJS) -o $(NAME)
				$(PROGRESS_BAR)
				@echo ""

# Default rule
all:			$(NAME)

# Rule for cleaning up object files
clean:
				@echo "\n🧹 $(PASTEL_RED)Cleaning up $(PASTEL_VIOLET)project $(DEFAULT)object files\t\t"
				@$(RM) -r $(OBJ)
				$(PROGRESS_BAR)

# Full clean rule (objects files, executable and libraries)
fclean:			clean
				@echo "\n🗑️  $(PASTEL_RED)Deleting $(PASTEL_VIOLET)$(NAME)$(DEFAULT) executable\t\t"
				@$(RM) $(NAME)
				$(PROGRESS_BAR)
				@echo ""

# Rebuild rule
re:				fclean all

# Rule to compile the program with debugging flags
debug:			$(OBJS)
				@echo "\n🔗 Compiling in $(PASTEL_VIOLET)debug$(DEFAULT) mode\t\t\t"
				@$(CC) $(CFLAGS) -I$(INC) $(OBJS) -o $(NAME) -g3 -fsanitize=address
				$(PROGRESS_BAR)

# Rule to display help
help:
				@echo "\n$(PASTEL_VIOLET)all$(DEFAULT)		- Build the executable $(NAME)"
				@echo "$(PASTEL_VIOLET)clean$(DEFAULT)		- Clean up object files"
				@echo "$(PASTEL_VIOLET)fclean$(DEFAULT)		- Clean up all object files and executable"
				@echo "$(PASTEL_VIOLET)re$(DEFAULT)		- Rebuild the entire project"
				@echo "$(PASTEL_VIOLET)debug$(DEFAULT)		- Run the program with debugging flags -g3 -fsanitize=address\n"

# Rule to ensure that these targets are always executed as intended, even if there are files with the same name
.PHONY:			all clean fclean re debug help