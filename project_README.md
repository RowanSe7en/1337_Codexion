*This project has been created as part of the 42 curriculum by brouane.*

---

## Description

Codexion is a concurrent simulation written in C that models **N coders** sitting at a circular table, each needing **two USB hardware dongles** to compile their quantum code. It is a Dining-Philosophers-style concurrency problem, implemented with two selectable scheduling modes, per-dongle cooldowns, and a precise burnout monitor.

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

**Goal.** The project's purpose is to practice real POSIX-thread concurrency: mutual exclusion, deadlock avoidance, starvation prevention, and race-free shared state, without relying on higher-level concurrency primitives beyond `pthread_mutex_t`. The core engineering challenges are:

- **Deadlock prevention** without a global lock: dongles are acquired as an atomic pair (breaking hold-and-wait), reinforced by even/odd dongle-assignment ordering (breaking circular wait).
- **Starvation prevention** via two scheduling policies: FIFO arrival-order fairness and EDF (Earliest Deadline First) urgency-based fairness.
- **Precise burnout detection** within a 10 ms window, enforced by a dedicated monitor thread polling at 500 µs intervals.
- **Thread-safe state access** through a uniform getter/setter pattern where every shared field is protected by its own dedicated mutex.
- **Serialized logging** via a single `log_mtx` that prevents any two state messages from interleaving on the same line.

---

## Instructions

### Prerequisites

- GCC or Clang with POSIX thread support (`-pthread`)
- GNU Make
- Linux or macOS (POSIX-compliant system)

### Compilation

```bash
git clone <repository-url> codexion
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

| Argument | Type | Description |
|---|---|---|
| `number_of_coders` | `int > 0` | Number of coders **and** number of dongles |
| `time_to_burnout` | `int ms` | Max time between compile starts before burnout |
| `time_to_compile` | `int ms` | Duration of the compile phase (both dongles held) |
| `time_to_debug` | `int ms` | Duration of the debug phase (no dongles held) |
| `time_to_refactor` | `int ms` | Duration of the refactor phase (no dongles held) |
| `number_of_compiles_required` | `int >= 1` | Required compiles per coder to end cleanly |
| `dongle_cooldown` | `int ms` | Minimum rest period after a dongle is released |
| `scheduler` | `string` | `fifo` or `edf` |

All arguments are mandatory. `number_of_coders` must be `> 0` and `number_of_compiles_required` must be `>= 1`; the remaining numeric arguments accept `0`. Negative numbers, non-integers, overflow values, and any scheduler other than `fifo` or `edf` are rejected with a descriptive error.

### Examples

```bash
./codexion 5 2000 200 200 200 10 0 fifo
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

### Testing and Validation

**Valgrind Memcheck** — verifies all heap memory is freed and no uninitialized memory is accessed:

```bash
valgrind --leak-check=full --show-leak-kinds=all \
         ./codexion 5 800 200 100 100 5 0 fifo
```

**Helgrind** — detects data races, lock-order violations, and misuse of POSIX threading primitives:

```bash
valgrind --tool=helgrind \
         ./codexion 5 800 200 100 100 5 0 edf
```

