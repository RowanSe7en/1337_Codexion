/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_entry.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/07 16:13:16 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	init_sim_mutexes(t_simulation *sim)
{
	initiate_mutex(&sim->log_mtx, sim);
	initiate_mutex(&sim->start_time_mtx, sim);
	initiate_mutex(&sim->is_ready_mtx, sim);
	initiate_mutex(&sim->is_finished_mtx, sim);
}

static void	setup_sim(t_simulation *sim, t_arguments data, int size)
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

int	main(int ac, char **av)
{
	t_arguments		data;
	t_simulation	sim;
	int				size;

	if (ac != 9)
		return (bye_bye());
	data = parser(ac, av);
	if (data.valid == 0)
		return (1);
	size = data.number_of_coders;
	sim.coders = malloc(sizeof(t_coder) * size);
	sim.dongles = malloc(sizeof(t_dongle) * size);
	sim.codes_sims = NULL;
	if (!sim.coders || !sim.dongles)
		freedom(&sim, 0);
	setup_sim(&sim, data, size);
	program_starter(&sim);
	freedom(&sim, 1);
	return (0);
}
