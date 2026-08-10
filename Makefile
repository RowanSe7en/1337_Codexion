# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/01/08 21:41:29 by brouane           #+#    #+#              #
#    Updated: 2026/08/10 21:17:37 by brouane          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CC		= cc
CFLAGS	= -Wall -Wextra -Werror -pthread
RM		= rm -f

NAME	= codexion

SRCS	= coders/codexion.c \
		  coders/parser_helper.c \
		  coders/parser.c \
		  coders/time.c \
		  coders/printers.c \
		  coders/mutex_manager.c \
		  coders/freedom.c \
		  coders/thread_manager.c \
		  coders/watcher.c \
		  coders/dongle.c \
		  coders/edf.c \
		  coders/main_loop.c \
		  coders/program_starter.c \
		  coders/getters.c \
		  coders/fifo.c \
		  coders/setters.c \
		  coders/init.c \
		  coders/pair_scheduler.c

OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
