/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_helper.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 23:00:08 by brouane           #+#    #+#             */
/*   Updated: 2026/06/12 19:17:47 by brouane          ###   ########.fr       */
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

short	ft_isdigit(char d)
{
	if (d >= 48 && 57 >= d)
		return (1);
	return (0);
}

short	ft_issign(char s)
{
	if (s == 43 || s == 45)
		return (1);
	return (0);
}

short	dig_sign_checker(char *str)
{
	int	i;

	i = 0;
	if (!str[i])
		return (1);
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

int	ft_atoi(const char *nptr)
{
	size_t	i;
	long	result;
	int		sign;

	i = 0;
	result = 0;
	sign = 1;
	while (nptr[i] == '+' || nptr[i] == '-' || nptr[i] == 32)
	{
		if (nptr[i] == '-')
			sign = -1;
		i++;
	}
	while (nptr[i] >= '0' && nptr[i] <= '9')
	{
		result = result * 10 + (nptr[i] - '0');
		if (result * sign > INT_MAX || result * sign < 0)
			return (-1);
		i++;
	}
	return ((int)result * sign);
}
