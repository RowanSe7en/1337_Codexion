/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pair_scheduler.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 00:00:00 by brouane           #+#    #+#             */
/*   Updated: 2026/07/25 00:00:00 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	get_dongle_key(t_code_sim *cs)
{
	if (cs->sim->is_edf)
		return (compute_deadline(cs->coder, cs->sim));
	return ((long long)cs->coder->coder_id);
}

void	register_pair(t_code_sim *cs, t_dongle *d1, t_dongle *d2, long long key)
{
	if (cs->sim->is_edf)
	{
		edf_register(d1, key, cs->sim);
		edf_register(d2, key, cs->sim);
	}
	else
	{
		fifo_register(d1, (int)key, cs->sim);
		fifo_register(d2, (int)key, cs->sim);
	}
}

void	deregister_pair(t_code_sim *cs, t_dongle *d1, t_dongle *d2,
			long long key)
{
	if (cs->sim->is_edf)
	{
		edf_deregister(d1, key, cs->sim);
		edf_deregister(d2, key, cs->sim);
	}
	else
	{
		fifo_deregister(d1, (int)key, cs->sim);
		fifo_deregister(d2, (int)key, cs->sim);
	}
}

int	turn_ready_pair(t_code_sim *cs, t_dongle *d1, t_dongle *d2, long long key)
{
	if (cs->sim->is_edf)
		return (edf_early(d1, key, cs->sim) && edf_early(d2, key, cs->sim));
	return (fifo_first(d1, (int)key, cs->sim)
		&& fifo_first(d2, (int)key, cs->sim));
}
