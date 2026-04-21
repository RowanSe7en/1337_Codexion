STEP 1 — Allocation (YES, you start here)

You will typically allocate:

1) Array of coders
t_coder *coders = malloc(sizeof(t_coder) * number_of_coders);
2) Array of dongles
t_dongle *dongles = malloc(sizeof(t_dongle) * number_of_coders);

✔ Why same size?
Because:

N coders → N dongles (circular adjacency)



STEP 1.1 — Initialize dongles

Each dongle is shared → must be safe from the start:

for (int i = 0; i < n; i++)
{
    dongles[i].dongle_id = i;
    dongles[i].available = 1;
    dongles[i].cooldown_until = 0;

    pthread_mutex_init(&dongles[i].mutex, NULL);
}