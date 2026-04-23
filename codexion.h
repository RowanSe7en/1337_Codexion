/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:35 by brouane           #+#    #+#             */
/*   Updated: 2026/04/22 17:23:31 by brouane          ###   ########.fr       */
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
    size_t valid;
    size_t number_of_coders;
    size_t time_to_burnout;
    size_t time_to_compile;
    size_t time_to_debug;
    size_t time_to_refactor;
    size_t number_of_compiles_required;
    size_t dongle_cooldown;
    char *scheduler;

} t_arguments;

typedef struct s_dongle
{
    size_t dongle_id;
    size_t is_available;
    pthread_mutex_t mtx;

} t_dongle;

typedef struct s_coder
{
    pthread_t coder;
    size_t coder_id;
    size_t compile_count;
    t_dongle *first_dongle;
    t_dongle *second_dongle;
    t_simulation *sim;
    long last_compile_time;
    pthread_mutex_t state_mtx;

} t_coder;

typedef struct s_simulation
{
    t_arguments args;
    t_coder     *coders;
    t_dongle    *dongles;

    long        start_time;
    size_t         is_finished;

    pthread_mutex_t log_mtx;
    pthread_mutex_t is_finished_mtx;
    
} t_simulation;

t_arguments parser(int ac, char **av);
int	ft_atoi(const char *nptr);
size_t	ft_strlen(const char *s);
int	ft_isdigit(char d);
int	ft_issign(char s);
int	dig_sign_checker(char *str);
long get_time_ms(void);
void log_action(t_coder *c, char *action);

#endif