/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:52 by brouane           #+#    #+#             */
/*   Updated: 2026/04/19 21:44:53 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int main(int ac, char **av)
{
    if (ac != 9)
        return 1;

    t_arguments data = parser(ac, av);
    
    if (data.valid == 0)
        return 1;

    printf("All arguments are valid.\n");
    printf("Number of coders: %d\n", data.number_of_coders);
    printf("Time to burnout: %d\n", data.time_to_burnout);
    printf("Time to compile: %d\n", data.time_to_compile);
    printf("Time to debug: %d\n", data.time_to_debug);
    printf("Time to refactor: %d\n", data.time_to_refactor);
    printf("Number of compiles required: %d\n", data.number_of_compiles_required);
    printf("Dongle cooldown: %d\n", data.dongle_cooldown);
    printf("Scheduler: %s\n", data.scheduler);



    return 0;
}