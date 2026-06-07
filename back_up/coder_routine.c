/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/07 16:59:04 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	debug(t_code_sim *code_sim)
{
	log_action(code_sim->sim, code_sim->coder, "is debugging");
	precise_sleep(code_sim->sim->args.time_to_debug, code_sim->sim);
}

void	refactor(t_code_sim *code_sim)
{
	log_action(code_sim->sim, code_sim->coder, "is refactoring");
	precise_sleep(code_sim->sim->args.time_to_refactor, code_sim->sim);
	if (!is_finished(code_sim->sim))
		set_compile_count(code_sim->coder, code_sim->sim);
}

void	*main_loop(void *arg)
{
	t_code_sim	*code_sim;
	long long	required;
	long long	compile_count;

	code_sim = (t_code_sim *)arg;
	sync_threads(code_sim->sim);
	required = code_sim->sim->args.number_of_compiles_required;
	while (!is_finished(code_sim->sim))
	{
		compile_count = get_compile_count(code_sim->coder, code_sim->sim);
		if (compile_count == required)
			break ;
		compile(code_sim);
		debug(code_sim);
		refactor(code_sim);
	}
	return (NULL);
}
