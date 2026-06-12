/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_loop.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/12 20:30:58 by brouane          ###   ########.fr       */
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

void	debug(t_code_sim *code_sim)
{
	log_action(code_sim->sim, code_sim->coder, "is debugging");
	precise_sleep(code_sim->sim->args.time_to_debug, code_sim->sim);
}

void	compile(t_code_sim *cs)
{
	long long	now;

	take_dongle(cs, cs->coder->first_dongle);
	if (is_finished(cs->sim))
		return ;
	if (cs->coder->first_dongle != cs->coder->second_dongle)
	{
		take_dongle(cs, cs->coder->second_dongle);
		if (is_finished(cs->sim))
		{
			unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim);
			return ;
		}
		log_action(cs->sim, cs->coder, "is compiling");
		now = get_time_us();
		set_last_compile_time(cs->coder, now, cs->sim);
		precise_sleep(cs->sim->args.time_to_compile, cs->sim);
		now = get_time_us();
		set_last_used_time(cs->coder->first_dongle, now, cs->sim);
		set_last_used_time(cs->coder->second_dongle, now, cs->sim);
		unlock_mutex(&cs->coder->second_dongle->dongle_mtx, cs->sim);
	}
	unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim);
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
