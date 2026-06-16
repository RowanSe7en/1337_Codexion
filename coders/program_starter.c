/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   program_starter.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/16 18:08:18 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

short	is_finished(t_simulation *sim)
{
	short	answer;

	lock_mutex(&sim->is_finished_mtx, sim);
	answer = sim->is_finished;
	unlock_mutex(&sim->is_finished_mtx, sim);
	return (answer);
}

void	program_starter(t_simulation *sim)
{
	int			num_of_coders;
	int			i;
	t_code_sim	*codes_sims;

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
