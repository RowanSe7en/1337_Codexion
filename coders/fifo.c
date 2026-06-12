/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fifo.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 18:42:49 by brouane           #+#    #+#             */
/*   Updated: 2026/06/12 20:47:45 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	fifo_register(t_dongle *d, int coder_id, t_simulation *sim)
{
	int	i;

	lock_mutex(&d->scheduler.order_mtx, sim);
	i = 0;
	while (i < 2)
	{
		if (d->scheduler.order[i] == 0)
		{
			d->scheduler.order[i] = coder_id;
			break ;
		}
		i++;
	}
	unlock_mutex(&d->scheduler.order_mtx, sim);
}

void	fifo_deregister(t_dongle *d, t_simulation *sim)
{
	lock_mutex(&d->scheduler.order_mtx, sim);
	d->scheduler.order[0] = d->scheduler.order[1];
	d->scheduler.order[1] = 0;
	unlock_mutex(&d->scheduler.order_mtx, sim);
}

void	fifo_wait_turn(t_dongle *d, int my_id, t_code_sim *cs)
{
	precise_sleep(5, cs->sim);
	while (!is_finished(cs->sim))
	{
		if (fifo_first(d, my_id, cs->sim))
			return ;
		precise_sleep(1, cs->sim);
	}
}

int	fifo_first(t_dongle *d, int my_id,
				t_simulation *sim)
{
	int			val;

	lock_mutex(&d->scheduler.order_mtx, sim);
	val = d->scheduler.order[0];
	if (val != my_id)
	{
		unlock_mutex(&d->scheduler.order_mtx, sim);
		return (0);
	}
	unlock_mutex(&d->scheduler.order_mtx, sim);
	return (1);
}
