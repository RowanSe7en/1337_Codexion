/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_helper.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 23:00:08 by brouane           #+#    #+#             */
/*   Updated: 2026/04/19 23:52:54 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

size_t	ft_strlen(const char *s)
{
	size_t	len;

	len = 0;
	while (s[len])
		len++;
	return (len);
}

int	ft_isdigit(char d)
{
	if (d >= 48 && 57 >= d)
		return (1);
	return (0);
}

int	ft_issign(char s)
{
	if (s == 43 || s == 45)
		return (1);
	return (0);
}

int	dig_sign_checker(char *str)
{
	int	i;

	i = 0;
    if (!str[i])
    {
        return (1);
    }
	while (str[i])
	{
        while (str[i] == 32)
            i++;
        if (!(ft_isdigit(str[i]) || ft_issign(str[i])))
			return (1);
		if (ft_issign(str[i]) && ((i != 0 && str[i - 1] != 32)
				|| (i == 0 && ft_strlen(str) == 1)))
			return (1);
		i++;
	}
	return (0);
}
