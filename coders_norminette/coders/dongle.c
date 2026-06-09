/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/07 16:13:16 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

short	get_ready(t_simulation *sim)
{
	short	answer;

	lock_mutex(&sim->is_ready_mtx, sim);
	answer = sim->is_all_ready;
	unlock_mutex(&sim->is_ready_mtx, sim);
	return (answer);
}

void	sync_threads(t_simulation *sim)
{
	while (!get_ready(sim))
		usleep(1000);
}

int	get_counter(t_dongle *dongle, t_simulation *sim)
{
	int	answer;

	lock_mutex(&dongle->scheduler.counter_mtx, sim);
	answer = dongle->scheduler.counter;
	unlock_mutex(&dongle->scheduler.counter_mtx, sim);
	return (answer);
}

int	get_coders_passed(t_dongle *dongle, t_simulation *sim)
{
	int	answer;

	lock_mutex(&dongle->passed_mtx, sim);
	answer = dongle->coders_passed;
	unlock_mutex(&dongle->passed_mtx, sim);
	return (answer);
}

void	reset_passed(t_dongle *dongle, t_simulation *sim)
{
	lock_mutex(&dongle->passed_mtx, sim);
	dongle->coders_passed -= 1;
	unlock_mutex(&dongle->passed_mtx, sim);
}
