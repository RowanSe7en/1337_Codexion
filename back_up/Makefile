# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/01/08 21:41:29 by brouane           #+#    #+#              #
#    Updated: 2026/06/07 17:18:17 by brouane          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CC      = cc
CFLAGS  = -Wall -Wextra  #-Werror
RM      = rm -f

NAME    = codexion

SRCS    = codexion.c \
          program.c \
          dongle_stuff.c \
          dr_strange.c \
          groot.c \
          guardians_of_the_galaxy.c \
          time_variance_authority.c \
          he_who_remains.c \
          ultron_vision.c \
          freedom.c \
          kang.c \
          coder.c \
          watcher.c \
          getters.c \
          getters2.c \
          setters.c \
          sync.c \
          edf.c \
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
