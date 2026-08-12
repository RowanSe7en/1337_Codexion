/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/08/12 13:13:17 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	dongle_is_ready(t_dongle *d, long long cooldown, t_simulation *sim)
{
	long long	now;
	long long	elapsed;

	now = get_time_us();
	elapsed = now - get_last_used_time(d, sim);
	return (elapsed >= cooldown);
}

int	try_grab_pair(t_code_sim *cs, t_dongle *d1, t_dongle *d2)
{
	t_simulation	*sim;
	long long		cooldown;

	sim = cs->sim;
	cooldown = ms_to_us(sim->args.dongle_cooldown);
	if (!dongle_is_ready(d1, cooldown, sim)
		|| !dongle_is_ready(d2, cooldown, sim))
		return (0);
	if (pthread_mutex_trylock(&d1->dongle_mtx) != 0)
		return (0);
	if (!dongle_is_ready(d1, cooldown, sim)
		|| !dongle_is_ready(d2, cooldown, sim))
	{
		unlock_mutex(&d1->dongle_mtx, sim);
		return (0);
	}
	if (pthread_mutex_trylock(&d2->dongle_mtx) != 0)
	{
		unlock_mutex(&d1->dongle_mtx, sim);
		return (0);
	}
	return (1);
}

int	take_dongle_pair(t_code_sim *cs)
{
	t_dongle	*d1;
	t_dongle	*d2;
	long long	key;

	d1 = cs->coder->first_dongle;
	d2 = cs->coder->second_dongle;
	key = get_dongle_key(cs);
	register_pair(cs, d1, d2, key);
	while (!is_finished(cs->sim))
	{
		if (turn_ready_pair(cs, d1, d2, key) && try_grab_pair(cs, d1, d2))
		{
			deregister_pair(cs, d1, d2, key);
			log_action(cs->sim, cs->coder, "has taken a dongle", 0);
			log_action(cs->sim, cs->coder, "has taken a dongle", 0);
			return (1);
		}
		usleep(500);
	}
	deregister_pair(cs, d1, d2, key);
	return (0);
}
