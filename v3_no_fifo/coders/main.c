/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/11 21:38:09 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	refactor(t_code_sim *code_sim)
{
	log_action(code_sim->sim, code_sim->coder, "is refactoring");
	precise_sleep(code_sim->sim->args.time_to_refactor, code_sim->sim);
	if (!is_finished(code_sim->sim))
		set_compile_count(code_sim->coder, code_sim->sim);
}

void	*main_loop(void *arg)
{
	t_code_sim	*code_sim;
	long long	required;
	long long	compile_count;

	code_sim = (t_code_sim *)arg;
	sync_threads(code_sim->sim);
	required = code_sim->sim->args.number_of_compiles_required;
	while (!is_finished(code_sim->sim))
	{
		compile_count = get_compile_count(code_sim->coder, code_sim->sim);
		if (compile_count == required)
			break ;
		compile(code_sim);
		if (code_sim->coder->first_dongle == code_sim->coder->second_dongle)
			break ;
		debug(code_sim);
		refactor(code_sim);
	}
	return (NULL);
}

short	check_if_coder_burned_out(t_simulation *sim)
{
	int			i;
	long long	last_compile_time;
	long long	now;

	i = 0;
	while (i < sim->args.number_of_coders)
	{
		if (get_compile_count(&sim->coders[i], sim)
			>= sim->args.number_of_compiles_required)
		{
			i++;
			continue ;
		}
		last_compile_time = get_last_compile_time(&sim->coders[i], sim);
		now = get_time_us();
		if (now - last_compile_time >= ms_to_us(sim->args.time_to_burnout))
		{
			log_action(sim, &sim->coders[i], "burned out");
			return (1);
		}
		i++;
	}
	return (0);
}

void	check_if_all_compiles_done(t_simulation *sim)
{
	int			i;
	long long	compile_count;

	i = 0;
	while (i < sim->args.number_of_coders)
	{
		compile_count = get_compile_count(&sim->coders[i], sim);
		if (compile_count != sim->args.number_of_compiles_required)
			return ;
		i++;
	}
	set_finished(sim);
}

void	*the_watcher(void *arg)
{
	t_simulation	*sim;

	sim = (t_simulation *)arg;
	sync_threads(sim);
	while (!is_finished(sim))
	{
		if (check_if_coder_burned_out(sim))
		{
			set_finished(sim);
			return (NULL);
		}
		if (!is_finished(sim))
		{
			check_if_all_compiles_done(sim);
			usleep(100);
		}
	}
	return (NULL);
}
