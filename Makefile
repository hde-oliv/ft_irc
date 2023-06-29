SRC_DIR	:=	src
OBJ_DIR	:=	obj

SRC		:=	Server.cpp main.cpp Utils.cpp Channel.cpp Client.cpp
SRCS	:=	$(addprefix $(SRC_DIR)/,$(SRC))
OBJS	:=	$(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
CC		:=	c++
CFLAGS	+=	-Wall -Wextra -Werror -std=c++98 -g
RM		:=	rm -rf
NAME	:=	ircserv


all:		$(NAME)

teste: all
	./ft_irc 1234 8000

$(OBJ_DIR)/%.o:		$(SRC_DIR)/%.cpp
			$(CC) $(CFLAGS) -c $< -o $@

$(NAME):	$(OBJS)
			$(CC) $(OBJS) $(CFLAGS) -o $(NAME)

clean:
			$(RM) $(OBJS)

fclean: 	clean
			$(RM) $(NAME)

re: 		fclean all

.PHONY: 	all clean fclean re
