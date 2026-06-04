/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   freedom.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:33:53 by brouane           #+#    #+#             */
/*   Updated: 2026/06/04 19:44:50 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void freedom(t_simulation *sim)// good
{
    if (sim->coders)
        free(sim->coders);
    
    if (sim->dongles)
        free(sim->dongles);
    
    if (sim->codes_sims)
        free(sim->codes_sims);

    exit(0);
}