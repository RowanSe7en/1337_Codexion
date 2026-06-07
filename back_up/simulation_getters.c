/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_getters.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/07 22:28:32 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	get_coders_passed(t_dongle *dongle, t_simulation *sim)
{
	int	answer;

	lock_mutex(&dongle->passed_mtx, sim);
	answer = dongle->coders_passed;
	unlock_mutex(&dongle->passed_mtx, sim);
	return (answer);
}

short	get_ready(t_simulation *sim)
{
	short	answer;

	lock_mutex(&sim->is_ready_mtx, sim);
	answer = sim->is_all_ready;
	unlock_mutex(&sim->is_ready_mtx, sim);
	return (answer);
}
