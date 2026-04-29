/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   freedom.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:33:53 by brouane           #+#    #+#             */
/*   Updated: 2026/04/29 12:33:54 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int freedom(t_coder *coders, t_dongle *dongles)
{
    free(coders);
    free(dongles);

    return 1;
}