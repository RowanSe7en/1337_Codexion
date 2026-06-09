/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/09 15:36:03 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <unistd.h>
# include <string.h>
# include <sys/time.h>
# include <limits.h>

typedef struct s_arguments
{
	int		number_of_coders;
	int		time_to_burnout;
	int		time_to_compile;
	int		time_to_debug;
	int		time_to_refactor;
	int		number_of_compiles_required;
	int		dongle_cooldown;
	char	*scheduler;
	int		valid;
}	t_arguments;

typedef struct s_scheduler
{
	pthread_mutex_t	counter_mtx;
	long long		order[2];
	short			counter;
}	t_scheduler;

typedef struct s_dongle
{
	int				dongle_id;
	long long		last_used_time;
	int				coders_passed;
	pthread_mutex_t	dongle_mtx;
	pthread_mutex_t	used_time_mtx;
	pthread_mutex_t	passed_mtx;
	pthread_mutex_t	reset_mtx;
	t_scheduler		scheduler;
}	t_dongle;

typedef struct s_simulation	t_simulation;

typedef struct s_coder
{
	int				coder_id;
	long long		compile_count;
	long long		last_compile_time;
	pthread_t		coder;
	pthread_mutex_t	state_mtx;
	t_dongle		*first_dongle;
	t_dongle		*second_dongle;
	t_simulation	*sim;
}	t_coder;

struct s_simulation
{
	t_arguments		args;
	t_coder			*coders;
	t_dongle		*dongles;
	void			*codes_sims;
	pthread_t		watcher_thread;
	pthread_mutex_t	log_mtx;
	pthread_mutex_t	is_ready_mtx;
	pthread_mutex_t	is_finished_mtx;
	pthread_mutex_t	start_time_mtx;
	long long		start_time;
	short			is_finished;
	short			is_all_ready;
	short			is_edf;
};

typedef struct s_code_sim
{
	t_simulation	*sim;
	t_coder			*coder;
}	t_code_sim;

long long	get_last_compile_time(t_coder *coder, t_simulation *sim);
long long	get_compile_count(t_coder *coder, t_simulation *sim);
long long	get_last_used_time(t_dongle *dongle, t_simulation *sim);
long long	get_start_time(t_simulation *sim);
int			get_counter(t_dongle *dongle, t_simulation *sim);

int			get_coders_passed(t_dongle *dongle, t_simulation *sim);
short		get_ready(t_simulation *sim);

void		set_last_compile_time(t_coder *coder, long long now,
				t_simulation *sim);
void		set_compile_count(t_coder *coder, t_simulation *sim);
void		set_last_used_time(t_dongle *dongle, long long time,
				t_simulation *sim);
void		set_coders_passed(t_dongle *dongle, t_simulation *sim);
void		set_finished(t_simulation *sim);

short		is_finished(t_simulation *sim);
void		sync_threads(t_simulation *sim);
void		reset_passed(t_dongle *dongle, t_simulation *sim);
int			dongle_is_ready(t_dongle *d, long long cooldown,
				t_simulation *sim);
void		wait_dongle_ready(t_dongle *d, t_simulation *sim);

long long	compute_deadline(t_coder *coder, t_simulation *sim);
void		edf_register(t_dongle *d, long long deadline,
				t_simulation *sim);
void		edf_wait_turn(t_dongle *d, long long my_deadline,
				t_code_sim *code_sim);
void		edf_reset(t_dongle *d, t_simulation *sim);

void		register_dongle(t_code_sim *cs, t_dongle *d,
				int holding_first);
int			take_dongle(t_code_sim *cs, t_dongle *d,
				int already_held, int holding_first);
void		compile(t_code_sim *cs);

void		debug(t_code_sim *code_sim);
void		refactor(t_code_sim *code_sim);
void		*main_loop(void *arg);

short		check_if_coder_burned_out(t_simulation *sim);
void		check_if_all_compiles_done(t_simulation *sim);
void		*the_watcher(void *arg);

void		program_starter(t_simulation *sim);

long long	get_time_ms(void);
long long	get_time_us(void);
long long	ms_to_us(long long ms);
long long	us_to_ms(long long us);
void		precise_sleep(long long duration_ms, t_simulation *sim);

void		lock_mutex(pthread_mutex_t *mutex, t_simulation *sim);
void		unlock_mutex(pthread_mutex_t *mutex, t_simulation *sim);
void		initiate_mutex(pthread_mutex_t *mutex, t_simulation *sim);
void		destroy_them_all(t_simulation *sim);

void		thread_create(pthread_t *coder, void *func,
				t_code_sim *code_sim);
void		watcher_thread_create(pthread_t *watcher_thread,
				void *func, t_simulation *sim);
void		thread_join(pthread_t *thread, t_simulation *sim);

void		log_action(t_simulation *sim, t_coder *coder,
				char *action);
void		freedom(t_simulation *sim, short is_destroy);

int			bye_bye(void);
t_arguments	parser(int ac, char **av);
short		dig_sign_checker(char *str);
int			ft_atoi(const char *nptr);

#endif
