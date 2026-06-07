/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   he_who_remains.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:33:44 by brouane           #+#    #+#             */
/*   Updated: 2026/06/07 16:46:53 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int bye_bye()// good
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
	lock_mutex(&sim->log_mtx, sim);
	if (!is_finished(sim))  // ← move the check INSIDE the log_mtx lock
	{
		long long timestamp = get_time_ms() - us_to_ms(get_start_time(sim));
		printf("%lld %d %s\n", timestamp, coder->coder_id, action);
	}
	unlock_mutex(&sim->log_mtx, sim);
}
