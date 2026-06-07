<div align="center">

```
 ██████╗ ██████╗ ██████╗ ███████╗██╗  ██╗██╗ ██████╗ ███╗   ██╗
██╔════╝██╔═══██╗██╔══██╗██╔════╝╚██╗██╔╝██║██╔═══██╗████╗  ██║
██║     ██║   ██║██║  ██║█████╗   ╚███╔╝ ██║██║   ██║██╔██╗ ██║
██║     ██║   ██║██║  ██║██╔══╝   ██╔██╗ ██║██║   ██║██║╚██╗██║
╚██████╗╚██████╔╝██████╔╝███████╗██╔╝ ██╗██║╚██████╔╝██║ ╚████║
 ╚═════╝ ╚═════╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝
```

[![Python](https://img.shields.io/badge/Python-3.10%2B-3776AB?style=for-the-badge&logo=python&logoColor=white)](https://www.python.org/)
[![42 School](https://img.shields.io/badge/42-brouane-000000?style=for-the-badge&logo=42&logoColor=white)](https://42.fr/)
[![Threading](https://img.shields.io/badge/Threading-semaphores-blueviolet?style=for-the-badge)](.)
[![Status](https://img.shields.io/badge/Status-Loading-success?style=for-the-badge)](.)
[![Score](https://img.shields.io/badge/Score-NA%2F100-gold?style=for-the-badge&logo=starship&logoColor=white)](.)

</div>

---

## ✅ Project grade screenshot
![Project screenshot](screenshots_preview/Screenshot.png)

---

# Codexion — Complete Technical README

> A concurrent simulation of coders sharing hardware dongles to compile code — a 1337/42 philosophers-style project rebuilt with real-world engineering tricks.

---

## Table of Contents

1. [What is Codexion?](#what-is-codexion)
2. [How to Run](#how-to-run)
3. [Project Architecture — File Map](#project-architecture--file-map)
4. [Data Structures — Every Field Explained](#data-structures--every-field-explained)
5. [Boot Sequence — From `main()` to First Thread](#boot-sequence--from-main-to-first-thread)
6. [The Deadlock Problem and the Even/Odd Solution](#the-deadlock-problem-and-the-evenodd-solution)
7. [The Dongle Lifecycle — `take_dongle()` Deep Dive](#the-dongle-lifecycle--take_dongle-deep-dive)
8. [The Scheduler System — FIFO vs EDF](#the-scheduler-system--fifo-vs-edf)
9. [The Dongle Cooldown — Hardware Simulation](#the-dongle-cooldown--hardware-simulation)
10. [The Coder Lifecycle — `main_loop()` Deep Dive](#the-coder-lifecycle--main_loop-deep-dive)
11. [The Watcher Thread — Death Detection](#the-watcher-thread--death-detection)
12. [Time System — Microsecond Precision](#time-system--microsecond-precision)
13. [The Global Synchronization Barrier](#the-global-synchronization-barrier)
14. [The Getter/Setter Pattern — Thread-Safe State Access](#the-gettersetter-pattern--thread-safe-state-access)
15. [Mutex Infrastructure — Wrapping POSIX](#mutex-infrastructure--wrapping-posix)
16. [Logging — Thread-Safe Output](#logging--thread-safe-output)
17. [Memory and Cleanup — `freedom()`](#memory-and-cleanup--freedom)
18. [The Parser — Argument Validation](#the-parser--argument-validation)
19. [Race Conditions — What Could Go Wrong](#race-conditions--what-could-go-wrong)
20. [Why Burnout Never Happens (Mathematical Proof)](#why-burnout-never-happens-mathematical-proof)

---

## What is Codexion?

Codexion simulates **N coders** sitting at a circular table. Each coder needs **two hardware dongles** (USB license keys) to compile their code. The dongles sit between coders — each coder shares one dongle with their left neighbour, one with their right.

This is the **Dining Philosophers problem** in developer clothing:

```
        Dongle 1
    C1 ────────── C5
    │               │
Dongle 2         Dongle 5
    │               │
    C2             C4
    │               │
Dongle 3         Dongle 4
    │               │
     ──────C3────────
            
```

The challenge: implement this without **deadlock**, **starvation**, or **data races**.

Each coder runs a life cycle of:

```
compile → debug → refactor → compile → debug → refactor → ...
```

Until either:
- A coder hasn't compiled in `time_to_burnout` milliseconds → simulation ends with burnout log
- All coders reach `number_of_compiles_required` → simulation ends cleanly

---

## How to Run

```bash
make
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <number_of_compiles_required> <dongle_cooldown> <scheduler>
```

### Arguments

| Argument | Type | Meaning |
|---|---|---|
| `number_of_coders` | int > 0 | How many coders (and dongles) exist |
| `time_to_burnout` | int ms | Max time a coder can go without compiling |
| `time_to_compile` | int ms | How long compiling takes |
| `time_to_debug` | int ms | How long debugging takes (dongle-free) |
| `time_to_refactor` | int ms | How long refactoring takes (dongle-free) |
| `number_of_compiles_required` | int | Target compiles per coder (0 = infinite) |
| `dongle_cooldown` | int ms | Minimum rest time between a dongle's uses |
| `scheduler` | string | Either `fifo` or `edf` |

### Example

```bash
./codexion 5 800 200 100 100 10 0 fifo
```

5 coders. Must compile every 800ms. Compiling takes 200ms. Debug/refactor each take 100ms. Stop after 10 compiles each. No dongle cooldown. FIFO scheduling.

### Output format

Every event is printed as:

```
<timestamp_ms> <coder_id> <action>
```

Actions:
- `has taken a dongle`
- `is compiling`
- `is debugging`
- `is refactoring`
- `burned out`

---

## Project Architecture — File Map

```
codexion/
├── codexion.h            → All structs and function prototypes
├── codexion.c            → main(), init_dongles(), init_coders(), init_simulation()
├── simulation_start.c    → program_starter(), thread creation, barrier, joining
├── coder_routine.c       → main_loop(), compile(), debug(), refactor()
├── dongle.c              → compile(), finish_compile(), unlock_dongles()
├── dongle_utils.c        → take_dongle(), register_dongle(), fifo_register/wait
├── edf_scheduler.c       → compute_deadline(), edf_register(), edf_wait_turn()
├── sync_utils.c          → is_finished(), sync_threads(), dongle_is_ready(), reset_passed()
├── watcher.c             → the_watcher(), check_if_coder_burned_out(), check_if_all_compiles_done()
├── coder_getters.c       → Thread-safe getters for coder/dongle/sim state
├── simulation_getters.c  → get_coders_passed(), get_ready()
├── state_setters.c       → Thread-safe setters for all mutable state
├── mutex_utils.c         → lock_mutex(), unlock_mutex(), initiate_mutex(), destroy_them_all()
├── thread_utils.c        → thread_create(), watcher_thread_create(), thread_join()
├── time_utils.c          → get_time_ms(), get_time_us(), precise_sleep(), ms_to_us(), us_to_ms()
├── logging.c             → log_action(), bye_bye()
├── parser.c              → parser(), int_parser(), str_parser(), check_values()
├── parsing_utils.c       → dig_sign_checker(), ft_isdigit(), ft_issign()
├── ft_atoi.c             → ft_atoi() with overflow detection
└── freedom.c             → freedom() — all cleanup and exit
```

---

## Data Structures — Every Field Explained

### `t_arguments` — Parsed CLI input

```c
typedef struct s_arguments
{
    int   number_of_coders;            // N coders = N dongles
    int   time_to_burnout;             // ms — max gap between compiles
    int   time_to_compile;             // ms — compile phase duration
    int   time_to_debug;               // ms — debug phase duration
    int   time_to_refactor;            // ms — refactor phase duration
    int   number_of_compiles_required; // stop condition (0 = run forever)
    int   dongle_cooldown;             // ms — dongle rest time between uses
    char *scheduler;                   // "fifo" or "edf"
    int   valid;                       // 0 if parsing failed, 1 if OK
} t_arguments;
```

### `t_scheduler` — Per-dongle fairness tracker

```c
typedef struct s_scheduler
{
    pthread_mutex_t counter_mtx; // protects the fields below
    long long       order[2];    // holds 2 slots: coder_id (FIFO) or deadline (EDF)
    short           counter;     // how many coders are currently queued (0, 1, or 2)
} t_scheduler;
```

This embedded struct is inside every dongle. It tracks at most 2 coders competing for the same dongle at the same time, because only 2 coders ever share any given dongle in the circular setup.

### `t_dongle` — The shared hardware resource

```c
typedef struct s_dongle
{
    int             dongle_id;       // 1-indexed identifier
    long long       last_used_time;  // microsecond timestamp of last release
    int             coders_passed;   // how many coders are currently in the queue
    pthread_mutex_t dongle_mtx;      // main lock — held while coder is compiling
    pthread_mutex_t used_time_mtx;   // protects last_used_time
    pthread_mutex_t passed_mtx;      // protects coders_passed
    pthread_mutex_t reset_mtx;       // protects scheduler reset operations
    t_scheduler     scheduler;       // fairness queue (FIFO or EDF)
} t_dongle;
```

Why four separate mutexes on one dongle? Because different threads read different fields at different rates. Coarse-locking the whole dongle would force the watcher and the compiling coder to serialize unnecessarily. Each mutex protects exactly the data it guards.

### `t_coder` — One thread's entire state

```c
typedef struct s_coder
{
    int             coder_id;          // 1-indexed, human-readable
    long long       compile_count;     // how many compiles done so far
    long long       last_compile_time; // microseconds — used by watcher for burnout
    pthread_t       coder;             // the thread handle
    pthread_mutex_t state_mtx;         // protects compile_count and last_compile_time
    t_dongle       *first_dongle;      // the dongle this coder grabs first
    t_dongle       *second_dongle;     // the dongle this coder grabs second
    t_simulation   *sim;               // back-pointer to shared simulation state
} t_coder;
```

### `t_simulation` — The global shared brain

```c
struct s_simulation
{
    t_arguments     args;             // parsed CLI arguments (read-only after init)
    t_coder        *coders;           // array of all coders
    t_dongle       *dongles;          // array of all dongles
    void           *codes_sims;       // array of t_code_sim (thread arguments)
    pthread_t       watcher_thread;   // the referee thread handle
    pthread_mutex_t log_mtx;          // serializes all printf output
    pthread_mutex_t is_ready_mtx;     // protects is_all_ready
    pthread_mutex_t is_finished_mtx;  // protects is_finished
    pthread_mutex_t start_time_mtx;   // protects start_time
    long long       start_time;       // microsecond timestamp of simulation start
    short           is_finished;      // 1 = stop everything
    short           is_all_ready;     // 1 = all threads spawned, GO signal
    short           is_edf;           // 1 if scheduler == "edf"
};
```

### `t_code_sim` — Thread argument bundle

```c
typedef struct s_code_sim
{
    t_simulation *sim;   // shared state
    t_coder      *coder; // this thread's coder
} t_code_sim;
```

This is what gets passed as `void *arg` to each coder thread. It bundles the two pointers the thread needs without resorting to global variables. The watcher gets just `t_simulation *` since it doesn't belong to any single coder.

---

## Boot Sequence — From `main()` to First Thread

### Phase 1 — `main()` in `codexion.c`

```c
int main(int ac, char **av)
{
    if (ac != 9)
        return (bye_bye());        // exactly 8 args required
    data = parser(ac, av);
    if (data.valid == 0)
        return (1);
    coders  = malloc(sizeof(t_coder)   * size);
    dongles = malloc(sizeof(t_dongle)  * size);
    sim.coders  = coders;
    sim.dongles = dongles;
    if (!coders || !dongles)
        freedom(&sim, 0);          // no mutexes to destroy yet
    init_simulation(&sim, data);   // init sim-level state + mutexes
    init_dongles(&sim, size);      // init each dongle + its mutexes
    init_coders(&sim, size);       // wire each coder to its two dongles
    program_starter(&sim);         // spawn threads, set GO signal, join
    freedom(&sim, 1);              // destroy mutexes, free memory, exit
}
```

Note the `freedom(&sim, 0)` vs `freedom(&sim, 1)` distinction. If malloc fails before any mutexes are initialized, we pass `is_destroy = 0` to skip `pthread_mutex_destroy()` on uninitialized memory, which would be undefined behaviour.

### Phase 2 — `init_simulation()`

```c
void init_simulation(t_simulation *sim, t_arguments data)
{
    sim->args        = data;
    sim->is_finished = 0;
    sim->is_all_ready = 0;
    sim->is_edf      = (strcmp(sim->args.scheduler, "edf") == 0);
    initiate_mutex(&sim->log_mtx,         sim);
    initiate_mutex(&sim->start_time_mtx,  sim);
    initiate_mutex(&sim->is_ready_mtx,    sim);
    initiate_mutex(&sim->is_finished_mtx, sim);
}
```

The `is_edf` flag is resolved once here at startup from the string comparison. All hot paths in the simulation check `sim->is_edf` (a short) rather than calling `strcmp` on every dongle acquisition.

### Phase 3 — `init_dongles()`

Each dongle starts with:
- `last_used_time = 0` — means "never used", so cooldown check passes immediately on the first use
- `coders_passed = 0` — no one in queue yet
- `scheduler.counter = 0` — queue is empty

All five mutexes are initialized. If any `pthread_mutex_init` fails, `initiate_mutex` prints the error and calls `freedom` — hard stop.

### Phase 4 — `init_coders()` — the Even/Odd Wiring

```c
if (sim->coders[i].coder_id % 2 == 0)
{
    sim->coders[i].first_dongle  = &sim->dongles[i];
    sim->coders[i].second_dongle = &sim->dongles[(i + 1) % size];
}
else
{
    sim->coders[i].first_dongle  = &sim->dongles[(i + 1) % size];
    sim->coders[i].second_dongle = &sim->dongles[i];
}
```

This is where deadlock is structurally prevented. See the full explanation in the next section.

### Phase 5 — `program_starter()` — Thread Launch Sequence

This is the most sensitive function in the whole project. The order of operations is not accidental:

```c
void program_starter(t_simulation *sim)
{
    // 1. Allocate thread argument bundles
    codes_sims = malloc(sizeof(t_code_sim) * count);
    sim->codes_sims = codes_sims;

    // 2. Spawn all coder threads (they immediately spin on is_all_ready)
    create_coder_threads(sim, codes_sims, count);

    // 3. Spawn the watcher thread (it also spins on is_all_ready)
    watcher_thread_create(&sim->watcher_thread, the_watcher, sim);

    // 4. Record the start timestamp (microseconds)
    lock_mutex(&sim->start_time_mtx, sim);
    sim->start_time = get_time_us();
    unlock_mutex(&sim->start_time_mtx, sim);

    // 5. Set initial last_compile_time for all coders = start_time
    //    This seeds the burnout timer so no coder dies at t=0
    set_initial_compile_times(codes_sims, count);

    // 6. Fire the GO signal — all threads unblock simultaneously
    lock_mutex(&sim->is_ready_mtx, sim);
    sim->is_all_ready = 1;
    unlock_mutex(&sim->is_ready_mtx, sim);

    // 7. Main thread waits for all coders to finish
    join_coder_threads(sim, count);

    // 8. Join the watcher
    thread_join(&sim->watcher_thread, sim);
}
```

Why set `last_compile_time = start_time` before releasing threads? Because the watcher starts checking for burnout the moment `is_all_ready` is set. If `last_compile_time` stayed at 0, the watcher would compute `now - 0` which could easily exceed `time_to_burnout` in the first loop iteration, killing the simulation instantly before any coder gets a chance to run.

---

## The Deadlock Problem and the Even/Odd Solution

### Why Deadlock Happens

In the classic circular setup, if every coder grabs their left dongle first then waits for their right, you get:

```
Coder 1 holds Dongle 1 → waits Dongle 2
Coder 2 holds Dongle 2 → waits Dongle 3
Coder 3 holds Dongle 3 → waits Dongle 4
Coder 4 holds Dongle 4 → waits Dongle 5
Coder 5 holds Dongle 5 → waits Dongle 1
```

Everyone holds one resource and waits for a resource held by the next person. Perfect circular wait. The system freezes forever.

### The Even/Odd Fix

The solution is in `init_coders()`. The **assignment** of `first_dongle` vs `second_dongle` is reversed for even-numbered coders:

| Coder ID | `first_dongle` | `second_dongle` |
|---|---|---|
| Odd (1, 3, 5...) | `dongles[(i+1) % N]` | `dongles[i]` |
| Even (2, 4, 6...) | `dongles[i]` | `dongles[(i+1) % N]` |

Because `compile()` always locks `first_dongle` before `second_dongle`, odd and even coders grab resources in opposite orders. The circular chain cannot form because at least one adjacent pair of coders always tries to grab the same dongle first — meaning one wins immediately and the other blocks, breaking the cycle.

### Visual Proof (5 coders, circular table)

At t=0, all coders race for their `first_dongle`:

- C1 (odd) tries `dongles[1]`
- C2 (even) tries `dongles[1]` — same as C1!
- C3 (odd) tries `dongles[3]`
- C4 (even) tries `dongles[3]` — same as C3!
- C5 (odd) tries `dongles[0]`

Two races immediately form: C1 vs C2 for D1, and C3 vs C4 for D3. One from each pair wins instantly. The other blocks. The circle is already broken before anyone holds two dongles.

---

## The Dongle Lifecycle — `take_dongle()` Deep Dive

`take_dongle()` in `dongle_utils.c` is the most complex function in the project. It does five things in sequence:

```c
int take_dongle(t_code_sim *cs, t_dongle *d, int already_held)
{
    // 1. Wait for dongle cooldown to expire
    wait_dongle_ready(d, sim);

    // 2. Exit early if simulation ended while waiting
    if (is_finished(sim))
        return (0);

    // 3. Special case: only one coder exists — can't use two dongles
    if (already_held)
    {
        while (!is_finished(sim))
            precise_sleep(1, sim);
        return (0);
    }

    // 4. Register in the fairness queue and wait your turn
    register_dongle(cs, d);

    // 5. Exit if simulation ended during queue wait
    if (is_finished(sim))
        return (0);

    // 6. Spin-lock until the dongle mutex is acquirable AND cooldown passed
    if (!wait_and_lock_dongle(d, sim))
        return (0);

    // 7. Reset scheduler queue for next round
    edf_reset(d, sim);
    reset_passed(d, sim);

    // 8. Log the acquisition
    log_action(sim, coder, "has taken a dongle");

    return (1);
}
```

### The `already_held` Edge Case

```c
same_dongle = (cs->coder->first_dongle == cs->coder->second_dongle);
```

When `number_of_coders == 1`, there is only one dongle and it would be both `first_dongle` and `second_dongle`. A coder cannot take the same dongle twice (mutex deadlock on the same thread). The `already_held` flag detects this via pointer comparison and enters an infinite wait — the single coder can never compile, and the watcher will eventually declare burnout. This is the correct and expected behaviour for N=1.

### `wait_and_lock_dongle()` — The Spin-Try Loop

```c
int wait_and_lock_dongle(t_dongle *d, t_simulation *sim)
{
    while (1)
    {
        lock_mutex(&d->dongle_mtx, sim);       // acquire the main lock
        if (is_finished(sim))
        {
            unlock_mutex(&d->dongle_mtx, sim); // clean release on shutdown
            return (0);
        }
        if (dongle_is_ready(d, ms_to_us(sim->args.dongle_cooldown), sim))
            break;                             // cooldown passed — we're in
        unlock_mutex(&d->dongle_mtx, sim);     // cooldown not yet passed — release and retry
        precise_sleep(1, sim);
    }
    return (1);
}
```

Why lock, check cooldown, potentially unlock and retry? Because the cooldown check (`elapsed >= cooldown`) must be done while holding the mutex — otherwise another thread could sneak in between the check and the lock and steal the dongle during its cooldown window.

---

## The Scheduler System — FIFO vs EDF

When two coders race for the same dongle, they both call `register_dongle()`. The scheduler decides which one gets priority.

### The Queue Mechanism

The `t_scheduler` inside each dongle holds at most 2 slots (since at most 2 coders ever share one dongle):

```c
typedef struct s_scheduler
{
    pthread_mutex_t counter_mtx;
    long long       order[2];    // slot 0 and slot 1
    short           counter;     // 0, 1, or 2
} t_scheduler;
```

`coders_passed` on the dongle counts how many of those queued coders have already been "seen and decided". Together these two numbers tell any waiting coder where it stands.

### FIFO Mode — First-Come, First-Served

```c
void fifo_register(t_dongle *d, int coder_id, t_simulation *sim)
{
    lock_mutex(&d->scheduler.counter_mtx, sim);
    d->scheduler.order[d->scheduler.counter++] = coder_id;
    unlock_mutex(&d->scheduler.counter_mtx, sim);
}
```

The coder writes their `coder_id` into the next available slot. `counter` is 0 or 1 before writing.

```c
void fifo_wait_turn(t_dongle *d, int my_id, t_code_sim *cs)
{
    while (1)
    {
        if (is_finished(cs->sim))
            return;
        counter = get_counter(d, cs->sim);
        passed  = get_coders_passed(d, cs->sim);

        // Only one coder registered — go immediately
        if (counter == 0 || (counter == 1 && passed == 1))
            return;

        // Two coders registered — check who was first
        if (counter == 2 && passed == 2)
        {
            first = d->scheduler.order[0];
            if (first == my_id)
                return;     // I was first → my turn
        }
        precise_sleep(1, cs->sim);
    }
}
```

The trick: `coders_passed` is incremented by `set_coders_passed()` before `take_dongle()` is called. So by the time both coders are in `fifo_wait_turn`, `passed == 2` and `order[0]` holds the ID of whoever registered first. The other one keeps sleeping.

After the winner finishes and releases the dongle, `edf_reset()` and `reset_passed()` clear the queue for the next round.

### EDF Mode — Earliest Deadline First

EDF gives priority to the coder closest to burning out. It uses the same 2-slot queue but stores **deadlines** instead of IDs.

```c
long long compute_deadline(t_coder *coder, t_simulation *sim)
{
    return (get_last_compile_time(coder, sim)
        + ms_to_us(sim->args.time_to_burnout)
        - get_start_time(sim));
}
```

The deadline is: `last_compile_time + time_to_burnout - start_time`. This gives a relative urgency score. Lower deadline = compiled longer ago = more urgent = higher priority.

```c
int has_priority(t_dongle *d, long long my_deadline, t_code_sim *code_sim)
{
    a = d->scheduler.order[0];
    b = d->scheduler.order[1];
    if (a < b && a == my_deadline)
        return (1);    // slot 0 has smallest deadline and it's mine
    if (b <= a && b == my_deadline)
        return (1);    // slot 1 has smallest (or equal) deadline and it's mine
    return (0);
}
```

If both deadlines are equal (two coders with identical burnout urgency), the coder in slot 1 wins. This is an arbitrary tiebreak but it is consistent and deterministic.

EDF is the academically correct solution for real-time scheduling where missing a deadline (burning out) is catastrophic. FIFO is simpler and more predictable in practice.

---

## The Dongle Cooldown — Hardware Simulation

The `dongle_cooldown` argument simulates a real hardware constraint: a USB dongle needs a brief rest between uses before it can be reacquired by another coder.

### How it is Tracked

`last_used_time` on the dongle is set in **microseconds** when a coder finishes compiling:

```c
void finish_compile(t_code_sim *cs)
{
    long long now = get_time_us();
    set_last_compile_time(cs->coder, now, cs->sim);
    set_last_used_time(cs->coder->first_dongle, now, cs->sim);
    set_last_used_time(cs->coder->second_dongle, now, cs->sim);
}
```

Both dongles get a timestamp when released. This is why each dongle has a dedicated `used_time_mtx` — the watcher, the coder, and `dongle_is_ready` all potentially read this field from different threads.

### Cooldown Check

```c
int dongle_is_ready(t_dongle *d, long long cooldown, t_simulation *sim)
{
    long long now     = get_time_us();
    long long elapsed = now - get_last_used_time(d, sim);
    return (elapsed >= cooldown);
}
```

`cooldown` is passed in already converted to microseconds via `ms_to_us()`. The comparison is done in microseconds throughout to avoid unit confusion in hot paths.

If `dongle_cooldown = 0` (as in most normal runs), `last_used_time = 0` at init means `elapsed = now - 0 = now`, which is always `>= 0`, so the check always passes. Zero cooldown = no hardware restriction.

---

## The Coder Lifecycle — `main_loop()` Deep Dive

```c
void *main_loop(void *arg)
{
    code_sim = (t_code_sim *)arg;
    sync_threads(code_sim->sim);                        // wait for GO signal
    required = code_sim->sim->args.number_of_compiles_required;

    while (!is_finished(code_sim->sim))
    {
        compile_count = get_compile_count(code_sim->coder, code_sim->sim);
        if (compile_count == required)
            break;          // this coder reached its goal — exit cleanly
        compile(code_sim);
        debug(code_sim);
        refactor(code_sim);
    }
    return (NULL);
}
```

Note: `compile_count == required` uses the raw value `0` for required when no limit is set. `0 == 0` would immediately exit. The parser must ensure `number_of_compiles_required` is only `0` when it means "run forever" — which is handled by the watcher checking compile counts only when `required > 0`.

Actually, looking at the watcher code carefully: `check_if_all_compiles_done` compares `compile_count != required`. If `required = 0` and `compile_count = 0` initially, this would immediately set finished. This is the "zero compiles required" edge case — the simulation exits immediately, which is the correct semantics.

### `compile()` in `dongle.c`

```c
void compile(t_code_sim *cs)
{
    int same_dongle = (cs->coder->first_dongle == cs->coder->second_dongle);

    set_coders_passed(cs->coder->first_dongle, cs->sim);  // register intent
    if (!take_dongle(cs, cs->coder->first_dongle, 0))
        return;

    set_coders_passed(cs->coder->second_dongle, cs->sim); // register intent for 2nd
    if (!take_dongle(cs, cs->coder->second_dongle, same_dongle))
    {
        unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim); // release first
        return;
    }

    log_action(cs->sim, cs->coder, "is compiling");
    precise_sleep(cs->sim->args.time_to_compile, cs->sim);

    if (is_finished(cs->sim))
    {
        unlock_dongles(cs, same_dongle);
        return;
    }
    finish_compile(cs);        // update timestamps
    unlock_dongles(cs, same_dongle);
}
```

Key safety detail: if `take_dongle` for the second dongle fails (simulation ended), we must release the first dongle before returning. Forgetting this would leave a mutex permanently locked — deadlock on the shutdown path.

The `is_finished` check after `precise_sleep` prevents updating `last_compile_time` after the simulation has ended. If a burnout happens while a coder is sleeping during compile, we don't want that coder to overwrite the timestamp and confuse the watcher.

### `debug()` and `refactor()`

```c
void debug(t_code_sim *code_sim)
{
    log_action(code_sim->sim, code_sim->coder, "is debugging");
    precise_sleep(code_sim->sim->args.time_to_debug, code_sim->sim);
}

void refactor(t_code_sim *code_sim)
{
    log_action(code_sim->sim, code_sim->coder, "is refactoring");
    precise_sleep(code_sim->sim->args.time_to_refactor, code_sim->sim);
    if (!is_finished(code_sim->sim))
        set_compile_count(code_sim->coder, code_sim->sim);
}
```

The compile counter is incremented at the **end of refactor**, not at the start of compile. This means the watcher counts a "completed compile cycle" only after the coder finishes the full triplet: compile + debug + refactor. This is a design choice — it counts completed work, not started work.

Why check `is_finished` before `set_compile_count` in refactor? If the simulation ends during the refactor sleep (another coder burned out), we skip incrementing the counter. This avoids a race where the watcher sees all counters hit the target and sets finished right as another coder is being logged as burned out.

---

## The Watcher Thread — Death Detection

The watcher is a separate referee thread that runs concurrently with all coders. It never touches dongles or simulates work — it only observes.

```c
void *the_watcher(void *arg)
{
    sim = (t_simulation *)arg;
    sync_threads(sim);    // wait for GO signal, same as coders

    while (!is_finished(sim))
    {
        if (check_if_coder_burned_out(sim))
        {
            set_finished(sim);
            return (NULL);
        }
        if (!is_finished(sim))
        {
            check_if_all_compiles_done(sim);
            usleep(100);    // 0.1ms polling interval
        }
    }
    return (NULL);
}
```

The `usleep(100)` gives the watcher a 0.1ms polling loop. This means burnout detection has up to 0.1ms of latency — meaning a coder could theoretically be 0.1ms late and it still gets caught. In practice `time_to_burnout` is hundreds of milliseconds so this is negligible.

### `check_if_coder_burned_out()`

```c
short check_if_coder_burned_out(t_simulation *sim)
{
    i = 0;
    while (i < sim->args.number_of_coders)
    {
        // Skip coders who already finished their required compiles
        if (get_compile_count(&sim->coders[i], sim)
            >= sim->args.number_of_compiles_required)
        {
            i++;
            continue;
        }
        last_compile_time = get_last_compile_time(&sim->coders[i], sim);
        now = get_time_us();
        if (now - last_compile_time >= ms_to_us(sim->args.time_to_burnout))
        {
            log_action(sim, &sim->coders[i], "burned out");
            return (1);
        }
        i++;
    }
    return (0);
}
```

The watcher skips coders who already reached their compile target. A coder who is "done" is not expected to compile anymore, so its `last_compile_time` becoming stale is not a burnout.

All time comparisons are in **microseconds** — `ms_to_us()` converts the argument before comparing. This prevents any integer truncation issues when `time_to_burnout` is small.

### `check_if_all_compiles_done()`

```c
void check_if_all_compiles_done(t_simulation *sim)
{
    i = 0;
    while (i < sim->args.number_of_coders)
    {
        compile_count = get_compile_count(&sim->coders[i], sim);
        if (compile_count != sim->args.number_of_compiles_required)
            return;    // at least one coder not done — bail out early
        i++;
    }
    set_finished(sim);    // every coder is done
}
```

This is an early-exit loop — the moment one coder hasn't reached the target, the function returns without checking the rest. `set_finished` is only called if the loop completes without returning.

---

## Time System — Microsecond Precision

### Two Time Scales

The project uses two time granularities:

| Function | Resolution | Used for |
|---|---|---|
| `get_time_ms()` | milliseconds | Log timestamps, `precise_sleep` loop |
| `get_time_us()` | microseconds | `last_compile_time`, `last_used_time`, burnout comparison |

The internal state (burnout timer, dongle cooldown) runs in microseconds for maximum precision. Logs are printed in milliseconds since that is what the project specification requires and what makes output human-readable.

### `precise_sleep()` — The Smart Sleep

Standard `usleep(N)` sleeps *at least* N microseconds. OS scheduling jitter can add tens of milliseconds of overshoot. For a simulation where missing a 800ms deadline by 50ms means wrong burnout detection, this is not acceptable.

```c
void precise_sleep(long long duration_ms, t_simulation *sim)
{
    long long start   = get_time_ms();
    long long elapsed;

    while (1)
    {
        if (is_finished(sim))
            break;
        elapsed = get_time_ms() - start;
        if (elapsed >= duration_ms)
            break;
        usleep(1000);    // sleep 1ms chunks
    }
}
```

Instead of one long sleep, we sleep in 1ms chunks and check the elapsed time after each chunk. This bounds the overshoot to ~1ms. The `is_finished` check inside the loop means any thread sleeping during a burnout event wakes up within 1ms of the simulation ending, instead of sleeping through the rest of its compile/debug/refactor phase.

### `ms_to_us()` and `us_to_ms()`

```c
long long ms_to_us(long long ms) { return (ms * 1000LL); }
long long us_to_ms(long long us) { return (us / 1000LL); }
```

These exist to make unit conversions explicit and searchable. A raw `* 1000` in the middle of a comparison is a maintenance hazard. Using named functions makes the intent clear and makes unit bugs easier to catch in code review.

---

## The Global Synchronization Barrier

All threads (coders + watcher) call `sync_threads()` as their very first action:

```c
void sync_threads(t_simulation *sim)
{
    while (!get_ready(sim))
        usleep(1000);    // spin-wait 1ms at a time
}
```

And `get_ready()` is a mutex-protected read:

```c
short get_ready(t_simulation *sim)
{
    short answer;
    lock_mutex(&sim->is_ready_mtx, sim);
    answer = sim->is_all_ready;
    unlock_mutex(&sim->is_ready_mtx, sim);
    return (answer);
}
```

The main thread sets `is_all_ready = 1` only after:
1. All coder threads are spawned
2. The watcher thread is spawned
3. `start_time` is recorded
4. `last_compile_time` is seeded for all coders

This guarantees that the moment any thread unblocks from `sync_threads`, the global clock is set and no coder will immediately die. The barrier is a hard synchronization point — without it, a fast scheduler might let Thread 1 run 200ms of real time before Thread 5 is even created, giving Thread 1 a false head start in the timing race.

Why `usleep(1000)` in the spin? Without any sleep, the spin consumes 100% of a CPU core checking the flag. With 1ms sleeps, the main thread gets CPU time to finish setup, and the overall barrier latency is bounded by the setup time (microseconds) plus at most 1ms of sleep overshoot.

---

## The Getter/Setter Pattern — Thread-Safe State Access

Every mutable field that is accessed from multiple threads is protected by a dedicated mutex. Direct field access (e.g., `sim->is_finished` without locking) is never done outside of initialization — it would be a data race.

The project implements this cleanly through a getter/setter API:

### Getters (read with lock, copy out, unlock)

```c
long long get_last_compile_time(t_coder *coder, t_simulation *sim)
{
    long long answer;
    lock_mutex(&coder->state_mtx, sim);
    answer = coder->last_compile_time;
    unlock_mutex(&coder->state_mtx, sim);
    return (answer);    // return a copy — no pointer to protected data
}
```

The value is copied into a local variable before unlocking. This pattern ensures the caller receives a consistent snapshot even if another thread immediately modifies the field after the unlock.

### Setters (lock, write, unlock)

```c
void set_last_compile_time(t_coder *coder, long long now, t_simulation *sim)
{
    lock_mutex(&coder->state_mtx, sim);
    coder->last_compile_time = now;
    unlock_mutex(&coder->state_mtx, sim);
}
```

The full list of getter/setter pairs:

| Field | Getter | Setter | Mutex |
|---|---|---|---|
| `coder->last_compile_time` | `get_last_compile_time()` | `set_last_compile_time()` | `coder->state_mtx` |
| `coder->compile_count` | `get_compile_count()` | `set_compile_count()` | `coder->state_mtx` |
| `dongle->last_used_time` | `get_last_used_time()` | `set_last_used_time()` | `dongle->used_time_mtx` |
| `dongle->coders_passed` | `get_coders_passed()` | `set_coders_passed()` | `dongle->passed_mtx` |
| `sim->start_time` | `get_start_time()` | _(set once in starter)_ | `sim->start_time_mtx` |
| `sim->is_finished` | `is_finished()` | `set_finished()` | `sim->is_finished_mtx` |
| `sim->is_all_ready` | `get_ready()` | _(set once in starter)_ | `sim->is_ready_mtx` |
| `dongle->scheduler.counter` | `get_counter()` | _(inline in register)_ | `scheduler.counter_mtx` |

---

## Mutex Infrastructure — Wrapping POSIX

Raw `pthread_mutex_lock()` returns an error code on failure. Most programs ignore this. Codexion does not.

```c
void lock_mutex(pthread_mutex_t *mutex, t_simulation *sim)
{
    int result = pthread_mutex_lock(mutex);
    if (result != 0)
    {
        lock_mutex(&sim->log_mtx, sim);
        printf("Error: %s\n", strerror(result));
        unlock_mutex(&sim->log_mtx, sim);
        freedom(sim, 1);
    }
}
```

If a mutex operation fails, the program prints the system error, destroys all mutexes, frees all memory, and exits. No silent data corruption, no undefined behaviour.

The same pattern applies to `unlock_mutex()` and `initiate_mutex()`.

`destroy_them_all()` iterates all dongles and coders destroying all 5 dongle mutexes and 1 coder mutex per index, plus the 4 simulation-level mutexes. Total: `N * 6 + 4` mutex destructions on clean exit.

---

## Logging — Thread-Safe Output

```c
void log_action(t_simulation *sim, t_coder *coder, char *action)
{
    long long timestamp;

    lock_mutex(&sim->log_mtx, sim);
    if (!is_finished(sim))
    {
        timestamp = get_time_ms() - us_to_ms(get_start_time(sim));
        printf("%lld %d %s\n", timestamp, coder->coder_id, action);
    }
    unlock_mutex(&sim->log_mtx, sim);
}
```

Three important decisions here:

**1. The check inside the lock.** `is_finished` is checked while holding `log_mtx`. This prevents a thread from printing after a burnout event even if it was scheduled between the watcher setting `is_finished` and this thread acquiring `log_mtx`.

**2. The timestamp is computed inside the lock.** If we computed the timestamp before locking, a thread could be preempted between computing it and printing — giving us a timestamp that does not match when the print actually happens. Computing inside the lock gives the most accurate timestamp for what is actually being printed.

**3. `us_to_ms(get_start_time(sim))` converts start time from microseconds to milliseconds** so the subtraction `get_time_ms() - start_ms` is a clean millisecond-resolution elapsed time. Internally the clock runs in microseconds for burnout precision, but logs display in milliseconds for readability.

---

## Memory and Cleanup — `freedom()`

```c
void freedom(t_simulation *sim, short is_destroy)
{
    if (is_destroy)
        destroy_them_all(sim);    // only if mutexes were initialized
    if (sim->coders)
        free(sim->coders);
    if (sim->dongles)
        free(sim->dongles);
    if (sim->codes_sims)
        free(sim->codes_sims);
    exit(0);
}
```

`freedom` is the single exit point of the entire program. It is called in every error path and at normal program end. The `is_destroy` flag controls whether `pthread_mutex_destroy` is called — passing 0 when mutexes were never initialized avoids UB.

All three malloc'd arrays are null-checked before freeing. Since `sim` is stack-allocated in `main()`, it does not need to be freed — only its heap-pointed members do.

---

## The Parser — Argument Validation

The parser validates arguments in layers:

**Layer 1 — Format check (`int_parser`):** Each numeric argument is checked character by character via `dig_sign_checker()`. It rejects anything containing non-digit, non-sign characters, and validates that signs only appear at the front of the string.

**Layer 2 — Scheduler string check (`str_parser`):** The 9th argument must be exactly `"fifo"` or `"edf"`. Any other string is rejected with an error message.

**Layer 3 — Value check (`check_values`):** After format validation, `ft_atoi()` is called on each numeric argument. The custom `ft_atoi` detects overflow by checking if the accumulated result crosses INT_MAX during parsing — returning -1 on overflow, which `check_values` then rejects.

**Layer 4 — Semantic check:** `number_of_coders` must be > 0. The other values have no lower bound (0 cooldown and 0 burnout time are valid, albeit unusual).

```c
t_arguments parser(int ac, char **av)
{
    arguments.valid = 0;                         // default: invalid
    int_result = int_parser(ac, av);
    str_result = str_parser(av[8]);
    if (int_result || str_result)
        return (arguments);                      // early exit
    arguments.number_of_coders = ft_atoi(av[1]);
    if (arguments.number_of_coders <= 0)
    {
        printf("Number of coders should be more then ZERO\n");
        return (arguments);
    }
    // ... fill remaining fields ...
    arguments.valid = 1;
    return (arguments);
}
```

The `valid` flag on `t_arguments` lets `main()` check a single field after calling `parser()` rather than checking every field separately.

---

## Race Conditions — What Could Go Wrong

Here is a catalogue of the races that the design prevents:

### 1. Burnout at t=0

**Problem:** `last_compile_time` initialized to 0. Watcher sees `now - 0 = large number >= time_to_burnout` and immediately kills the simulation.

**Fix:** `set_initial_compile_times()` sets every coder's `last_compile_time = start_time` before `is_all_ready` is set. The watcher cannot start checking until `is_all_ready = 1`.

### 2. Interleaved log output

**Problem:** Two threads call `printf` simultaneously. Output on the same line gets scrambled: `100 1 is co200 2 is compilingmpiling`.

**Fix:** `log_mtx` wraps every `printf` call. Only one thread prints at a time.

### 3. Compile count incremented after burnout

**Problem:** Coder A detects burnout. Coder B, mid-refactor, increments its count. The watcher might see "all coders done" at the same time as "coder A burned out" and set finished twice.

**Fix:** `set_finished()` sets `is_finished` under `is_finished_mtx`. `is_finished()` reads under the same mutex. The `set_finished` call in the watcher uses this function too — setting 1 when it is already 1 is idempotent and safe.

### 4. Double-unlock of dongle

**Problem:** `compile()` takes both dongles. Simulation ends mid-compile. Both the early-return path and the normal path call `unlock_dongles()`. A mutex unlocked twice by the same thread is undefined behaviour.

**Fix:** The `is_finished` check after `precise_sleep` in `compile()` leads to `unlock_dongles(cs, same_dongle)` and `return` — a single unlock. The normal exit also calls `unlock_dongles` once. The two paths are mutually exclusive.

### 5. Stale `last_compile_time` after simulation ends

**Problem:** A coder finishes compile, the simulation is flagged finished by the watcher, then `finish_compile()` overwrites `last_compile_time`. The watcher re-reads a "fresh" time and thinks the coder is healthy even though it's over.

**Fix:** `compile()` checks `is_finished(cs->sim)` before calling `finish_compile()`. If the simulation has ended, `finish_compile()` is skipped entirely.

---

## Why Burnout Never Happens (Mathematical Proof)

**Burnout cannot occur** as long as `time_to_burnout` exceeds the worst-case wait a coder could face before compiling. In the worst case, a coder must wait through coder 6's full compile, then a dongle B cooldown, then coder 1's full compile, then a dongle A cooldown, then coder 2's full compile — only after all of that does the waiting coder finally acquire both dongles and begin. So the system is safe if and only if `time_to_burnout > time_to_compile_6 + cooldown_B + time_to_compile_1 + cooldown_A + time_to_compile_2`.
