/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/09 15:56:04 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	unlock_dongles(t_code_sim *cs, int same_dongle)
{
	unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim);
	if (!same_dongle)
		unlock_mutex(&cs->coder->second_dongle->dongle_mtx, cs->sim);
}

void	finish_compile(t_code_sim *cs, long long now)
{
	set_last_used_time(cs->coder->first_dongle, now, cs->sim);
	set_last_used_time(cs->coder->second_dongle, now, cs->sim);
}

void	compile(t_code_sim *cs)
{
	int			same_dongle;
	long long	now;

	same_dongle = (cs->coder->first_dongle
			== cs->coder->second_dongle);
	set_coders_passed(cs->coder->first_dongle, cs->sim);
	if (!take_dongle(cs, cs->coder->first_dongle, 0, 0))
		return ;
	set_coders_passed(cs->coder->second_dongle, cs->sim);
	if (!take_dongle(cs, cs->coder->second_dongle, same_dongle, 1))
	{
		unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim);
		return ;
	}
	log_action(cs->sim, cs->coder, "is compiling");
	now = get_time_us();
	set_last_compile_time(cs->coder, now, cs->sim);
	precise_sleep(cs->sim->args.time_to_compile, cs->sim);
	if (is_finished(cs->sim))
	{
		unlock_dongles(cs, same_dongle);
		return ;
	}
	finish_compile(cs, now);
	unlock_dongles(cs, same_dongle);
}
