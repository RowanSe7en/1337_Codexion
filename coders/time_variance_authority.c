/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_variance_authority.c                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:33:32 by brouane           #+#    #+#             */
/*   Updated: 2026/05/02 00:02:04 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((long long)tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void precise_sleep(long long duration_ms, t_simulation *sim)
{
    long long start;
    long long elapsed;

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

