# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/01/08 21:41:29 by brouane           #+#    #+#              #
#    Updated: 2026/06/07 22:29:44 by brouane          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CC      = cc
CFLAGS  = -Wall -Wextra  #-Werror
RM      = rm -f

NAME    = codexion

SRCS	= codexion.c \
		  simulation_start.c \
		  dongle_utils.c \
		  ft_atoi.c \
		  parsing_utils.c \
		  parser.c \
		  time_utils.c \
		  logging.c \
		  mutex_utils.c \
		  freedom.c \
		  thread_utils.c \
		  coder_routine.c \
		  watcher.c \
		  coder_getters.c \
		  simulation_getters.c \
		  state_setters.c \
		  sync_utils.c \
		  edf_scheduler.c \
		  dongle.c

OBJS    = $(SRCS:.c=.o)

all: $(NAME)

it: all
	clear
	./codexion 6 2000 200 200 200 3 400 edf

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
