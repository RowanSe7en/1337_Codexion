/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/06/12 19:08:12 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int ac, char **av)
{
	t_arguments		data;
	t_simulation	sim;
	int				size;

	if (ac != 9)
		return (bye_bye());
	data = parser(ac, av);
	if (data.valid == 0)
		return (1);
	size = data.number_of_coders;
	sim.coders = malloc(sizeof(t_coder) * size);
	sim.dongles = malloc(sizeof(t_dongle) * size);
	sim.codes_sims = NULL;
	if (!sim.coders || !sim.dongles)
		freedom(&sim, 0);
	setup_sim(&sim, data, size);
	program_starter(&sim);
	freedom(&sim, 1);
	return (0);
}
