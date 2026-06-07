/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   watcher.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/07 15:41:43 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

short	check_if_coder_burned_out(t_simulation *sim)
{
	for (int i = 0; i < sim->args.number_of_coders; i++)
	{
		if (get_compile_count(&sim->coders[i], sim)
			>= sim->args.number_of_compiles_required)
			continue ;
		long long last_compile_time = get_last_compile_time(&sim->coders[i],
				sim);
		long long now = get_time_us();
		if (now - last_compile_time >= ms_to_us(sim->args.time_to_burnout))
		{
			log_action(sim, &sim->coders[i], "burned out");
			return 1;
		}
	}
	return 0;
}

void	check_if_all_compiles_done(t_simulation *sim)
{
	for (int i = 0; i < sim->args.number_of_coders; i++)
	{
		long long compile_count = get_compile_count(&sim->coders[i], sim);
		if (compile_count != sim->args.number_of_compiles_required)
			return ;
	}
	set_finished(sim);
}

void	*the_watcher(void *arg)
{
	t_simulation	*sim = (t_simulation *)arg;

	sync_threads(sim);
	while (!is_finished(sim))
	{
		if (check_if_coder_burned_out(sim))
		{
			set_finished(sim);
			return NULL;
		}
		if (!is_finished(sim))
		{
			check_if_all_compiles_done(sim);
			usleep(100);
		}
	}
	return NULL;
}
