Here is the continuation of your README, written as a **progress log of what your code already achieves**.

---

# STEP 1 — Allocation (YES, you start here)

You allocate the core shared resources of the simulation.

### 1) Array of coders

```c
t_coder *coders = malloc(sizeof(t_coder) * number_of_coders);
```

### 2) Array of dongles

```c
t_dongle *dongles = malloc(sizeof(t_dongle) * number_of_coders);
```

✔ **Why same size?**

Because the simulation is circular:

```
Coder i  → needs Dongle i and Dongle (i+1)%N
```

N coders → N dongles.

---

# STEP 1.1 — Initialize dongles

Each dongle is a **shared resource**, so it must be safe from the beginning.

What your code does:

```c
for (int i = 0; i < size; i++)
{
    dongles[i].dongle_id = i;
    dongles[i].is_available = 1;
    pthread_mutex_init(&dongles[i].mtx, NULL);
}
```

✔ Each dongle now has:

| Field          | Meaning                      |
| -------------- | ---------------------------- |
| `dongle_id`    | Unique identifier            |
| `is_available` | Logical availability flag    |
| `mtx`          | Mutex protecting this dongle |

✔ This is the **first real concurrency protection** in the project.

---

# STEP 2 — Create the Simulation Object

You created a **central shared state container**.

```c
t_simulation sim;
sim.args = data;
sim.coders = coders;
sim.dongles = dongles;
sim.is_finished = 0;
sim.start_time = get_time_ms();
```

This struct is **critical architecture**.

It acts as the global context shared by all threads.

### What this means architecturally

Instead of global variables, you use a **shared struct** passed to threads.

This makes the program:

* modular
* testable
* thread-safe ready
* aligned with 42/philosophers architecture

✔ This is a big milestone.

---

# STEP 2.1 — Global Log Mutex

You created a mutex for synchronized output.

```c
pthread_mutex_init(&sim.log_mtx, NULL);
```

✔ Why this is important:

Multiple threads will print at the same time → without a mutex → logs become corrupted/interleaved.

You prepared the infrastructure for **thread-safe logging**.

---

# STEP 3 — Initialize Coders

You created and wired every coder.

```c
for (int i = 0; i < size; i++)
{
    coders[i].coder_id = i + 1;
    coders[i].compile_count = 0;
    coders[i].last_compile_time = 0;
    coders[i].sim = &sim;

    coders[i].first_dongle = &dongles[i];
    coders[i].second_dongle = &dongles[(i + 1) % size];
}
```

### What you achieved here

Each coder now knows:

| Field               | Meaning                      |
| ------------------- | ---------------------------- |
| `coder_id`          | Unique human-readable ID     |
| `compile_count`     | Progress counter             |
| `last_compile_time` | Timing state                 |
| `sim`               | Pointer to shared simulation |
| `first_dongle`      | Left resource                |
| `second_dongle`     | Right resource               |

✔ The circular dependency is now fully wired.

This line is extremely important:

```c
coders[i].second_dongle = &dongles[(i + 1) % size];
```

This creates the **circular topology** of the dining philosophers problem.

You have now successfully modeled the **resource graph**.

---

# STEP 4 — Time System

You implemented a time helper:

```c
long get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
```

✔ This provides:

* millisecond precision timing
* simulation start timestamp
* foundation for burnout/compile timing later

This is the **clock of the simulation**.

---

# STEP 5 — Thread Routine Skeleton

You created the first thread routine:

```c
void *coder_routine(void *arg)
{
    t_coder *c = (t_coder *)arg;

    while (!c->sim->is_finished)
    {
        printf("%d is thinking\n", c->coder_id);
    }
    return NULL;
}
```

What exists right now:

* Infinite loop per coder
* Shared stop condition (`sim->is_finished`)
* Thread argument casting
* Basic behaviour placeholder

✔ Threads are now **alive and executing concurrently**.

This is the first moment your program becomes **multithreaded**.

---

# STEP 6 — Thread Creation

You spawn all coder threads:

```c
for (int i = 0; i < size; i++)
{
    pthread_create(&coders[i].coder, NULL, coder_routine, &coders[i]);
}
```

✔ This means:

* N threads created
* Each thread receives its own struct
* All threads run simultaneously

This is the **birth of concurrency** in your project.

---
