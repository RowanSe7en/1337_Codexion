/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kang.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brouane <brouane@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 20:50:03 by brouane           #+#    #+#             */
/*   Updated: 2026/06/04 17:28:46 by brouane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void thread_create(pthread_t *coder, void *func, t_code_sim *code_sim)// good
{
    int result = pthread_create(coder, NULL, func, code_sim);

    if (result != 0)
    {
        printf("Error: %s\n", strerror(result));
        freedom(code_sim->sim->coders, code_sim->sim->dongles);
        exit(1);
    }

}

void watcher_thread_create(pthread_t *watcher_thread, void *func, t_simulation *sim)// good
{
    int result = pthread_create(watcher_thread, NULL, func, sim);

    if (result != 0)
    {
        printf("Error: %s\n", strerror(result));
        freedom(sim->coders, sim->dongles);
        exit(1);
    }

}

void thread_join(pthread_t *coder, t_simulation *sim)// good
{
    int result = pthread_join(*coder, NULL);

    if (result != 0)
    {
        printf("Error: %s\n", strerror(result));
        freedom(sim->coders, sim->dongles);
        exit(1);
    }
}
