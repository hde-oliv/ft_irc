SRC_DIR	:=	src
OBJ_DIR	:=	obj

SRC		:=	Server.cpp main.cpp
SRCS	:=	$(addprefix $(SRC_DIR)/,$(SRC))
OBJS	:=	$(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
CC		:=	c++
CFLAGS	+=	-Wall -Wextra -Werror -std=c++98
RM		:=	rm -rf
NAME	:=	ft_irc

all:		$(NAME)

$(OBJ_DIR)/%.o:		$(SRC_DIR)/%.cpp
			$(CC) $(CFLAGS) -c $< -o $@

$(NAME):	$(OBJS)
			$(CC) $(OBJS) $(CFLAGS) -o $(NAME)

clean:
			$(RM) $(OBJ)

fclean: 	clean
			$(RM) $(NAME)

re: 		fclean all

.PHONY: 	all clean fclean re
