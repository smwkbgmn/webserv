NAME		= webserv

CPP			= c++
FLAG		= -Wall -Wextra -Werror \
$(FLG_STD) -I src -I src/common -I src/http -I src/http/module -I src/core


# FLG_STD		= -std=c++98
FLG_STD		= -std=c++11

SRC			= \
$(wildcard src/*.cpp) \
$(wildcard src/common/*.cpp) \
$(wildcard src/http/*.cpp) \
$(wildcard src/http/module/*.cpp) \
$(wildcard src/core/*.cpp)

# OBJ			= $(SRC:.cpp=.o)
# OBJ			= $(patsubst src/%.cpp, obj/%.o,$(SRC))
OBJ         = $(patsubst src/%.cpp, obj/%.o, $(SRC))

all			: $(NAME)

run			: all
			./$(NAME)

$(NAME)		: $(OBJ)
			$(CPP) $(FLAG) $(OBJ) -o $@

obj/%.o		: src/%.cpp
			  mkdir -p $(@D)
			  $(CPP) $(FLAG) -c $< -o $@

# %.o			: %.cpp
# 			$(CPP) $(FLAG) -c $< -o $@

clean		:
			$(RM) -r obj
#			$(RM) $(OBJ)

fclean		:
			make clean
			make rmlog
			$(RM) $(NAME)

rmlog		:
#			$(RM) $(wildcard log/*.log)
			$(RM) -r log

re			:
			make fclean
			make all

.PHONY		: all celan fclean re
