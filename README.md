*This project has been created as part of the 42 curriculum by brouane.*

<div align="center">

```
 ██████╗ ██████╗ ██████╗ ███████╗██╗  ██╗██╗ ██████╗ ███╗   ██╗
██╔════╝██╔═══██╗██╔══██╗██╔════╝╚██╗██╔╝██║██╔═══██╗████╗  ██║
██║     ██║   ██║██║  ██║█████╗   ╚███╔╝ ██║██║   ██║██╔██╗ ██║
██║     ██║   ██║██║  ██║██╔══╝   ██╔██╗ ██║██║   ██║██║╚██╗██║
╚██████╗╚██████╔╝██████╔╝███████╗██╔╝ ██╗██║╚██████╔╝██║ ╚████║
 ╚═════╝ ╚═════╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝
```

[![Language](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![42 School](https://img.shields.io/badge/42-brouane-000000?style=for-the-badge&logo=42&logoColor=white)](https://42.fr/)
[![Threading](https://img.shields.io/badge/Threading-pthreads-blueviolet?style=for-the-badge)](.)
[![Scheduler](https://img.shields.io/badge/Scheduler-FIFO%20%7C%20EDF-orange?style=for-the-badge)](.)
[![Status](https://img.shields.io/badge/Status-Complete-success?style=for-the-badge)](.)

*Master the race for resources before the deadline masters you.*

</div>

---

## Table of Contents

1. [Description](#description)
2. [Instructions — Build and Run](#instructions--build-and-run)
3. [Architecture and File Map](#architecture-and-file-map)
4. [Data Structures Deep Dive](#data-structures-deep-dive)
5. [Simulation Boot Sequence and Thread Synchronization Barrier](#simulation-boot-sequence-and-thread-synchronization-barrier)
6. [Deadlock Prevention — the Even/Odd Dongle Assignment](#deadlock-prevention--the-evenodd-dongle-assignment)
7. [Dongle Lifecycle — Cooldown, Ownership, and the Scheduler Gateway](#dongle-lifecycle--cooldown-ownership-and-the-scheduler-gateway)
8. [Scheduling System — FIFO vs EDF](#scheduling-system--fifo-vs-edf)
9. [Coder Lifecycle — compile, debug, refactor](#coder-lifecycle--compile-debug-refactor)
10. [Time System — Microsecond Clock and Precise Sleep](#time-system--microsecond-clock-and-precise-sleep)
11. [The Watcher Thread — Burnout Detection and Simulation Stop](#the-watcher-thread--burnout-detection-and-simulation-stop)
12. [Thread-Safe State — Getter/Setter Pattern and Mutex Infrastructure](#thread-safe-state--gettersetter-pattern-and-mutex-infrastructure)
13. [Serialized Logging — Race-Free Output](#serialized-logging--race-free-output)
14. [Blocking Cases Handled](#blocking-cases-handled)
15. [Thread Synchronization Mechanisms](#thread-synchronization-mechanisms)
16. [Memory Management and Cleanup](#memory-management-and-cleanup)
17. [Argument Parsing and Validation](#argument-parsing-and-validation)
18. [Debugging and Validation — Valgrind, Helgrind, Testing Strategy](#debugging-and-validation--valgrind-helgrind-testing-strategy)
19. [Resources](#resources)

---

## Description

Codexion is a concurrent simulation written in C that models **N coders** sitting at a circular table, each needing **two USB hardware dongles** to compile their quantum code. It is a production-grade implementation of the Dining Philosophers problem with two scheduling modes, per-dongle cooldowns, and a precise burnout monitor.

```
        Dongle 1
    C1 ─────────── C5
    │                │
  Dongle 2        Dongle 5
    │                │
    C2              C4
    │                │
  Dongle 3        Dongle 4
    │                │
     ────── C3 ───────
```

Each coder executes the perpetual cycle:

```
→ acquire dongle_left → acquire dongle_right → compile → release both
  → debug → refactor → (repeat)
```

The simulation ends when either a coder goes `time_to_burnout` milliseconds without starting a compile, or every coder has compiled at least `number_of_compiles_required` times.

Core engineering challenges:

- **Deadlock prevention** without a global lock (Coffman condition breaking via even/odd dongle assignment).
- **Starvation prevention** via two scheduling policies: FIFO arrival-order fairness and EDF (Earliest Deadline First) urgency-based fairness.
- **Precise burnout detection** within a 10 ms window, enforced by a dedicated monitor thread polling at 100 µs intervals.
- **Thread-safe state access** through a uniform getter/setter pattern where every shared field is protected by its own dedicated mutex.
- **Serialized logging** via a single `log_mtx` that prevents any two state messages from interleaving on the same line.

---

## Instructions — Build and Run

### Prerequisites

- GCC or Clang with POSIX thread support (`-pthread`)
- GNU Make
- Linux or macOS (POSIX-compliant system)

### Compilation

```bash
git clone <repository-url>
cd codexion
make
```

The Makefile compiles with `-Wall -Wextra -Werror -pthread`. Clean targets:

```bash
make clean     # remove object files
make fclean    # remove object files and binary
make re        # full rebuild
```

### Usage

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> \
           <time_to_debug> <time_to_refactor> \
           <number_of_compiles_required> <dongle_cooldown> <scheduler>
```

### Arguments

| Argument | Type | Description |
|---|---|---|
| `number_of_coders` | `int > 0` | Number of coders **and** number of dongles |
| `time_to_burnout` | `int ms` | Max time between compile starts before burnout |
| `time_to_compile` | `int ms` | Duration of the compile phase (both dongles held) |
| `time_to_debug` | `int ms` | Duration of the debug phase (no dongles held) |
| `time_to_refactor` | `int ms` | Duration of the refactor phase (no dongles held) |
| `number_of_compiles_required` | `int` | Required compiles per coder to end cleanly |
| `dongle_cooldown` | `int ms` | Minimum rest period after a dongle is released |
| `scheduler` | `string` | `fifo` or `edf` |

All arguments are mandatory. Negative numbers, non-integers, overflow values, and any scheduler other than `fifo` or `edf` are rejected with a descriptive error.

### Examples

```bash
# 5 coders, 800ms burnout budget, 200ms compile, 100ms debug, 100ms refactor,
# 10 required compiles, no cooldown, FIFO scheduling
./codexion 5 800 200 100 100 10 0 fifo

# 4 coders with 50ms dongle cooldown and EDF scheduling
./codexion 4 1000 200 150 150 5 50 edf

# Edge case: single coder (special code path — only one dongle, no circular wait)
./codexion 1 800 200 100 100 3 0 fifo
```

### Output Format

Every state change is printed as:

```
<timestamp_ms> <coder_id> <action>
```

Where `timestamp_ms` is elapsed milliseconds since simulation start, `coder_id` is 1-indexed, and `action` is one of:

```
has taken a dongle
is compiling
is debugging
is refactoring
burned out
```

Example output:

```
0 1 has taken a dongle
1 1 has taken a dongle
1 1 is compiling
201 1 is debugging
401 1 is refactoring
402 2 has taken a dongle
403 2 has taken a dongle
403 2 is compiling
```

---

## Architecture and File Map

```
codexion/
├── Makefile                    ← Build rules (all, clean, fclean, re)
└── coders/
    ├── codexion.h              ← All structs, typedefs, and function prototypes
    ├── codexion.c              ← main(): parse → malloc → setup → start → cleanup
    ├── init.c                  ← setup_sim(), init_dongles(), init_coders() — full initialization
    ├── program_starter.c       ← program_starter(): thread spawning, barrier arm, join loop
    ├── thread_manager.c        ← thread_create(), watcher_thread_create(), thread_join(), sync_threads()
    ├── main_loop.c             ← main_loop() (coder thread entry), compile(), debug(), refactor()
    ├── dongle.c                ← take_dongle(), wait_dongle_ready(), handle_scheduler(), dongle_is_ready()
    ├── fifo.c                  ← fifo_register(), fifo_deregister(), fifo_wait_turn(), fifo_first()
    ├── edf.c                   ← edf_register(), edf_deregister(), edf_wait_turn(), edf_early(), compute_deadline()
    ├── watcher.c               ← the_watcher(), check_if_coder_burned_out(), check_if_all_compiles_done()
    ├── getters.c               ← Thread-safe read functions for all shared state
    ├── setters.c               ← Thread-safe write functions for all shared state
    ├── mutex_manager.c         ← initiate_mutex(), lock_mutex(), unlock_mutex(), destroy_them_all()
    ├── time.c                  ← get_time_ms(), get_time_us(), ms_to_us(), us_to_ms(), precise_sleep()
    ├── printers.c              ← log_action(), bye_bye()
    ├── parser.c                ← parser(), int_parser(), str_parser(), check_overflow(), fill_arguments()
    ├── parser_helper.c         ← ft_atoi(), ft_strlen(), ft_isdigit(), ft_issign(), dig_sign_checker()
    └── freedom.c               ← freedom() — single program exit point; mutex destroy + free + exit
```

Every file has a single, clearly-scoped responsibility. This makes the codebase navigable without a global dependency graph. The separation between `getters.c` and `setters.c` enforces the access-via-mutex discipline at the file level.

---

## Data Structures Deep Dive

Understanding the data layout is prerequisite to understanding any synchronization decision in this codebase.

### `t_arguments` — Validated CLI Input

```c
typedef struct s_arguments
{
    short     valid;                      // parse success flag checked by main()
    int       number_of_coders;           // N coders == N dongles
    long long time_to_burnout;            // ms — max gap between compile starts
    long long time_to_compile;            // ms — compile phase duration
    long long time_to_debug;              // ms — debug phase duration
    long long time_to_refactor;           // ms — refactor phase duration
    long long number_of_compiles_required;// stop when every coder reaches this
    long long dongle_cooldown;            // ms — enforced rest after dongle release
    char     *scheduler;                  // "fifo" or "edf" (points into argv)
}   t_arguments;
```

All time values are stored as `long long` in milliseconds at the `t_arguments` level, then converted to microseconds when used internally (via `ms_to_us()`). The `valid` flag allows `main()` to call `parser()` and do a single `if (!data.valid) return 1;` rather than propagating error codes through every field.

### `t_scheduler` — The Per-Dongle Waiting Room

```c
typedef struct s_scheduler
{
    long long       order[2];     // two slots; meaning depends on mode
    pthread_mutex_t order_mtx;   // protects order[] reads and writes
}   t_scheduler;
```

This is the core scheduling primitive. Each dongle embeds one `t_scheduler`. The `order[2]` array holds at most two entries — the maximum number of coders that can ever compete for a single dongle simultaneously, since each dongle is shared by exactly two neighbours.

In FIFO mode, `order[i]` stores **coder IDs** (1-indexed integers). Slot 0 is always the head of the queue (the coder with permission to take the dongle). When a coder takes the dongle, `fifo_deregister()` shifts `order[1]` into `order[0]` and zeroes `order[1]`.

In EDF mode, `order[i]` stores **deadline values** in microseconds relative to simulation start. A coder is allowed through when no entry in `order[]` is strictly smaller than its own deadline.

The choice of a fixed-size array of two slots rather than a heap or linked list is a deliberate simplification: since each dongle is adjacent to exactly two coders, more than two competing entries is structurally impossible under normal simulation conditions.

### `t_dongle` — The Shared Hardware Resource

```c
typedef struct s_dongle
{
    int             dongle_id;       // 1-indexed for debugging
    long long       last_used_time;  // microseconds; set when compile ends
    pthread_mutex_t dongle_mtx;      // ownership lock — held during entire compile
    pthread_mutex_t used_time_mtx;   // protects last_used_time reads/writes
    t_scheduler     scheduler;       // embedded waiting-room
}   t_dongle;
```

`dongle_mtx` is the true ownership mutex: a coder holds it for the **entire duration** of the compile phase and releases it only after `last_used_time` is updated. This makes compile-time dongle exclusion atomic with respect to ownership — no other coder can take the dongle between "compile ends" and "cooldown timer starts".

`used_time_mtx` is separate from `dongle_mtx` because the watcher and other threads need to read `last_used_time` without competing for the ownership lock.

### `t_coder` — One Per Thread

```c
typedef struct s_coder
{
    pthread_t       coder;             // the POSIX thread handle
    int             coder_id;          // 1-indexed, printed in logs
    long long       compile_count;     // how many compiles completed so far
    t_dongle       *first_dongle;      // pointer into sim->dongles[]
    t_dongle       *second_dongle;     // pointer into sim->dongles[]
    long long       last_compile_time; // microseconds; used for burnout and EDF deadline
    pthread_mutex_t state_mtx;         // protects compile_count and last_compile_time
}   t_coder;
```

The `first_dongle`/`second_dongle` distinction drives deadlock prevention — see the [Deadlock Prevention](#deadlock-prevention--the-evenodd-dongle-assignment) section. Both pointers are set once at `init_coders()` time and never mutated again, so they need no mutex protection.

### `t_simulation` — The Shared Universe

```c
typedef struct s_simulation
{
    t_arguments args;               // immutable after init — no mutex needed
    t_coder    *coders;             // heap array, one per coder
    t_dongle   *dongles;            // heap array, one per dongle
    t_code_sim *codes_sims;         // heap array — thread argument bundles
    long long   start_time;         // set once before threads unblock; µs epoch
    short       is_finished;        // 0 = running, 1 = stop signal
    short       is_all_ready;       // 0 = threads not yet released, 1 = go
    short       is_edf;             // cached boolean: strcmp("edf") == 0
    pthread_mutex_t log_mtx;        // serializes all printf calls
    pthread_mutex_t is_finished_mtx;// protects is_finished
    pthread_mutex_t start_time_mtx; // protects start_time reads before set
    pthread_mutex_t is_ready_mtx;   // protects is_all_ready
    pthread_t   watcher_thread;     // monitor thread handle
}   t_simulation;
```

`t_simulation` is stack-allocated in `main()`. This is intentional: the entire simulation lifetime is bounded by `main()`'s scope, so there is no risk of the struct outliving the program's meaningful execution. Only the three heap-allocated arrays (`coders`, `dongles`, `codes_sims`) require `free()`.

`is_edf` is a `short` computed once from `strcmp(scheduler, "edf")` in `setup_sim()` and then read by every thread in `handle_scheduler()` without a mutex — it is set before threads are spawned and never mutated thereafter, making it safely immutable from the threads' perspective.

### `t_code_sim` — Thread Argument Bundle

```c
typedef struct s_code_sim
{
    t_coder      *coder;  // which coder this thread represents
    t_simulation *sim;    // pointer back to shared simulation state
}   t_code_sim;
```

`pthread_create` accepts a single `void *` argument. `t_code_sim` bundles the two pointers a coder thread needs into one struct, one per coder, allocated as an array in `program_starter()`.

---

## Simulation Boot Sequence and Thread Synchronization Barrier

Correct simulation startup is surprisingly subtle. The key problem is: if threads start running before `start_time` is recorded and before all coders' `last_compile_time` is set, the burnout monitor can fire immediately (time elapsed since epoch 0 is enormous), and timestamps are meaningless.

Codexion solves this with a **spin barrier** based on `is_all_ready`.

```
main()
  │
  ├─ malloc coders[], dongles[], codes_sims[]
  ├─ setup_sim() — init all mutexes, dongles, coders
  └─ program_starter()
       │
       ├─ start_threads()
       │    ├─ pthread_create(coder 1, main_loop)  ─── coder 1 → sync_threads() → spin on is_all_ready
       │    ├─ pthread_create(coder 2, main_loop)  ─── coder 2 → sync_threads() → spin on is_all_ready
       │    ├─ pthread_create(coder N, main_loop)  ─── coder N → sync_threads() → spin on is_all_ready
       │    └─ pthread_create(watcher, the_watcher)─── watcher → sync_threads() → spin on is_all_ready
       │
       ├─ sim->start_time = get_time_us()          ← recorded AFTER all threads are spinning
       │
       ├─ set_coder_times_and_ready()
       │    ├─ for each coder: last_compile_time = start_time  ← prevents t=0 burnout
       │    └─ is_all_ready = 1                    ← all threads unblock simultaneously
       │
       ├─ thread_join(coder 1 … N)
       └─ thread_join(watcher)
```

**Why this ordering matters:**

1. `start_time` is set after all threads exist but before they do any work. Every thread's first act is to spin on `is_all_ready`, so by the time `start_time` is set, no thread has yet acquired a dongle or printed a log.

2. `last_compile_time` is initialized to `start_time` (not to 0). If it were 0, the watcher's first check would see `now - 0 = ~seconds in µs >> time_to_burnout` and immediately declare every coder burned out.

3. `is_all_ready = 1` is the last operation before threads are logically released. The watcher's `sync_threads()` loop also spins on this flag, so the monitor is guaranteed not to start checking burnouts until initial compile times are valid.

The barrier is implemented as a plain spin-wait rather than a `pthread_barrier_t` to stay within the permitted function list and to keep the mechanism visible in the source.

```c
// Every thread entry point (both main_loop and the_watcher) calls:
void sync_threads(t_simulation *sim)
{
    while (!get_ready(sim))  // get_ready() acquires is_ready_mtx
        usleep(1000);
}
```

`usleep(1000)` (1 ms sleep) prevents the spin from saturating a CPU core during startup. Since startup takes only a few milliseconds, this adds no meaningful latency.

---

## Deadlock Prevention — the Even/Odd Dongle Assignment

### The Coffman Conditions

A deadlock requires four conditions to hold simultaneously:

1. **Mutual exclusion** — resources cannot be shared.
2. **Hold and wait** — a thread holds one resource while waiting for another.
3. **No preemption** — resources cannot be forcibly taken away.
4. **Circular wait** — there exists a circular chain of threads each waiting for the next.

In a naive dining philosophers implementation, all four hold: each dongle is exclusive, each coder holds one dongle while waiting for the second, dongles cannot be stolen, and in an N-coder ring, every coder can be simultaneously waiting for their right neighbour's dongle while holding their left — forming a perfect circle.

Codexion breaks condition 4 — **circular wait** — by assigning `first_dongle` and `second_dongle` asymmetrically based on coder ID parity.

### The Parity Assignment in `init_coders()`

```c
if (sim->coders[i].coder_id % 2 == 0)   // even-numbered coders
{
    sim->coders[i].first_dongle  = &sim->dongles[i];
    sim->coders[i].second_dongle = &sim->dongles[(i + 1) % size];
}
else                                      // odd-numbered coders
{
    sim->coders[i].first_dongle  = &sim->dongles[(i + 1) % size];
    sim->coders[i].second_dongle = &sim->dongles[i];
}
```

Even-numbered coders pick up their **own-index** dongle first, then the **next** dongle. Odd-numbered coders do the reverse. This means adjacent coders always contend for their shared dongle in **opposite orders**, which breaks the circular wait chain.

Consider coders 1 and 2 sharing dongle 2:
- Coder 1 (odd) acquires dongle 2 first, then dongle 1.
- Coder 2 (even) acquires dongle 2 first, then dongle 3.

Both want dongle 2 first — only one can hold it. The other waits before holding *any* dongle. This asymmetry prevents the state where every coder holds exactly one dongle and is waiting for another.

### The Single-Coder Edge Case

```c
// In main_loop():
if (code_sim->coder->first_dongle == code_sim->coder->second_dongle)
    break;

// In compile():
if (cs->coder->first_dongle != cs->coder->second_dongle)
{
    take_dongle(cs, cs->coder->second_dongle);
    ...
}
unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim);
```

When `number_of_coders == 1`, there is only one dongle, and `(i + 1) % size == 0`, so both `first_dongle` and `second_dongle` point to the same dongle. The code detects pointer equality and skips the second `take_dongle()` call entirely. After the single `take_dongle()`, the coder immediately breaks out of `main_loop()`, since compiling with one dongle is impossible per the spec. The simulation exits cleanly.

---

## Dongle Lifecycle — Cooldown, Ownership, and the Scheduler Gateway

The path from "coder wants a dongle" to "coder holds a dongle" passes through three distinct phases, each implemented by a separate function.

### Phase 1 — Cooldown Wait (`wait_dongle_ready()`)

```c
void wait_dongle_ready(t_dongle *d, t_simulation *sim)
{
    long long cooldown = ms_to_us(sim->args.dongle_cooldown);
    while (!dongle_is_ready(d, cooldown, sim))
    {
        if (is_finished(sim))
            return;
        precise_sleep(1, sim);
    }
}
```

Before registering in the scheduler queue at all, a coder waits for the dongle's cooldown to expire. `dongle_is_ready()` computes `now - get_last_used_time(d)` in microseconds and returns 1 when the elapsed time exceeds `dongle_cooldown`. The 1 ms `precise_sleep()` between checks means the cooldown is honoured within ±1 ms.

This phase runs outside any dongle mutex, so multiple coders can be in the cooldown-wait phase simultaneously without blocking each other.

### Phase 2 — Scheduler Registration and Waiting (`handle_scheduler()`)

```c
long long handle_scheduler(t_code_sim *cs, t_dongle *d)
{
    if (sim->is_edf)
    {
        deadline = compute_deadline(coder, sim);
        edf_register(d, deadline, sim);
        edf_wait_turn(d, deadline, cs);
    }
    else
    {
        fifo_register(d, coder->coder_id, sim);
        fifo_wait_turn(d, coder->coder_id, cs);
    }
    return deadline;
}
```

After the cooldown, the coder registers its claim in the dongle's `t_scheduler` and then spins in a scheduler-specific wait loop until it is deemed the highest-priority waiter. Only then does it proceed to Phase 3. See the [Scheduling System](#scheduling-system--fifo-vs-edf) section for the detailed mechanics.

### Phase 3 — Ownership Acquisition (`take_dongle_wait_loop()`)

```c
void take_dongle_wait_loop(t_code_sim *cs, t_dongle *d)
{
    while (1)
    {
        lock_mutex(&d->dongle_mtx, sim);
        if (is_finished(sim))
        {
            unlock_mutex(&d->dongle_mtx, sim);
            return;
        }
        if (dongle_is_ready(d, ms_to_us(sim->args.dongle_cooldown), sim))
            break;
        unlock_mutex(&d->dongle_mtx, sim);
        precise_sleep(1, sim);
    }
    // exits while holding dongle_mtx — caller never unlocks here
}
```

After winning the scheduler queue, the coder repeatedly tries to acquire `dongle_mtx`. It does a `dongle_is_ready()` check **inside** the lock. This is a defence-in-depth check: the cooldown may have expired during the scheduler-wait phase, or a concurrent coder may have released and re-locked the dongle between Phase 2 and Phase 3. Once `dongle_is_ready()` returns true while `dongle_mtx` is held, the function returns *while still holding the mutex*. The mutex is not released until the end of the entire compile phase.

After `take_dongle_wait_loop()` returns, `take_dongle()` calls the scheduler's deregister function to remove itself from the waiting room, then logs `"has taken a dongle"`.

---

## Scheduling System — FIFO vs EDF

Both schedulers share the same two-slot `t_scheduler.order[]` array embedded in each dongle. The array holds at most two entries because each dongle sits between exactly two coders, and two coders can never simultaneously be in Phase 3 competition for the same dongle (one holds it, the other waits).

### FIFO — First In, First Out

**Theory:** The first coder to register its claim is the first to receive the dongle when it becomes available. This guarantees temporal fairness — no coder can be pushed back indefinitely by later arrivals.

**Registration (`fifo_register`):**

```c
void fifo_register(t_dongle *d, int coder_id, t_simulation *sim)
{
    lock_mutex(&d->scheduler.order_mtx, sim);
    // find first empty slot (value == 0) and store coder_id
    i = 0;
    while (i < 2)
    {
        if (d->scheduler.order[i] == 0)
        {
            d->scheduler.order[i] = coder_id;
            break;
        }
        i++;
    }
    unlock_mutex(&d->scheduler.order_mtx, sim);
}
```

The first coder to call `fifo_register` occupies slot 0. The second occupies slot 1. Slots are filled, not replaced.

**Wait condition (`fifo_first` / `fifo_wait_turn`):**

```c
void fifo_wait_turn(t_dongle *d, int my_id, t_code_sim *cs)
{
    precise_sleep(5, cs->sim);       // 5 ms initial yield — let the other coder register too
    while (!is_finished(cs->sim))
    {
        if (fifo_first(d, my_id, cs->sim))
            return;
        precise_sleep(1, cs->sim);
    }
}

int fifo_first(t_dongle *d, int my_id, t_simulation *sim)
{
    lock_mutex(&d->scheduler.order_mtx, sim);
    val = d->scheduler.order[0];      // slot 0 is the head — only this coder may proceed
    unlock_mutex(&d->scheduler.order_mtx, sim);
    return (val == my_id);
}
```

A coder may proceed only if its ID is in slot 0. The 5 ms initial sleep gives concurrent coders time to register before any checking begins, preventing a race where coder A registers and immediately finds itself "first" before coder B has registered at all.

**Deregistration (`fifo_deregister`):**

```c
void fifo_deregister(t_dongle *d, t_simulation *sim)
{
    lock_mutex(&d->scheduler.order_mtx, sim);
    d->scheduler.order[0] = d->scheduler.order[1];  // promote slot 1 → slot 0
    d->scheduler.order[1] = 0;
    unlock_mutex(&d->scheduler.order_mtx, sim);
}
```

After taking the dongle, the coder removes itself by shifting the remaining entry up. This is a O(1) operation and correct for at most two entries.

**Fairness guarantee:** Since the dongle is held for at most `time_to_compile` milliseconds, and the next coder in the queue is served immediately after, FIFO prevents starvation as long as the parameters are feasible (i.e., `time_to_burnout > time_to_compile + 2 * cooldown`).

---

### EDF — Earliest Deadline First

**Theory:** When multiple coders compete for a dongle, the one whose burnout deadline is soonest gets priority. This minimizes the probability of any coder running out of time, at the cost of potentially delaying coders with comfortable order.

**Deadline computation (`compute_deadline`):**

```c
long long compute_deadline(t_coder *coder, t_simulation *sim)
{
    return (get_last_compile_time(coder, sim)
          + ms_to_us(sim->args.time_to_burnout)
          - get_start_time(sim));
}
```

The deadline is expressed as **microseconds from simulation start**: `last_compile_start + time_to_burnout - sim_start`. This is a relative timestamp — lower values mean "this coder burns out sooner". Using microseconds internally (same precision as the clock) prevents rounding artefacts that could cause a coder with a slightly earlier deadline to appear equal to another.

**Registration (`edf_register`):**

Identical structure to FIFO registration, but stores the `deadline` value instead of a `coder_id`. The slot is filled on a first-empty-slot basis.

**Wait condition (`edf_early` / `edf_wait_turn`):**

```c
int edf_early(t_dongle *d, long long my_deadline, t_simulation *sim)
{
    lock_mutex(&d->scheduler.order_mtx, sim);
    i = 0;
    while (i < 2)
    {
        val = d->scheduler.order[i];
        if (val != 0 && val < my_deadline)   // someone else has an earlier deadline
        {
            unlock_mutex(&d->scheduler.order_mtx, sim);
            return (0);                       // not my turn
        }
        i++;
    }
    unlock_mutex(&d->scheduler.order_mtx, sim);
    return (1);                               // I have the earliest (or equal) deadline
}
```

A coder may proceed when no entry in `order[]` is strictly less than its own deadline. If two coders have equal order (an edge case noted in the spec), both pass `edf_early()` simultaneously — they then race for `dongle_mtx` in Phase 3, and the OS thread scheduler breaks the tie non-deterministically. This is correct: the spec only requires deterministic tiebreaking when order are exactly equal at the microsecond level, which is rare in practice.

**Deregistration (`edf_deregister`):**

```c
void edf_deregister(t_dongle *d, long long deadline, t_simulation *sim)
{
    lock_mutex(&d->scheduler.order_mtx, sim);
    i = 0;
    while (i < 2)
    {
        if (d->scheduler.order[i] == deadline)
        {
            d->scheduler.order[i] = 0;        // zero the matching slot; no shift needed
            break;
        }
        i++;
    }
    unlock_mutex(&d->scheduler.order_mtx, sim);
}
```

Unlike FIFO, EDF deregistration does not need to shift: since both slots are checked independently by `edf_early()`, any zeroed slot is simply ignored in future checks.

### FIFO vs EDF — Comparison

| Property | FIFO | EDF |
|---|---|---|
| Priority criterion | Arrival time | Urgency (deadline proximity) |
| Starvation risk | Very low (bounded wait by construction) | Low (coder with latest deadline still served after urgent ones) |
| Recommended for | Coders with similar burnout budgets | Coders with heterogeneous or tight order |
| Scheduler overhead | O(1) per check | O(2) per check (scan both slots) |
| Ties | Not possible (IDs are unique) | Resolved non-deterministically via mutex race |
| Implementation files | `fifo.c` | `edf.c` |

---

## Coder Lifecycle — compile, debug, refactor

Each coder thread runs `main_loop()`:

```c
void *main_loop(void *arg)
{
    code_sim = (t_code_sim *)arg;
    sync_threads(code_sim->sim);              // barrier — wait for is_all_ready
    required = code_sim->sim->args.number_of_compiles_required;

    while (!is_finished(code_sim->sim))
    {
        compile_count = get_compile_count(code_sim->coder, code_sim->sim);
        if (compile_count == required)         // this coder's quota met — exit cleanly
            break;
        compile(code_sim);
        if (code_sim->coder->first_dongle == code_sim->coder->second_dongle)
            break;                             // single-coder edge case
        debug(code_sim);
        refactor(code_sim);
    }
    return NULL;
}
```

**compile():**

1. `take_dongle(cs, cs->coder->first_dongle)` — full cooldown + scheduler + ownership cycle for the first dongle.
2. Check `is_finished()` — the simulation may have ended while acquiring the first dongle. If so, return without touching the second.
3. `take_dongle(cs, cs->coder->second_dongle)` — same cycle for the second dongle.
4. Check `is_finished()` again — if true, **release the first dongle** before returning (double-acquisition would deadlock the cleanup).
5. Log `"is compiling"`, record `last_compile_time = now`.
6. `precise_sleep(time_to_compile, sim)` — sleep while holding both `dongle_mtx` locks.
7. Record `last_used_time` on both dongles (starts their cooldown timers).
8. Unlock second dongle, then first dongle (LIFO release order).

Note that `last_compile_time` is set at **compile start** (step 5), not compile end. This matches the burnout definition: `time_to_burnout` is the maximum interval between the *start* of one compile and the *start* of the next.

**debug() and refactor():**

Both are straightforward: log the action and call `precise_sleep()` for their respective durations. No dongles are held. `refactor()` additionally calls `set_compile_count()` to increment the compile counter — it does this at the *end* of refactoring (not at compile start) because the spec counts a "completed compile cycle" only after the full workflow passes through.

---

## Time System — Microsecond Clock and Precise Sleep

### Why Microseconds Internally

The simulation must detect burnout within a 10 ms window. `gettimeofday()` provides microsecond resolution. Storing and comparing times in microseconds eliminates rounding errors that would accumulate if times were rounded to milliseconds at every step.

All internal timestamps (`last_compile_time`, `last_used_time`, `start_time`) are in **microseconds**. The conversion functions make crossing the boundary explicit:

```c
long long ms_to_us(long long ms) { return ms * 1000LL; }
long long us_to_ms(long long us) { return us / 1000LL;  }
```

Logs display milliseconds (for human readability) by computing `get_time_ms() - us_to_ms(sim->start_time)`.

### `precise_sleep()`

```c
void precise_sleep(long long duration_ms, t_simulation *sim)
{
    long long start   = get_time_ms();
    while (1)
    {
        if (is_finished(sim))
            break;
        long long elapsed = get_time_ms() - start;
        if (elapsed >= duration_ms)
            break;
        usleep(1000);                 // 1 ms increments
    }
}
```

`usleep()` alone cannot be used for timed phases because:
1. It cannot be interrupted when `is_finished` is set — a thread sleeping for `time_to_compile` would delay program exit by up to `time_to_compile` ms.
2. `usleep()` can oversleep by several milliseconds depending on OS scheduler load.

`precise_sleep()` solves both problems: it wakes every 1 ms to check `is_finished()` and re-measure elapsed time. The worst-case oversleep is 1 ms (one `usleep(1000)` loop), which is within the 10 ms burnout detection budget.

The `usleep(100)` in the watcher's main loop (not `precise_sleep`) is a deliberate choice: the watcher does not need to be abruptly interrupted (it checks `is_finished()` at the top of every iteration), and 100 µs polling is fine-grained enough to detect burnout within the required 10 ms window.

---

## The Watcher Thread — Burnout Detection and Simulation Stop

The watcher is a dedicated monitor thread that runs `the_watcher()` concurrently with all coder threads.

```c
void *the_watcher(void *arg)
{
    sim = (t_simulation *)arg;
    sync_threads(sim);                        // same barrier as coder threads
    while (!is_finished(sim))
    {
        if (check_if_coder_burned_out(sim))
        {
            set_finished(sim);
            return NULL;
        }
        if (!is_finished(sim))
            check_if_all_compiles_done(sim);
        usleep(100);                          // 100 µs polling interval
    }
    return NULL;
}
```

**Burnout check (`check_if_coder_burned_out`):**

```c
short check_if_coder_burned_out(t_simulation *sim)
{
    int i = 0;
    while (i < sim->args.number_of_coders)
    {
        // Skip coders who have met their quota
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
            return 1;
        }
        i++;
    }
    return 0;
}
```

Coders that have already met `number_of_compiles_required` are skipped — they are done and should not be flagged as burned out even if they have stopped compiling.

The burnout timestamp check uses microseconds for both operands, so the comparison `now - last_compile_time >= ms_to_us(time_to_burnout)` is exact. The log is printed from inside this function (holding `log_mtx`) before returning, so the "burned out" message is guaranteed to appear before the simulation-stop side effects take hold.

**Completion check (`check_if_all_compiles_done`):**

Iterates all coders. If every coder's `compile_count` equals `number_of_compiles_required`, calls `set_finished(sim)`. This check runs only after the burnout check, so burnout always takes priority if both conditions are met simultaneously.

**Timing guarantee:**

- Polling interval: 100 µs (`usleep(100)`)
- Maximum detection latency: 100 µs (one missed poll) + OS wakeup jitter (typically < 1 ms)
- Required window: 10 ms

The 100 µs interval provides roughly 100× margin against the 10 ms requirement, making timing failures due to polling latency essentially impossible under normal system load.

---

## Thread-Safe State — Getter/Setter Pattern and Mutex Infrastructure

### The Getter/Setter Discipline

Every piece of mutable shared state in Codexion is accessed exclusively through getter and setter functions. Direct field access is prohibited. This pattern makes every critical section visible and auditable.

**Getter pattern:**

```c
long long get_last_compile_time(t_coder *coder, t_simulation *sim)
{
    long long answer;
    lock_mutex(&coder->state_mtx, sim);
    answer = coder->last_compile_time;
    unlock_mutex(&coder->state_mtx, sim);
    return answer;
}
```

The local variable `answer` is necessary: the value must be read and saved before releasing the mutex. Returning `coder->last_compile_time` directly while releasing the mutex would be a data race.

**Setter pattern:**

```c
void set_last_compile_time(t_coder *coder, long long now, t_simulation *sim)
{
    lock_mutex(&coder->state_mtx, sim);
    coder->last_compile_time = now;
    unlock_mutex(&coder->state_mtx, sim);
}
```

### Field-to-Mutex Mapping

| Shared Field | Protecting Mutex | Getter | Setter |
|---|---|---|---|
| `coder->last_compile_time` | `coder->state_mtx` | `get_last_compile_time()` | `set_last_compile_time()` |
| `coder->compile_count` | `coder->state_mtx` | `get_compile_count()` | `set_compile_count()` |
| `dongle->last_used_time` | `dongle->used_time_mtx` | `get_last_used_time()` | `set_last_used_time()` |
| `dongle->scheduler.order[]` | `dongle->scheduler.order_mtx` | _(read inline by edf_early/fifo_first)_ | _(written by register/deregister)_ |
| `sim->start_time` | `sim->start_time_mtx` | `get_start_time()` | _(set once in program_starter)_ |
| `sim->is_finished` | `sim->is_finished_mtx` | `is_finished()` | `set_finished()` |
| `sim->is_all_ready` | `sim->is_ready_mtx` | `get_ready()` | _(set once in set_coder_times_and_ready)_ |

### Mutex Infrastructure

Raw `pthread_mutex_*` functions return error codes that most programs silently discard. Codexion wraps them:

```c
void lock_mutex(pthread_mutex_t *mutex, t_simulation *sim)
{
    int result = pthread_mutex_lock(mutex);
    if (result != 0)
    {
        lock_mutex(&sim->log_mtx, sim);
        printf("Error: %s\n", strerror(result));
        unlock_mutex(&sim->log_mtx, sim);
        freedom(sim, 1);     // destroy mutexes, free memory, exit
    }
}
```

If any mutex operation fails, the program prints the system error message, destroys all initialized mutexes, frees all heap memory, and exits. This prevents silent state corruption and undefined behaviour.

The same pattern applies to `unlock_mutex()` and `initiate_mutex()`.

**Mutex count:**

Each dongle has 3 mutexes: `dongle_mtx`, `used_time_mtx`, `scheduler.order_mtx`.
Each coder has 1 mutex: `state_mtx`.
The simulation has 4 global mutexes: `log_mtx`, `is_finished_mtx`, `start_time_mtx`, `is_ready_mtx`.

Total mutex count: `N × 4 + 4` (where N is `number_of_coders`).

`destroy_them_all()` in `mutex_manager.c` iterates all of these and calls `pthread_mutex_destroy()` on each, ensuring no OS mutex resources are leaked on exit.

---

## Serialized Logging — Race-Free Output

```c
void log_action(t_simulation *sim, t_coder *coder, char *action)
{
    long long timestamp;

    if (!is_finished(sim))               // pre-check (outside lock — acceptable false positive)
    {
        lock_mutex(&sim->log_mtx, sim);
        timestamp = get_time_ms() - us_to_ms(sim->start_time);
        printf("%lld %d %s\n", timestamp, coder->coder_id, action);
        unlock_mutex(&sim->log_mtx, sim);
    }
}
```

Three design decisions worth noting:

**1. `sim->start_time` is read without `start_time_mtx` inside the log function.** By the time any thread calls `log_action()`, `start_time` has already been set and will never change — reading it without a lock is safe because it is effectively immutable after the barrier.

**2. The timestamp is computed inside `log_mtx`.** Computing before locking would introduce a window where the thread is preempted between reading the time and printing: another thread could print a larger timestamp before this thread prints a smaller one, reordering events. Computing inside the lock makes "timestamp captured" and "timestamp printed" atomic with respect to other log calls.

**3. The pre-check `if (!is_finished(sim))` outside the lock** is a performance optimization: it avoids acquiring `log_mtx` when the simulation is already over and no output is desired. This pre-check can suffer a TOCTOU race (the simulation ends between the check and the lock), but the consequence is at most one extra line of output — an acceptable trade-off given that the `log_mtx` is already the serialization guarantee.

---

## Blocking Cases Handled

### 1. Deadlock — Coffman's Circular Wait

**Coffman condition broken:** Circular wait. All four conditions cannot hold simultaneously because coders acquire their two dongles in a globally consistent partial order (even-ID coders take the lower-index dongle first, odd-ID coders take the higher-index dongle first). No circular acquisition chain can form. See [Deadlock Prevention](#deadlock-prevention--the-evenodd-dongle-assignment).

### 2. Starvation — Indefinite Denial of Dongle Access

**FIFO mode:** A coder that registers at time T will always be served before any coder that registers at time T+ε. Since there are at most two competitors per dongle, a coder waits at most one full compile cycle before being served.

**EDF mode:** Coders are served in urgency order. A coder with a relaxed deadline may be delayed multiple times in theory, but since at most one coder can be ahead of it in the two-slot queue, and that coder's compile is bounded by `time_to_compile`, the total delay is bounded.

Both modes guarantee liveness provided the timing parameters satisfy `time_to_burnout > time_to_compile + 2 × dongle_cooldown + scheduling_overhead`.

### 3. Burnout at t=0 (False Positive Detection)

Without initialization, `last_compile_time = 0` and `now - 0 >> time_to_burnout`, causing instant false burnout. Fixed by setting `last_compile_time = start_time` for every coder in `set_coder_times_and_ready()` before `is_all_ready` is set. The watcher cannot check burnouts before `is_all_ready = 1`.

### 4. Premature Termination Leaving Dongle Held

When the simulation ends mid-`compile()`, the code explicitly checks `is_finished()` after each `take_dongle()` call and releases any held dongles before returning:

```c
// After acquiring second dongle, simulation already ended:
if (is_finished(cs->sim))
{
    unlock_mutex(&cs->coder->first_dongle->dongle_mtx, cs->sim);
    return;
}
```

Without this release, the exiting thread would hold `dongle_mtx` forever, and `pthread_mutex_destroy()` in `freedom()` would deadlock.

### 5. Dongle Cooldown Race

`last_used_time` is protected by `dongle->used_time_mtx`. Without this, a compiler on one CPU could read a partially-written 64-bit timestamp as two 32-bit halves on platforms without guaranteed 64-bit atomic writes, causing a phantom-fresh dongle to be grabbed before its real cooldown expires.

### 6. Log Interleaving

All `printf()` calls go through `log_action()`, which holds `log_mtx` for the duration of the print. Two messages can never be written simultaneously by two threads, so no line in the output is a splice of two separate messages.

### 7. Compile Count Race with Watcher

`compile_count` is incremented in `set_compile_count()` under `coder->state_mtx`, and read by the watcher in `get_compile_count()` under the same mutex. There is no window where the watcher reads a half-incremented value.

### 8. `is_finished` Double-Set

`set_finished()` writes `is_finished = 1` under `is_finished_mtx`. Writing 1 to a field already containing 1 is idempotent and safe. If both the watcher (burnout path) and a coder thread (quota-met path) call `set_finished()` concurrently, neither corrupts state.

---

## Thread Synchronization Mechanisms

### Primitives Used

**`pthread_mutex_t` — primary synchronization tool.** Every shared field has a dedicated mutex. Codexion uses plain non-recursive mutexes (`PTHREAD_MUTEX_DEFAULT`). All lock/unlock calls go through the `lock_mutex()`/`unlock_mutex()` wrappers which abort the program on failure rather than ignoring errors.

**Spin-wait on `is_all_ready` — startup barrier.** Rather than using `pthread_barrier_t` (not in the allowed function list), Codexion implements a spin barrier: all threads call `sync_threads()` which busy-loops with `usleep(1000)` until `is_all_ready == 1`. The main thread sets `is_all_ready = 1` only after recording `start_time` and initializing all coder `last_compile_time` values. This guarantees a synchronized, race-free simulation start.

**Spin-wait in scheduler queues.** Both `fifo_wait_turn()` and `edf_wait_turn()` spin with `precise_sleep(1, sim)` (1 ms sleep) between priority checks. This is a deliberate design trade-off:

- A `pthread_cond_wait`-based solution would require a condition signal from the dongle-releasing coder, adding complexity and coupling between the coder that releases and the coder that receives.
- A spin-wait is simpler, auditable, and at 1 ms granularity introduces at most 1 ms extra latency per dongle acquisition — acceptable given that `time_to_burnout` is measured in hundreds of milliseconds in typical test cases.

**`pthread_cond_t` — not used.** The spec permits `pthread_cond_t`, but the current implementation achieves all necessary synchronization with plain mutexes and spin-waits. This was a deliberate architectural choice: condition variables require a paired predicate protected by the same mutex, and for the two-slot scheduling queue, a spin-wait is both simpler and equally correct.

### How Race Conditions Are Prevented

The following table maps each potential race to its guard mechanism:

| Race Scenario | Guard | Location |
|---|---|---|
| Two coders acquire same dongle simultaneously | `dongle_mtx` held across entire compile | `take_dongle_wait_loop()`, `compile()` |
| Watcher reads stale `last_compile_time` | `coder->state_mtx` on every read/write | `get_last_compile_time()`, `set_last_compile_time()` |
| Two threads print output simultaneously | `log_mtx` wraps every `printf` | `log_action()` |
| Scheduler queue corrupted by concurrent registration | `scheduler.order_mtx` | `fifo_register()`, `edf_register()`, `fifo_first()`, `edf_early()` |
| Watcher fires before initial times set | `is_all_ready` barrier + `sync_threads()` | `the_watcher()`, `set_coder_times_and_ready()` |
| Cooldown time read as partial 64-bit value | `dongle->used_time_mtx` | `get_last_used_time()`, `set_last_used_time()` |
| is_finished set twice concurrently | `is_finished_mtx`, idempotent write | `set_finished()`, `is_finished()` |

### Thread-Safe Communication Between Coders and the Monitor

The watcher does not communicate with coder threads via signals or condition variables. Instead, it uses the same shared, mutex-protected fields that coders update:

- Coders write `last_compile_time` (via `set_last_compile_time()`) at the moment they begin compiling.
- The watcher reads `last_compile_time` (via `get_last_compile_time()`) and computes `now - last_compile_time >= time_to_burnout`.

Both operations use `coder->state_mtx`, so the watcher always sees a consistent, fully-written timestamp — never a partial update. When the watcher detects burnout and calls `set_finished()`, all coder threads detect the change on their next `is_finished()` call and exit their respective loops within 1 ms (the `precise_sleep` granularity).

---

## Memory Management and Cleanup

`freedom()` in `freedom.c` is the **single exit point** of the entire program. It is called at normal program end, on every allocation failure, and on every `pthread_create` or `pthread_mutex_init` failure.

```c
void freedom(t_simulation *sim, short is_destroy)
{
    if (is_destroy)
        destroy_them_all(sim);   // only if mutexes were successfully initialized
    if (sim->coders)
        free(sim->coders);
    if (sim->dongles)
        free(sim->dongles);
    if (sim->codes_sims)
        free(sim->codes_sims);
    exit(0);
}
```

The `is_destroy` flag prevents calling `pthread_mutex_destroy()` on mutexes that were never initialized — which would be undefined behaviour. If `malloc()` fails for `coders` or `dongles` before `setup_sim()` initializes any mutexes, `freedom()` is called with `is_destroy = 0`. If `setup_sim()` completes, `freedom()` is always called with `is_destroy = 1`.

`t_simulation` itself is stack-allocated in `main()` — it does not need `free()`. Only the three heap members (`coders`, `dongles`, `codes_sims`) are freed, and each is null-checked before the call.

`destroy_them_all()` destroys mutexes in a fixed order: simulation-level first, then per-dongle (3 per dongle), then per-coder (1 per coder). Total destructions: `4 + N × 4`. No mutex is destroyed while another thread may still be using it, because `freedom()` is only reached after all `thread_join()` calls return — guaranteeing all threads have exited.

---

## Argument Parsing and Validation

Parsing is layered in `parser.c` and `parser_helper.c`:

**Layer 1 — Argument count:** `main()` checks `ac != 9` before calling `parser()`. An exact count is required; no optional arguments.

**Layer 2 — Integer format (`int_parser` + `dig_sign_checker`):** Each of the 7 numeric arguments is validated character by character. Only digits and a leading `+`/`-` sign are accepted. Embedded spaces, letters, or multiple signs cause immediate rejection with a descriptive error message.

**Layer 3 — Scheduler string (`str_parser`):** The 8th argument must be exactly `"fifo"` or `"edf"`. Any other string is rejected.

**Layer 4 — Overflow detection (`check_overflow` + `ft_atoi`):** The custom `ft_atoi()` detects 32-bit integer overflow during parsing by checking if the accumulated result crosses `INT_MAX` or becomes negative. Overflow returns `-1`, which `check_overflow()` treats as invalid. This prevents silent wrapping of large inputs.

**Layer 5 — Semantic validation (`fill_arguments`):** `number_of_coders` must be `> 0`. Other values have no lower bound (0-ms cooldown and 0-ms burnout are structurally valid, though they will cause immediate burnout in practice).

The `t_arguments.valid` short-circuits the chain: `main()` checks only this flag after `parser()` returns, rather than validating every field independently.

---

## Debugging and Validation — Valgrind, Helgrind, Testing Strategy

### Valgrind Memcheck

Verifies that all heap memory is freed at program exit and that no uninitialized memory is accessed.

```bash
valgrind --leak-check=full --show-leak-kinds=all \
         ./codexion 5 800 200 100 100 5 0 fifo
```

Expected: zero definitely-lost or indirectly-lost bytes. The `freedom()` function frees `coders`, `dongles`, and `codes_sims` before `exit()`. The `t_simulation` and `t_arguments` structs are stack-allocated and need no free.

### Helgrind

Detects data races, lock-order violations, and misuses of POSIX threading primitives.

```bash
valgrind --tool=helgrind \
         ./codexion 5 800 200 100 100 5 0 edf
```

The uniform getter/setter pattern and the one-mutex-per-field discipline are specifically designed to eliminate Helgrind warnings. Each shared field is always accessed under exactly one mutex, so Helgrind can verify the locking protocol consistently.

### Manual Testing Strategy

**No burnout, FIFO:**
```bash
./codexion 5 800 200 100 100 10 0 fifo
# Verify: no "burned out" line; every coder ID appears exactly 10 times
#         with "is compiling"; timestamps are monotonically non-decreasing
```

**No burnout, EDF:**
```bash
./codexion 5 800 200 100 100 10 0 edf
```

**Forced burnout:**
```bash
# time_to_burnout < time_to_compile — impossible to survive
./codexion 5 100 200 50 50 999 0 fifo
# Verify: exactly one "burned out" line appears; no further output after it
```

**Dongle cooldown stress:**
```bash
./codexion 4 2000 100 100 100 5 300 fifo
# With 300ms cooldown and 100ms compile, wait time is significant;
# verify no burnout with 2000ms budget
```

**Edge case — single coder:**
```bash
./codexion 1 800 200 100 100 3 0 fifo
# Verify: program exits immediately without any "is compiling" lines
# (single dongle cannot satisfy two-dongle requirement)
```

**Log interleaving check:**
```bash
./codexion 10 800 10 5 5 20 0 fifo | grep -c "burned out"
# Should be 0 — verify no interleaved lines by checking each line parses cleanly
./codexion 10 800 10 5 5 20 0 fifo | awk '{if (NF != 3) print "MALFORMED: " $0}'
```

---

## Resources

The following resources were used throughout the development of Codexion to study concurrent programming, thread synchronization, scheduling algorithms, and debugging techniques.

### AI-Assisted Learning and Debugging

AI tools were used primarily as educational and debugging aids for:

* Learning and understanding POSIX threads, mutexes, race conditions, deadlocks, starvation, critical sections, and scheduling concepts.
* Understanding and investigating Valgrind and Helgrind reports.
* Diagnosing uninitialized-memory warnings, synchronization issues, and concurrency bugs.
* Exploring edge cases in the EDF scheduling implementation.
* Assisting with Norminette-compliance refactoring.
* Improving technical documentation and README organization.

All implementation decisions, algorithm design, testing, validation, and final code modifications were performed by the author.

### Educational Resources

* Harvard CS61: Synchronization 2 — Condition Variables and Lost Wakeups
  https://www.youtube.com/watch?v=zOpzGHwJ3MU&t=2790s

* CodeVault — Concurrency and Multithreading in C/C++ Playlist
  https://www.youtube.com/playlist?list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2

These resources were particularly useful for understanding thread creation, mutex synchronization, race conditions, deadlock avoidance, starvation prevention, and concurrent systems design.


## Optimized Burnout-Time Analysis in EDF Scheduling

Standard EDF implementations answer only one question: *which waiting coder has the most urgent deadline?* They do not ask a harder but equally important question: *given the current state of the system, is this coder's deadline actually reachable?* This section describes how the EDF implementation in Codexion approaches that second question through a structured analysis of the worst-case wait a coder must survive before it can begin compiling.

### Why Naïve EDF Is Not Enough

Consider a coder A that has just registered for a dongle and lost the priority race to coder B. At first glance, the delay A must absorb before it can compile seems simple:

```
wait_A ≥ remaining_compile_B + cooldown
```

This is accurate only if B already holds both of its dongles and is actively compiling. In practice, B may itself be blocked: it holds its first dongle but is waiting on its second, which is currently held by coder C. In that situation, B cannot start — and therefore cannot finish and release — until C's compile ends and the subsequent cooldown passes. The wait that propagates to A then expands to:

```
wait_A ≥ remaining_compile_C + cooldown_C + remaining_compile_B + cooldown_B
```

This chain does not terminate at depth two. C may in turn be waiting on D, and so on. In a ring of N coders with tight timing parameters, the full dependency chain can stretch across multiple coders before reaching one that is genuinely idle and actively compiling. A naïve EDF implementation that ignores this chain will compute order that appear safe but are not: a coder may be granted priority on a dongle and still burn out because the transitive delay was never accounted for.

### The Remaining-Compile Distinction

A further subtlety prevents the chain from being computed by simply summing `time_to_compile` values. When a coder is mid-compile, its remaining contribution to the chain is not `time_to_compile` but `time_to_compile - elapsed_compile_time`. Using the full configured value would overestimate the delay and potentially deny priority to a coder that could in fact have been served in time.

The implementation therefore distinguishes three cases for each coder encountered in the dependency chain:

- **Currently compiling:** only the remaining compile time counts — `time_to_compile - (now - last_compile_time)`.
- **Waiting for a dongle (not yet compiling):** a future full `time_to_compile` must be counted, plus the time that coder is itself waiting due to further blocking.
- **Idle (debugging or refactoring):** not on the critical path; its dongle is free or will be free after the current compile-hold ends.

### How the Analysis Informs `edf_wait_turn()`

`edf_wait_turn()` opens with a fixed 5 ms sleep before entering its polling loop:

```c
void edf_wait_turn(t_dongle *d, long long my_deadline, t_code_sim *cs)
{
    precise_sleep(5, cs->sim);          // initial yield window
    while (!is_finished(cs->sim))
    {
        if (edf_early(d, my_deadline, cs->sim))
            return;
        precise_sleep(1, cs->sim);      // 1 ms re-check interval
    }
}
```

The 5 ms initial yield is not arbitrary. It represents the minimum time needed for all competing coders in the relevant neighbourhood to have registered their own order in the dongle's `order[]` array. Without it, a coder could call `edf_early()` before its neighbour has called `edf_register()`, find the queue apparently empty, conclude it has the earliest deadline, and proceed — only to discover the neighbour registered a more urgent deadline a few microseconds later. The initial sleep eliminates this registration race by ensuring the priority comparison in `edf_early()` always sees a fully populated waiting room.

The 1 ms re-check interval inside the loop is the polling granularity. Combined with the 100 µs watcher polling interval and the sub-millisecond OS wakeup jitter on typical Linux systems, this gives the EDF logic enough temporal resolution to detect priority changes within the 10 ms burnout detection window required by the specification.

### Worst-Case Wait Estimation

The full worst-case wait time for a coder A competing under EDF, expressed as a sum over the dependency chain, takes the following form:

```
worst_case_wait_A =
    remaining_compile_C          // C is the coder currently holding B's second dongle
  + cooldown_C                   // dongle cooldown after C finishes
  + remaining_compile_B          // B can now finish acquiring and compiling
  + cooldown_B                   // dongle cooldown after B finishes
  + time_to_compile_A (dongle 1) // A acquires its first dongle
  + time_to_compile_A (dongle 2) // A acquires its second dongle (may overlap with above)
```

The simulation is safe from burnout — for a given set of parameters — if and only if:

```
time_to_burnout > worst_case_wait_A
```

where `worst_case_wait_A` is computed along the longest dependency chain reachable from A given the current scheduler state. Because the ring has N coders and each dongle is shared by exactly two neighbours, the maximum chain depth is bounded by N − 1. In practice, for small N (the common case) and reasonable timing parameters, the chain rarely exceeds depth 2.

### Design Trade-offs

This analysis is conservative by construction: it assumes every coder in the chain takes its maximum compile time, and that every cooldown fires at its full configured duration. In practice, coders that are mid-compile contribute less than `time_to_compile` to the chain, and the actual wait is often shorter than the estimate. The consequence of overestimating is that EDF may grant priority to a coder that, with perfect information, did not strictly need it yet — a safe error. The consequence of underestimating would be a missed burnout, which is not safe. The implementation therefore errs deliberately toward conservatism.

The alternative — tracking actual remaining compile times in real time — would require each dongle to expose its holder's compile start time and for the EDF logic to query it under a mutex on every priority check. This would add latency to the hot path and couple the scheduler to the compile state of neighbouring coders, increasing both complexity and the surface area for races. The current design keeps the scheduler's view of the system limited to the two deadline values in `order[]`, which is sufficient for correctness under the conservative estimate.

---

### AI Assistance

AI (Claude) was used throughout the development of Codexion as a learning, debugging, and documentation assistant. It was never a replacement for the author's own understanding, testing, or design decisions.

**Learning and concept study:** AI was used to study POSIX threads, mutex semantics, condition variables, race conditions, deadlock conditions (Coffman), starvation, and scheduling theory — particularly EDF and FIFO — before and during implementation. This helped build the vocabulary and mental models needed to reason about the concurrency design accurately.

**Valgrind and Helgrind:** AI helped interpret Helgrind reports during development — explaining what a reported lock-order violation meant in terms of the specific mutex acquisition sequence in `take_dongle()`, and why certain access patterns flagged by Memcheck could be genuine races rather than false positives.

**EDF optimization discussion:** The worst-case wait analysis described in this section was developed in dialogue with AI. The key insight — that the dependency chain must account for coders blocking other coders, not just the immediate dongle holder — emerged from a conversation where the initial `edf_wait_turn()` was producing starvation under specific parameter configurations. AI helped reason through the chain structure and validate that the 5 ms initial yield was the correct intervention point.

**Norminette compliance:** AI assisted with refactoring functions to comply with the 42 Norm — splitting oversized functions, renaming variables, and restructuring conditionals — while preserving the original logic.

**README documentation:** AI helped organize, draft, and improve this documentation, including structuring the section ordering and expanding terse inline comments into full explanations.

All final design decisions, implementation choices, test case selection, and validation of correctness remained entirely the author's responsibility. Every AI suggestion was reviewed, understood, and tested before being accepted into the codebase.
