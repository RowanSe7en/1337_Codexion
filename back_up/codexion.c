/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/06 16:33:32 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	program_starter(t_simulation *sim)
{
	int			num_of_coders = sim->args.number_of_coders;
	t_code_sim	*codes_sims = malloc(sizeof(t_code_sim) * num_of_coders);

	sim->codes_sims = codes_sims;
	if (!codes_sims)
		freedom(sim, 1);
	for (int i = 0; i < num_of_coders; i++)
	{
		codes_sims[i].sim = sim;
		codes_sims[i].coder = &sim->coders[i];
		thread_create(&sim->coders[i].coder, main_loop, &codes_sims[i]);
	}
	watcher_thread_create(&sim->watcher_thread, the_watcher, sim);
	lock_mutex(&sim->start_time_mtx, sim);
	sim->start_time = get_time_us();
	unlock_mutex(&sim->start_time_mtx, sim);
	for (int i = 0; i < num_of_coders; i++)
		set_last_compile_time(codes_sims[i].coder,
			get_start_time(codes_sims[i].sim), codes_sims[i].sim);
	lock_mutex(&sim->is_ready_mtx, sim);
	sim->is_all_ready = 1;
	unlock_mutex(&sim->is_ready_mtx, sim);
	for (int i = 0; i < num_of_coders; i++)
		thread_join(&sim->coders[i].coder, sim);
	thread_join(&sim->watcher_thread, sim);
}

int	main(int ac, char **av)
{
	if (ac != 9)
		return bye_bye();
	t_arguments data = parser(ac, av);
	if (data.valid == 0)
		return 1;
	int			size = data.number_of_coders;
	t_coder		*coders = malloc(sizeof(t_coder) * size);
	t_dongle	*dongles = malloc(sizeof(t_dongle) * size);
	t_simulation sim;
	sim.coders = coders;
	sim.dongles = dongles;
	if (!coders || !dongles)
		freedom(&sim, 0);
	sim.args = data;
	sim.is_finished = 0;
	sim.is_all_ready = 0;
	sim.is_edf = 0;
	if (strcmp(sim.args.scheduler, "edf") == 0)
		sim.is_edf = 1;
	initiate_mutex(&sim.log_mtx, &sim);
	initiate_mutex(&sim.start_time_mtx, &sim);
	initiate_mutex(&sim.is_ready_mtx, &sim);
	initiate_mutex(&sim.is_finished_mtx, &sim);
	for (int i = 0; i < size; i++)
	{
		dongles[i].dongle_id = i + 1;
		dongles[i].last_used_time = 0;
		dongles[i].scheduler.counter = 0;
		dongles[i].coders_passed = 0;
		initiate_mutex(&dongles[i].reset_mtx, &sim);
		initiate_mutex(&dongles[i].dongle_mtx, &sim);
		initiate_mutex(&dongles[i].passed_mtx, &sim);
		initiate_mutex(&dongles[i].used_time_mtx, &sim);
		initiate_mutex(&dongles[i].scheduler.counter_mtx, &sim);
	}
	for (int i = 0; i < size; i++)
	{
		coders[i].coder_id = i + 1;
		coders[i].compile_count = 0;
		coders[i].sim = &sim;
		initiate_mutex(&coders[i].state_mtx, &sim);
		if (coders[i].coder_id % 2 == 0)
		{
			coders[i].first_dongle = &dongles[i];
			coders[i].second_dongle = &dongles[(i + 1) % size];
		}
		else
		{
			coders[i].first_dongle = &dongles[(i + 1) % size];
			coders[i].second_dongle = &dongles[i];
		}
	}
	program_starter(&sim);
	freedom(&sim, 1);
	return 0;
}
