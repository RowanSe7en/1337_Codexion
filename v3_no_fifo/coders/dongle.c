/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/11 19:49:20 by brouane          ###   ########.fr       */
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
