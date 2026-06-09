/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_start.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/07 17:09:04 by brouane           #+#    #+#             */
/*   Updated: 2026/06/09 16:36:36 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	create_coder_threads(t_simulation *sim,
				t_code_sim *codes_sims, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		codes_sims[i].sim = sim;
		codes_sims[i].coder = &sim->coders[i];
		thread_create(&sim->coders[i].coder,
			main_loop, &codes_sims[i]);
		i++;
	}
}

void	set_initial_compile_times(t_code_sim *codes_sims, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		set_last_compile_time(codes_sims[i].coder,
			get_start_time(codes_sims[i].sim),
			codes_sims[i].sim);
		i++;
	}
}

void	join_coder_threads(t_simulation *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		thread_join(&sim->coders[i].coder, sim);
		i++;
	}
}

void	program_starter(t_simulation *sim)
{
	int			count;
	t_code_sim	*codes_sims;

	count = sim->args.number_of_coders;
	codes_sims = malloc(sizeof(t_code_sim) * count);
	sim->codes_sims = codes_sims;
	if (!codes_sims)
		freedom(sim, 1);
	create_coder_threads(sim, codes_sims, count);
	watcher_thread_create(&sim->watcher_thread, the_watcher, sim);
	lock_mutex(&sim->start_time_mtx, sim);
	sim->start_time = get_time_us();
	unlock_mutex(&sim->start_time_mtx, sim);
	set_initial_compile_times(codes_sims, count);
	lock_mutex(&sim->is_ready_mtx, sim);
	sim->is_all_ready = 1;
	unlock_mutex(&sim->is_ready_mtx, sim);
	join_coder_threads(sim, count);
	thread_join(&sim->watcher_thread, sim);
}
