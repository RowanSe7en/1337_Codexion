/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_variance_authority.c                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:33:32 by brouane           #+#    #+#             */
/*   Updated: 2026/04/29 12:33:33 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

unsigned long get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((unsigned long)tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void precise_sleep(unsigned long duration_ms, t_simulation *sim)
{
    unsigned long start;
    unsigned long elapsed;

    start = get_time_ms();
    while (1)
    {
        if (is_finished(sim))
            break;

        elapsed = get_time_ms() - start;
        if (elapsed >= duration_ms)
            break;

        usleep(200);
    }
}

