/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/07 16:13:16 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	start_threads(t_simulation *sim, int num)
{
	int	i;

	i = 0;
	while (i < num)
	{
		thread_create(&sim->coders[i].coder, main_loop, &sim->codes_sims[i]);
		i++;
	}
	watcher_thread_create(&sim->watcher_thread, the_watcher, sim);
}

void	set_coder_times_and_ready(t_simulation *sim, int num)
{
	int	i;

	i = 0;
	while (i < num)
	{
		set_last_compile_time(sim->codes_sims[i].coder,
			get_start_time(sim->codes_sims[i].sim),
			sim->codes_sims[i].sim);
		i++;
	}
	lock_mutex(&sim->is_ready_mtx, sim);
	sim->is_all_ready = 1;
	unlock_mutex(&sim->is_ready_mtx, sim);
}

void	program_starter(t_simulation *sim)
{
	int			num_of_coders;
	t_code_sim	*codes_sims;
	int			i;

	num_of_coders = sim->args.number_of_coders;
	codes_sims = malloc(sizeof(t_code_sim) * num_of_coders);
	sim->codes_sims = codes_sims;
	if (!codes_sims)
		freedom(sim, 1);
	i = 0;
	while (i < num_of_coders)
	{
		codes_sims[i].sim = sim;
		codes_sims[i].coder = &sim->coders[i];
		i++;
	}
	start_threads(sim, num_of_coders);
	lock_mutex(&sim->start_time_mtx, sim);
	sim->start_time = get_time_us();
	unlock_mutex(&sim->start_time_mtx, sim);
	set_coder_times_and_ready(sim, num_of_coders);
	i = 0;
	while (i < num_of_coders)
		thread_join(&sim->coders[i++].coder, sim);
	thread_join(&sim->watcher_thread, sim);
}

void	init_dongles(t_simulation *sim, int size)
{
	int	i;

	i = 0;
	while (i < size)
	{
		sim->dongles[i].dongle_id = i + 1;
		sim->dongles[i].last_used_time = 0;
		sim->dongles[i].scheduler.counter = 0;
		sim->dongles[i].coders_passed = 0;
		initiate_mutex(&sim->dongles[i].reset_mtx, sim);
		initiate_mutex(&sim->dongles[i].dongle_mtx, sim);
		initiate_mutex(&sim->dongles[i].passed_mtx, sim);
		initiate_mutex(&sim->dongles[i].used_time_mtx, sim);
		initiate_mutex(&sim->dongles[i].scheduler.counter_mtx, sim);
		i++;
	}
}

void	init_coders(t_simulation *sim, int size)
{
	int	i;

	i = 0;
	while (i < size)
	{
		sim->coders[i].coder_id = i + 1;
		sim->coders[i].compile_count = 0;
		sim->coders[i].sim = sim;
		initiate_mutex(&sim->coders[i].state_mtx, sim);
		if (sim->coders[i].coder_id % 2 == 0)
		{
			sim->coders[i].first_dongle = &sim->dongles[i];
			sim->coders[i].second_dongle = &sim->dongles[(i + 1) % size];
		}
		else
		{
			sim->coders[i].first_dongle = &sim->dongles[(i + 1) % size];
			sim->coders[i].second_dongle = &sim->dongles[i];
		}
		i++;
	}
}
