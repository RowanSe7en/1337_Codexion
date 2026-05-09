/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:35 by brouane           #+#    #+#             */
/*   Updated: 2026/05/09 17:07:38 by brouane          ###   ########.fr       */
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
typedef struct s_code_sim t_code_sim;

typedef struct s_arguments
{
    unsigned short valid;
    int number_of_coders;
    long long time_to_burnout;
    long long time_to_compile;
    long long time_to_debug;
    long long time_to_refactor;
    long long number_of_compiles_required;
    long long dongle_cooldown;
    char *scheduler;

} t_arguments;

typedef struct s_scheduler
{
    int counter;
    int order[2];

} t_scheduler;

typedef struct s_dongle
{
    int dongle_id;
    unsigned short is_available;
    pthread_mutex_t mtx;
    t_scheduler scheduler;

} t_dongle;

typedef struct s_coder
{
    pthread_t coder;
    int coder_id;
    long long compile_count;
    t_dongle *first_dongle;
    t_dongle *second_dongle;
    t_simulation *sim;
    long long last_compile_time;
    pthread_mutex_t state_mtx;
    unsigned short is_regestered;

} t_coder;

typedef struct s_simulation
{
    t_arguments args;
    t_coder     *coders;
    t_dongle    *dongles;

    long long   start_time;
    unsigned short  is_finished;
    unsigned short  is_all_ready;
    t_code_sim *fifo_array;

    pthread_mutex_t log_mtx;
    pthread_mutex_t is_finished_mtx;
    pthread_mutex_t is_ready_mtx;

    pthread_t watcher_thread;
    
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
long long get_time_ms(void);
void log_action(t_code_sim *code_sim, char *action);
void precise_sleep(long long duration_ms, t_simulation *sim);
unsigned short is_finished(t_simulation *sim);
int bye_bye();
void *malloc_for_me(long long bytes);
int freedom(t_coder *coders, t_dongle *dongles);
void destroy_them_all(t_simulation *sim);
void initiate_mutex(pthread_mutex_t *mutex, t_simulation *sim);
void lock_mutex(pthread_mutex_t *mutex, t_simulation *sim);
void unlock_mutex(pthread_mutex_t *mutex, t_simulation *sim);
void thread_create(pthread_t *coder, void *func, t_code_sim *code_sim);
void thread_join(pthread_t *coder, t_simulation *sim);

#endif