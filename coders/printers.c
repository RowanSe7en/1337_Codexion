/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   printers.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:33:44 by brouane           #+#    #+#             */
/*   Updated: 2026/08/10 21:33:22 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	bye_bye(void)
{
	printf("Pass exactly these number of arguments, with the EXACTE ");
	printf("order, and DO NOT miss any:\n");
	printf("number_of_coders time_to_burnout time_to_compile ");
	printf("time_to_debug time_to_refactor number_of_compiles_required");
	printf(" dongle_cooldown scheduler\n");
	return (1);
}

void	log_action(
	t_simulation *sim, t_coder *coder, char *action, short is_done
	)
{
	long long	timestamp;

	lock_mutex(&sim->log_mtx, sim);
	if (!is_finished(sim) && !sim->is_done)
	{
		timestamp = get_time_ms() - us_to_ms(get_start_time(sim));
		printf("%lld %d %s\n", timestamp, coder->coder_id, action);
		if (is_done)
			sim->is_done = 1;
	}
	unlock_mutex(&sim->log_mtx, sim);
}
