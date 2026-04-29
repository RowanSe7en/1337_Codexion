/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   he_who_remains.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:33:44 by brouane           #+#    #+#             */
/*   Updated: 2026/04/29 12:33:45 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int bye_bye()
{
    printf("Pass exactly these number of arguments, with the EXACTE ");
    printf("order, and DO NOT miss any:\n");
    printf("number_of_coders time_to_burnout time_to_compile ");
    printf("time_to_debug time_to_refactor number_of_compiles_required");
    printf(" dongle_cooldown scheduler\n");

    return (1);
}

void log_action(t_code_sim *code_sim, char *action)
{
    if (!code_sim->sim->is_finished)
    {
        unsigned long timestamp = get_time_ms() - code_sim->sim->start_time;
        // printf("get_time_ms() %ld - code_sim->coder->sim->start_time %ld\n", get_time_ms(), code_sim->sim->start_time);
        printf("%ld %d %s\n", timestamp, code_sim->coder->coder_id, action);
    }
}