/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   loki.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:33:40 by brouane           #+#    #+#             */
/*   Updated: 2026/05/02 00:02:35 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void *malloc_for_me(long long bytes)
{
    void *take_this;

    take_this = malloc(bytes);
    if (!take_this)
        return NULL;
    
    return take_this;
}