/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:35 by brouane           #+#    #+#             */
/*   Updated: 2026/04/29 11:33:23 by brouane          ###   ########.fr       */
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
    unsigned short valid;
    int number_of_coders;
    unsigned long long time_to_burnout;
    unsigned long long time_to_compile;
    unsigned long long time_to_debug;
    unsigned long long time_to_refactor;
    unsigned long long number_of_compiles_required;
    unsigned long long dongle_cooldown;
    char *scheduler;

} t_arguments;

typedef struct s_dongle
{
    int dongle_id;
    unsigned short is_available;
    pthread_mutex_t mtx;

} t_dongle;

typedef struct s_coder
{
    pthread_t coder;
    int coder_id;
    unsigned long long compile_count;
    t_dongle *first_dongle;
    t_dongle *second_dongle;
    t_simulation *sim;
    unsigned long long last_compile_time;
    pthread_mutex_t state_mtx;

} t_coder;

typedef struct s_simulation
{
    t_arguments args;
    t_coder     *coders;
    t_dongle    *dongles;

    unsigned long long   start_time;
    unsigned short  is_finished;
    unsigned short  is_all_ready;

    pthread_mutex_t log_mtx;
    pthread_mutex_t is_finished_mtx;
    pthread_mutex_t is_ready_mtx;

    // pthread_cond_t ready_cond;

    
} t_simulation;

typedef struct s_code_sim
{
    t_coder     *coder;
    t_simulation *sim;

} t_code_sim;

t_arguments parser(int ac, char **av);
int	ft_atoi(const char *nptr);
size_t	ft_strlen(const char *s);
unsigned short ft_isdigit(char d);
unsigned short ft_issign(char s);
unsigned short dig_sign_checker(char *str);
unsigned long long get_time_ms(void);
void log_action(t_code_sim *code_sim, char *action);
void precise_sleep(unsigned long long duration_ms, t_simulation *sim);
unsigned short is_finished(t_simulation *sim);
int bye_bye();

#endif