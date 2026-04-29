/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ultron.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:33:28 by brouane           #+#    #+#             */
/*   Updated: 2026/04/29 12:33:29 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void destroy_them_all(t_simulation *sim)
{
    pthread_mutex_destroy(&sim->log_mtx);
    pthread_mutex_destroy(&sim->is_ready_mtx);
    pthread_mutex_destroy(&sim->is_finished_mtx);

    for (int i = 0; i < sim->args.number_of_coders; i++)
    {
        pthread_mutex_destroy(&sim->dongles[i].mtx);
        pthread_mutex_destroy(&sim->coders[i].state_mtx);
    }
}