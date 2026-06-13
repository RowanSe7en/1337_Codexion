/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   printers.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:33:44 by brouane           #+#    #+#             */
/*   Updated: 2026/06/13 16:23:24 by brouane          ###   ########.fr       */
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

void	log_action(t_simulation *sim, t_coder *coder, char *action)
{
	long long	timestamp;

	if (!is_finished(sim))
	{
		lock_mutex(&sim->log_mtx, sim);
		timestamp = get_time_ms() - us_to_ms(sim->start_time);
		printf("%lld %d %s\n", timestamp, coder->coder_id, action);
		unlock_mutex(&sim->log_mtx, sim);
	}
}
