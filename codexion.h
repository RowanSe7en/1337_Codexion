/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:35 by brouane           #+#    #+#             */
/*   Updated: 2026/04/20 23:27:02 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
#define CODEXION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>
#include <unistd.h>
#include <sys/time.h>

typedef struct s_simulation t_simulation;

typedef struct s_arguments
{
    int valid;
    int number_of_coders;
    int time_to_burnout;
    int time_to_compile;
    int time_to_debug;
    int time_to_refactor;
    int number_of_compiles_required;
    int dongle_cooldown;
    char *scheduler;

} t_arguments;

typedef struct s_dongle
{
    int dongle_id;
    int is_available;
    pthread_mutex_t mtx;

} t_dongle;

typedef struct s_coder
{
    pthread_t coder;
    int coder_id;
    int compile_count;
    t_dongle *first_dongle;
    t_dongle *second_dongle;
    t_simulation *sim;
    long last_compile_time;

} t_coder;

typedef struct s_simulation
{
    t_arguments args;
    t_coder     *coders;
    t_dongle    *dongles;

    long        start_time;
    int         stop;

    pthread_mutex_t log_mtx;
} t_simulation;

t_arguments parser(int ac, char **av);
int	ft_atoi(const char *nptr);
size_t	ft_strlen(const char *s);
int	ft_isdigit(char d);
int	ft_issign(char s);
int	dig_sign_checker(char *str);

#endif