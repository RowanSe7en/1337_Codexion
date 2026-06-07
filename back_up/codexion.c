/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/07 22:27:51 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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
			sim->coders[i].second_dongle
				= &sim->dongles[(i + 1) % size];
		}
		else
		{
			sim->coders[i].first_dongle
				= &sim->dongles[(i + 1) % size];
			sim->coders[i].second_dongle = &sim->dongles[i];
		}
		i++;
	}
}

void	init_simulation(t_simulation *sim, t_arguments data)
{
	sim->args = data;
	sim->is_finished = 0;
	sim->is_all_ready = 0;
	sim->is_edf = 0;
	if (strcmp(sim->args.scheduler, "edf") == 0)
		sim->is_edf = 1;
	initiate_mutex(&sim->log_mtx, sim);
	initiate_mutex(&sim->start_time_mtx, sim);
	initiate_mutex(&sim->is_ready_mtx, sim);
	initiate_mutex(&sim->is_finished_mtx, sim);
}

int	main(int ac, char **av)
{
	t_arguments		data;
	int				size;
	t_coder			*coders;
	t_dongle		*dongles;
	t_simulation	sim;

	if (ac != 9)
		return (bye_bye());
	data = parser(ac, av);
	if (data.valid == 0)
		return (1);
	size = data.number_of_coders;
	coders = malloc(sizeof(t_coder) * size);
	dongles = malloc(sizeof(t_dongle) * size);
	sim.coders = coders;
	sim.dongles = dongles;
	if (!coders || !dongles)
		freedom(&sim, 0);
	init_simulation(&sim, data);
	init_dongles(&sim, size);
	init_coders(&sim, size);
	program_starter(&sim);
	freedom(&sim, 1);
	return (0);
}
