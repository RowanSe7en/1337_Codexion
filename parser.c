/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 21:44:33 by brouane           #+#    #+#             */
/*   Updated: 2026/04/26 23:16:38 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int str_parser(char *str)
{
    int i = 0;

    while (str[i])
    {
        if (!(str[i] > 'z' || str[i] < 'a') && !(str[i] > 'Z' || str[i] < 'A'))
            return 1;
        i++;
    }
    if (strcmp(str, "fifo") != 0 && strcmp(str, "edf") != 0)
    {
        printf("No scheduler allowed other than fifo or edf\n");
        return 1;
    }
    
    return 0;
}

int int_parser(int ac, char **av)
{
    int i = 0;

    while (ac - 1 > ++i)
    {
        if (dig_sign_checker(av[i]))
        {
            printf("Invalid integer argument: %s\n", av[i]);
            return 1;
        }    
    }
    return 0;
}

t_arguments parser(int ac, char **av)
{
    t_arguments arguments;
    arguments.valid = 0;

    int int_result = int_parser(ac, av);
    int str_result = str_parser(av[8]);
    if (int_result || str_result)
        return arguments;
    
    arguments.number_of_coders = ft_atoi(av[1]);
    if (arguments.number_of_coders <= 0)
    {
        printf("Number of coders should be more then ZERO\n");
        return arguments;
    }
    arguments.time_to_burnout = ft_atoi(av[2]);
    arguments.time_to_compile = ft_atoi(av[3]);
    arguments.time_to_debug = ft_atoi(av[4]);
    arguments.time_to_refactor = ft_atoi(av[5]);
    arguments.number_of_compiles_required = ft_atoi(av[6]);
    arguments.dongle_cooldown = ft_atoi(av[7]);
    int i = 2;
    while (i < ac - 1)
    {
        if (ft_atoi(av[i]) < 0)
        {
			printf("Integer overflow or Negative number: %s\n", av[i]);
            return arguments;
        }
        i++;
    }
    arguments.scheduler = av[8];
    arguments.valid = 1;

    return arguments;
}
