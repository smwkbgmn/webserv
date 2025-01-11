NAME		= webserv
CPP			= c++
FLAG		= -Wall -Wextra -Werror $(FLG_STD) -I src -I src/common -I src/http -I src/http/module -I src/core

# FLG_STD		= -std=c++98
FLG_STD		= -std=c++11

SRC			= \
	$(wildcard src/*.cpp) \
	$(wildcard src/common/*.cpp) \
	$(wildcard src/http/*.cpp) \
	$(wildcard src/http/module/*.cpp) \
	$(wildcard src/core/*.cpp)

OBJ         = $(patsubst src/%.cpp, obj/%.o, $(SRC))

# Define colors for pretty output
BLUE	= \033[34m
GREEN	= \033[32m
YELLOW	= \033[33m
RESET	= \033[0m

all			: $(NAME)

run			: all
			@echo "$(GREEN)Launching $(NAME)...$(RESET)"
			@./$(NAME)

$(NAME)		: $(OBJ)
			@echo "$(GREEN)Linking $(NAME)...$(RESET)"
			@$(CPP) $(FLAG) $(OBJ) -o $@
			@echo "$(GREEN)Build complete!$(RESET)"

obj/%.o		: src/%.cpp
			@mkdir -p $(@D)
			@echo "$(BLUE)Compiling $<...$(RESET)"
			@$(CPP) $(FLAG) -c $< -o $@

clean		:
			@echo "$(YELLOW)Cleaning object files...$(RESET)"
			@$(RM) -r obj

fclean		:
			@make clean
			@make rmlog
			@echo "$(YELLOW)Removing $(NAME)...$(RESET)"
			@$(RM) $(NAME)
			@echo "$(GREEN)Clean complete!$(RESET)"

rmlog		:
			@echo "$(YELLOW)Removing log files...$(RESET)"
			@$(RM) -r log

re			:
			@make fclean
			@make all

.PHONY		: all clean fclean re

# NAME		= webserv

# CPP			= c++
# FLAG		= -Wall -Wextra -Werror \
# $(FLG_STD) -I src -I src/common -I src/http -I src/http/module -I src/core


# # FLG_STD		= -std=c++98
# FLG_STD		= -std=c++11

# SRC			= \
# $(wildcard src/*.cpp) \
# $(wildcard src/common/*.cpp) \
# $(wildcard src/http/*.cpp) \
# $(wildcard src/http/module/*.cpp) \
# $(wildcard src/core/*.cpp)

# OBJ         = $(patsubst src/%.cpp, obj/%.o, $(SRC))

# all			: $(NAME)

# run			: all
# 			./$(NAME)

# $(NAME)		: $(OBJ)
# 			$(CPP) $(FLAG) $(OBJ) -o $@

# obj/%.o		: src/%.cpp
# 			  mkdir -p $(@D)
# 			  $(CPP) $(FLAG) -c $< -o $@

# clean		:
# 			$(RM) -r obj

# fclean		:
# 			make clean
# 			make rmlog
# 			$(RM) $(NAME)

# rmlog		:
# 			$(RM) -r log

# re			:
# 			make fclean
# 			make all

# .PHONY		: all celan fclean re
