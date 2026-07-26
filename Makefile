# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/01/08 21:41:29 by brouane           #+#    #+#              #
#    Updated: 2026/07/25 18:26:29 by brouane          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CC		= cc
CFLAGS	= -Wall -Wextra -Werror -pthread
RM		= rm -f

NAME	= codexion

# SRCS	= coders_old/codexion.c \
# 		  coders_old/parser_helper.c \
# 		  coders_old/parser.c \
# 		  coders_old/time.c \
# 		  coders_old/printers.c \
# 		  coders_old/mutex_manager.c \
# 		  coders_old/freedom.c \
# 		  coders_old/thread_manager.c \
# 		  coders_old/watcher.c \
# 		  coders_old/dongle.c \
# 		  coders_old/edf.c \
# 		  coders_old/main_loop.c \
# 		  coders_old/program_starter.c \
# 		  coders_old/getters.c \
# 		  coders_old/fifo.c \
# 		  coders_old/setters.c \
# 		  coders_old/init.c

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
