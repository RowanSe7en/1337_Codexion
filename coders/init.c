/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 19:06:17 by brouane           #+#    #+#             */
/*   Updated: 2026/06/12 20:39:57 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	init_sim_mutexes(t_simulation *sim)
{
	initiate_mutex(&sim->log_mtx, sim);
	initiate_mutex(&sim->start_time_mtx, sim);
	initiate_mutex(&sim->is_ready_mtx, sim);
	initiate_mutex(&sim->is_finished_mtx, sim);
}

void	init_dongles(t_simulation *sim, int size)
{
	int	i;

	i = 0;
	while (i < size)
	{
		sim->dongles[i].dongle_id = i + 1;
		sim->dongles[i].last_used_time = 0;
		initiate_mutex(&sim->dongles[i].dongle_mtx, sim);
		initiate_mutex(&sim->dongles[i].used_time_mtx, sim);
		initiate_mutex(&sim->dongles[i].scheduler.order_mtx, sim);
		sim->dongles[i].scheduler.order[0] = 0;
		sim->dongles[i].scheduler.order[1] = 0;
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

void	setup_sim(t_simulation *sim, t_arguments data, int size)
{
	sim->args = data;
	sim->is_finished = 0;
	sim->is_all_ready = 0;
	sim->is_edf = 0;
	if (strcmp(sim->args.scheduler, "edf") == 0)
		sim->is_edf = 1;
	init_sim_mutexes(sim);
	init_dongles(sim, size);
	init_coders(sim, size);
}