The uniform getter/setter pattern and the one-mutex-per-field discipline (see [Thread Synchronization Mechanisms](#thread-synchronization-mechanisms)) are specifically designed to eliminate Helgrind warnings: each shared field is always accessed under exactly one mutex, so Helgrind can verify the locking protocol consistently.

**Manual scenarios worth checking:**

```bash
# No burnout, FIFO — verify no "burned out" line; every coder ID appears
# exactly <number_of_compiles_required> times with "is compiling"
./codexion 5 800 200 100 100 10 0 fifo

# Forced burnout (time_to_burnout < time_to_compile) — verify exactly one
# "burned out" line appears, with no further output after it
./codexion 5 100 200 50 50 999 0 fifo

# Single coder — verify no "is compiling" line ever appears, and the run
# takes the full ~800ms burnout window before exiting (not an instant exit)
./codexion 1 800 200 100 100 3 0 fifo

# Log interleaving check — every line should parse as exactly 3 fields
./codexion 10 800 10 5 5 20 0 fifo | awk '{if (NF != 3) print "MALFORMED: " $0}'
```

---

## Blocking Cases Handled

### 1. Deadlock — Hold-and-Wait and Circular Wait

**Coffman conditions broken:** hold-and-wait and circular wait, independently.

- **Hold-and-wait** is broken by acquiring both dongles as an atomic pair: `try_grab_pair()` only locks a dongle mutex when it can lock both in the same attempt (trylock-only, never blocking), so a coder is never observed holding one dongle while stuck waiting on the other.
- **Circular wait** is additionally broken by a globally consistent partial order: coders acquire their two dongles asymmetrically based on ID parity (even-ID coders take the lower-index dongle first, odd-ID coders take the higher-index dongle first), so no circular acquisition chain can form even in designs that do hold-and-wait.

Either mechanism is independently sufficient to prevent deadlock; Codexion currently has both in place.

```c
int try_grab_pair(t_code_sim *cs, t_dongle *d1, t_dongle *d2)
{
    ...
    if (pthread_mutex_trylock(&d1->dongle_mtx) != 0)
        return (0);
    if (!dongle_is_ready(d1, cooldown, sim)
        || !dongle_is_ready(d2, cooldown, sim))
    {
        unlock_mutex(&d1->dongle_mtx, sim);
        return (0);
    }
    if (pthread_mutex_trylock(&d2->dongle_mtx) != 0)
    {
        unlock_mutex(&d1->dongle_mtx, sim);
        return (0);
    }
    return (1);
}
```

`pthread_mutex_trylock()` never blocks — it either acquires the lock immediately or fails immediately. If it locks `d1` but then can't lock `d2`, it unlocks `d1` again before returning rather than sitting on it.

### 2. Starvation — Indefinite Denial of Dongle Access

**FIFO mode:** A coder that registers at time T will always be served before any coder that registers at time T+ε. Since there are at most two competitors per dongle, a coder waits at most one full compile cycle before being served.

**EDF mode:** Coders are served in urgency order. Since at most one coder can be ahead of it in the two-slot queue, and that coder's compile is bounded by `time_to_compile`, the total delay is bounded.

Both modes guarantee liveness provided the timing parameters leave enough margin between `time_to_burnout` and the combined cost of one compile cycle plus cooldown and scheduling overhead.

### 3. Burnout at t=0 (False Positive Detection)

Without initialization, `last_compile_time = 0` and `now - 0 >> time_to_burnout`, causing instant false burnout. Fixed by setting `last_compile_time = start_time` for every coder in `set_coder_times_and_ready()` before `is_all_ready` is set. The watcher cannot check burnouts before `is_all_ready = 1`.

### 4. Premature Termination Without Leaking a Held Dongle

Because dongles are acquired atomically as a pair, there is no window where a coder holds one dongle and is still waiting on the other — so there is nothing partial to clean up if the simulation ends mid-wait. If `is_finished()` becomes true while a coder is inside `take_dongle_pair()`'s retry loop, the loop simply exits and returns `0` without ever having locked a mutex. If the simulation instead ends *while* a coder is already compiling (both dongles held), `precise_sleep(time_to_compile, sim)` returns early on `is_finished()`, but `compile()` still runs its unconditional cleanup afterward — `set_last_used_time()` on both dongles, then `unlock_mutex()` on both — so both dongles are released either way.

Without either of these guarantees, an exiting thread could hold `dongle_mtx` forever, and `pthread_mutex_destroy()` at cleanup would deadlock.

### 5. Dongle Cooldown Race

`last_used_time` is protected by `dongle->used_time_mtx`. Without this, a coder could read a partially-written 64-bit timestamp as two separate halves on platforms without guaranteed 64-bit atomic writes, causing a phantom-fresh dongle to be grabbed before its real cooldown expires.

### 6. Log Interleaving

All `printf()` calls go through `log_action()`, which holds `log_mtx` for the duration of the print. Two messages can never be written simultaneously by two threads, so no line in the output is a splice of two separate messages.

### 7. Compile Count Race with Watcher

`compile_count` is incremented in `set_compile_count()` under `coder->state_mtx`, and read by the watcher in `get_compile_count()` under the same mutex. There is no window where the watcher reads a half-incremented value.

### 8. `is_finished` Double-Set

`set_finished()` writes `is_finished = 1` under `is_finished_mtx`. Writing 1 to a field already containing 1 is idempotent and safe. If both the watcher (burnout path) and a coder thread (quota-met path) call `set_finished()` concurrently, neither corrupts state.

---

## Thread Synchronization Mechanisms

### Primitives Used

**`pthread_mutex_t` — primary synchronization tool.** Every shared field has a dedicated mutex. Codexion uses plain non-recursive mutexes (`PTHREAD_MUTEX_DEFAULT`). All lock/unlock calls go through `lock_mutex()`/`unlock_mutex()` wrappers which abort the program on failure rather than ignoring errors.

**`pthread_cond_t` — not used.** The spec permits `pthread_cond_t`, but the implementation achieves all necessary synchronization with plain mutexes and spin-waits instead (see below). This was a deliberate choice: condition variables require a paired predicate protected by the same mutex, and for a two-slot scheduling queue shared by exactly two competing coders, a bounded spin-wait is both simpler and equally correct.

**Custom event implementation — spin-wait barrier and retry loops, replacing `pthread_cond_t`:**

- *Startup barrier (`sync_threads()`).* Rather than `pthread_barrier_t` (not in the allowed function list) or a condition variable, all threads busy-loop with `usleep(1000)` until `is_all_ready == 1`. The main thread sets `is_all_ready = 1` only after recording `start_time` and initializing every coder's `last_compile_time`, guaranteeing a synchronized, race-free simulation start.
- *Dongle-pair acquisition (`take_dongle_pair()`).* A coder registers its intent (`register_pair()`), then repeatedly checks `turn_ready_pair() && try_grab_pair()`, sleeping `usleep(500)` between attempts until it succeeds or the simulation ends. This is functionally an event wait ("wake me when I can have both dongles") implemented by polling instead of by a condition-variable signal.

### How Race Conditions Are Prevented

Every shared resource — dongles, the log, and monitor state — is accessed exclusively through getter/setter functions that wrap a lock/read-or-write/unlock sequence, never touched directly:

```c
long long get_last_compile_time(t_coder *coder, t_simulation *sim)
{
    long long answer;
    lock_mutex(&coder->state_mtx, sim);
    answer = coder->last_compile_time;
    unlock_mutex(&coder->state_mtx, sim);
    return answer;
}

void set_last_compile_time(t_coder *coder, long long now, t_simulation *sim)
{
    lock_mutex(&coder->state_mtx, sim);
    coder->last_compile_time = now;
    unlock_mutex(&coder->state_mtx, sim);
}
```

The local variable in the getter is necessary: the value must be read and saved before releasing the mutex — returning the field directly while unlocking would itself be a race.

| Race Scenario | Guard | Location |
|---|---|---|
| Two coders acquire the same dongle simultaneously | `dongle_mtx` held across the entire compile | `try_grab_pair()`, `take_dongle_pair()`, `compile()` |
| Coder holds one dongle while blocked on the other (hold-and-wait) | Atomic pair acquisition — a mutex is only locked when both can be locked in the same attempt (trylock-only, never blocks) | `try_grab_pair()`, `take_dongle_pair()` |
| Monitor reads stale `last_compile_time` | `coder->state_mtx` on every read/write | `get_last_compile_time()`, `set_last_compile_time()` |
| Two threads print output simultaneously | `log_mtx` wraps every `printf` | `log_action()` |
| Scheduler queue corrupted by concurrent registration | `scheduler.order_mtx` | `fifo_register()`, `edf_register()`, `fifo_first()`, `edf_early()` |
| Monitor fires before initial times are set | `is_all_ready` barrier + `sync_threads()` | `the_watcher()`, `set_coder_times_and_ready()` |
| Cooldown timestamp read as a partial 64-bit value | `dongle->used_time_mtx` | `get_last_used_time()`, `set_last_used_time()` |
| `is_finished` set twice concurrently | `is_finished_mtx`, idempotent write | `set_finished()`, `is_finished()` |

Each dongle carries 3 mutexes (`dongle_mtx`, `used_time_mtx`, `scheduler.order_mtx`), each coder carries 1 (`state_mtx`), and the simulation carries 4 global mutexes (`log_mtx`, `is_finished_mtx`, `start_time_mtx`, `is_ready_mtx`) — a total of `N × 4 + 4` for `N` coders, all destroyed on exit by `destroy_them_all()`.

### Thread-Safe Communication Between Coders and the Monitor

The monitor (watcher) thread does not communicate with coder threads via signals or condition variables. Instead, it uses the same shared, mutex-protected fields that coders update:

- Coders write `last_compile_time` (via `set_last_compile_time()`) at the moment they begin compiling.
- The watcher reads `last_compile_time` (via `get_last_compile_time()`) every `500 µs` and computes `now - last_compile_time >= time_to_burnout`.

Both operations use `coder->state_mtx`, so the watcher always sees a consistent, fully-written timestamp — never a partial update. When the watcher detects burnout and calls `set_finished()`, every coder thread detects the change on its next `is_finished()` call and exits its loop within one retry cycle (at most `500 µs` for a coder mid-acquisition, or immediately after its current compile/debug/refactor step).

---

## Resources

### AI-Assisted Learning and Debugging

AI tools were used as educational and debugging aids for specific, bounded tasks — not for open-ended implementation:

* **Concurrency theory** — learning and understanding POSIX threads, mutex semantics, condition variables, Coffman's deadlock conditions, and starvation/fairness theory (FIFO and EDF) ahead of and during implementation.
* **Debugging (`dongle.c`, `edf.c`, `watcher.c`)** — interpreting Valgrind/Helgrind reports and reasoning through synchronization bugs (scheduler slot leaks, unconditional mutex unlocks, recursive-lock errors on the single-coder edge case).
* **Deadlock/hold-and-wait fix (`pair_scheduler.c`, `dongle.c`)** — reasoning through the atomic-pair acquisition redesign (`try_grab_pair()` / `take_dongle_pair()`) that replaced the earlier sequential per-dongle acquisition, to remove a hold-and-wait window.
* **Argument validation (`parser.c`)** — discussing edge cases around the `number_of_compiles_required >= 1` bound in `fill_arguments()`.
* **Norminette compliance** — assisting with refactoring functions across the codebase (splitting oversized functions, renaming variables, restructuring conditionals) to comply with the 42 Norm, while preserving behavior.
* **README documentation** — organizing this documentation and keeping it in sync as the implementation changed.

All implementation decisions, algorithm design, testing, validation, and final code modifications were performed by the author.

### Educational Resources

* Harvard CS61: Synchronization 2 — Condition Variables and Lost Wakeups
  https://www.youtube.com/watch?v=zOpzGHwJ3MU&t=2790s

* CodeVault — Concurrency and Multithreading in C/C++ Playlist
  https://www.youtube.com/playlist?list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2

These resources were particularly useful for understanding thread creation, mutex synchronization, race conditions, deadlock avoidance, starvation prevention, and concurrent systems design.
