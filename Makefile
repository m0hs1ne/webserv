SHELL = /bin/bash

NAME		= webserv
CC			= g++
CFLAGS		= -Wall -Werror -Wextra -std=c++98 -fsanitize=address -g
RM			= rm -f
ECHO		= echo -e

DEF_COLOR	=	\033[0;39m
ORANGE		=	\033[0;33m
GRAY		=	\033[0;90m
RED			=	\033[0;91m
GREEN		=	\033[1;92m
YELLOW		=	\033[1;93m
BLUE		=	\033[0;94m
MAGENTA		=	\033[0;95m
CYAN		=	\033[0;96m
WHITE		=	\033[0;97m


SRC = Server/main.cpp Server/webserv.cpp server/ParsingConfig.cpp Server/Request.cpp Server/Response.cpp code/tools.cpp \
	code/GetMethod/handlingGet.cpp code/GetMethod/Autoindex.cpp  code/handlingCgi/handlingCGI.cpp code/PostMethod/handlingPost.cpp \
	code/DeleteMethod/handlingDelete.cpp

all:		$(NAME)

$(NAME):	$(SRC)
			@$(ECHO) -n "$(YELLOW)[$(NAME)]:\t$(DEF_COLOR)"
			@$(CC) $(CFLAGS) $(SRC) -o $(NAME)
			@$(ECHO) "$(GREEN) => Success!$(DEF_COLOR)"

clean:

fclean:		clean
			@$(RM) $(NAME)
			@find . -name ".DS_Store" -delete
			@rm -rf webserv.dSYM
			@$(ECHO) -n "$(CYAN)[webServ]:\texec. files$(DEF_COLOR)$(GREEN)  => Cleaned!$(DEF_COLOR)\n"


re:			fclean  all 
			@$(ECHO) -n "$(GREEN)Cleaned and rebuilt everything for [webServ]!$(DEF_COLOR)\n"

.PHONY:		all clean fclean re