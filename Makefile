###############################################################################
#### executable program's name ################################################
NAME = webserv

###############################################################################
#### path and variables #######################################################
###############################################################################

SRC_DIR = ./src/
OBJ_DIR = ./obj/
HEADER_DIR = ./inc/

CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -g3
RM = rm -f

GREEN = \033[0;32m
BLUE = \033[0;34m
BOLD = \033[1m
RESET = \033[0;m

########### source files and objects ##########################################
###############################################################################

SRC_FILES = main.cpp utils.cpp cgi.cpp config_parser.cpp tokenizer.cpp \
			Listen.cpp Server.cpp HTTPRequest.cpp \
			Client.cpp handleDelete.cpp handleGet.cpp handlePost.cpp

OBJ = $(addprefix $(OBJ_DIR), $(SRC_FILES:.cpp=.o))

HEADER = -I$(HEADER_DIR)

#### rule by default ##########################################################
###############################################################################
all: $(NAME)

#### create objects (.o) from source files (.c) ###############################
###############################################################################

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) $(HEADER) -c $< -o $@
	@echo "$(BOLD)$(BLUE)Compiling $< $(RESET)"

#### linking objects in executable ############################################
###############################################################################

$(NAME): $(OBJ)
	@$(CC) $(CFLAGS) $(HEADER) $(OBJ) -o $(NAME)
	@echo "$(BOLD)$(GREEN)Compilation of $(NAME) is done ✔️$(RESET)"

#### clean objects ############################################################

clean:
	@echo "$(BOLD)$(BLUE)Deleting all binary objects, wait...$(RESET)"
	@$(RM) -r $(OBJ_DIR)/*.o
	@$(RM) -rf $(OBJ_DIR)

#### clean objects and executable #############################################

fclean: clean
	@echo "$(BOLD)$(BLUE)Deleting executable.$(RESET)"
	@$(RM) $(NAME)

###############################################################################

re: fclean all

.PHONY: all clean fclean re print

###############################################################################