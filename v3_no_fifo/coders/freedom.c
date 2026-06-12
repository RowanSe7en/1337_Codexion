/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   freedom.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:33:53 by brouane           #+#    #+#             */
/*   Updated: 2026/06/11 19:49:20 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	freedom(t_simulation *sim, short is_destroy)
{
	if (is_destroy)
		destroy_them_all(sim);
	if (sim->coders)
		free(sim->coders);
	if (sim->dongles)
		free(sim->dongles);
	if (sim->codes_sims)
		free(sim->codes_sims);
	exit(0);
}

void	debug(t_code_sim *code_sim)
{
	log_action(code_sim->sim, code_sim->coder, "is debugging");
	precise_sleep(code_sim->sim->args.time_to_debug, code_sim->sim);
}
