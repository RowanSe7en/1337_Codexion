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

    if (coders[i].coder_id % 2 == 0)
    {
        coders[i].first_dongle = &dongles[i];
        coders[i].second_dongle = &dongles[(i + 1) % size];
    }
    else
    {
        coders[i].second_dongle = &dongles[(i + 1) % size];
        coders[i].first_dongle = &dongles[i];
    }
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
---

# STEP 6 — Thread Creation

You spawn all coder threads:

```c
for (int i = 0; i < size; i++)
{
    pthread_create(&coders[i].coder, NULL, main_loop, &coders[i]);
}
```

✔ This means:

* N threads created
* Each thread receives its own struct
* All threads run simultaneously

This is the **birth of concurrency** in your project.

---










Yes: if every coder takes *left then right*, you will eventually get a **deadlock**.

---

# 💀 Why deadlock happens

All coders do the same order:

```
Coder 1 takes Dongle 1 → waits Dongle 2
Coder 2 takes Dongle 2 → waits Dongle 3
Coder 3 takes Dongle 3 → waits Dongle 4
Coder 4 takes Dongle 4 → waits Dongle 5
Coder 5 takes Dongle 5 → waits Dongle 1
```

Everyone holds **one dongle** and waits forever → circular wait → deadlock.

So we must **break the circular wait condition**.

There are two famous solutions used in 1337/42 projects.

---

# 🥇 Solution 1 — EVEN / ODD ORDER (most popular)

### Idea

Half the coders take dongles in opposite order.

| Coder       | Order             |
| ----------- | ----------------- |
| Odd coders  | take LEFT → RIGHT |
| Even coders | take RIGHT → LEFT |

This breaks the circle because not everyone grabs the same side first.

---

### Implementation idea

```c
void take_dongles(t_coder *c)
{
    if (c->coder_id % 2 == 0)
    {
        pthread_mutex_lock(&c->second_dongle->mtx);
        log_action(c, "has taken a dongle");

        pthread_mutex_lock(&c->first_dongle->mtx);
        log_action(c, "has taken a dongle");
    }
    else
    {
        pthread_mutex_lock(&c->first_dongle->mtx);
        log_action(c, "has taken a dongle");

        pthread_mutex_lock(&c->second_dongle->mtx);
        log_action(c, "has taken a dongle");
    }
}
```

### Why it works

Because at least half of the threads grab the opposite resource first → the circular chain is broken.

This is **the simplest and most accepted solution**.

---

# 🥈 Solution 2 — LAST CODER TAKES REVERSED ORDER

### Idea

All coders take LEFT → RIGHT
**Except the last coder**, who takes RIGHT → LEFT.

So only **one coder breaks the circle**.

---

### Implementation idea

```c
void take_dongles(t_coder *c)
{
    if (c->coder_id == c->sim->args.number_of_coders)
    {
        // last coder reversed
        pthread_mutex_lock(&c->second_dongle->mtx);
        log_action(c, "has taken a dongle");

        pthread_mutex_lock(&c->first_dongle->mtx);
        log_action(c, "has taken a dongle");
    }
    else
    {
        pthread_mutex_lock(&c->first_dongle->mtx);
        log_action(c, "has taken a dongle");

        pthread_mutex_lock(&c->second_dongle->mtx);
        log_action(c, "has taken a dongle");
    }
}
```

### Why it works

Deadlock requires a **perfect circle**.
By reversing ONE coder, the circle can never form.

This is also very common and valid.

---

# 🧠 Which one should you use?

| Method        | Difficulty | Reliability | Typical choice   |
| ------------- | ---------- | ----------- | ---------------- |
| Even/Odd      | ⭐ Easy     | ⭐⭐⭐⭐        | 🔥 Most used     |
| Last reversed | ⭐ Easy     | ⭐⭐⭐⭐        | Also very common |

Evaluators accept **both**.

Most students use **even/odd** because it distributes fairness better.

---

# 🔥 Important note

Deadlock is solved, but you can still have **starvation** later.
That’s handled by your **monitor thread** (death detection).

---

# 🎯 Final takeaway

Deadlock happens because everyone grabs resources in the same order.
You fix it by forcing **different locking orders**.

That’s the core trick.

---

Next step will be: releasing dongles + compile phase if you want to continue the lifecycle.





Here is the **exact ordered millisecond timeline** of the EVEN/ODD solution.
Assume `time_to_compile = 200ms` just to visualize time.

Rules:

* Odd coder → LEFT then RIGHT
* Even coder → RIGHT then LEFT
* Dongles are circular (5 coders, 5 dongles)

Dongles:
C1 → D1 & D2
C2 → D2 & D3
C3 → D3 & D4
C4 → D4 & D5
C5 → D5 & D1

---

# 🕒 Timeline — exact race

## Step 1 — t = 0ms → threads start simultaneously

All coders try to take their **first dongle**.

* C1 (odd) locks **D1**
* C2 (even) locks **D3**
* C3 (odd) tries D3 → ❌ blocked (C2 has it)
* C4 (even) locks **D5**
* C5 (odd) tries D5 → ❌ blocked (C4 has it)

State after first grabs:

| Coder | Holds | Waiting |
| ----- | ----- | ------- |
| C1    | D1    | D2      |
| C2    | D3    | D2      |
| C3    | —     | D3      |
| C4    | D5    | D4      |
| C5    | —     | D5      |

Free dongles right now → **D2 and D4**

This is the key moment: the circle is already broken.

---

## Step 2 — t = 1ms → race for second dongles

Two dongles are free → two winners appear simultaneously.

* C1 grabs **D2**
* C4 grabs **D4**

Now:

| Coder | Holds   | State               |
| ----- | ------- | ------------------- |
| C1    | D1 + D2 | 🔥 starts compiling |
| C4    | D5 + D4 | 🔥 starts compiling |

Two coders compile in parallel for **200ms**.

---

## Step 3 — t = 200ms → first release wave

C1 and C4 finish compiling and release dongles:

Released:

* D1
* D2
* D4
* D5

But remember: **C2 is still holding D3** (he grabbed it at t=0).

New state immediately after release:

| Coder | Holds | Waiting |
| ----- | ----- | ------- |
| C2    | D3    | D2      |
| C3    | —     | D3      |
| C5    | —     | D5      |

Free dongles now → **D1, D2, D4, D5**

---

## Step 4 — t = 201ms → next race begins

### C2 wakes up first (already holding D3)

D2 becomes free → C2 instantly grabs it.

C2 now has **D3 + D2** → starts compiling.

Current used dongles:

* D2 & D3 → used by C2

Free dongles:

* D1, D4, D5

---

## Step 5 — t = 202ms → next coder wakes up

Check waiting coders:

* C3 waiting D3 → still blocked (C2 has it)
* C5 waiting D5 → FREE ✔

C5 runs.

C5 grabs:

* D5 (first)
* D1 (second)

C5 now has **D5 + D1** → starts compiling.

Now again TWO coders compile in parallel:

* C2 compiling
* C5 compiling

---

## Step 6 — t = 400ms → second release wave

C2 releases → D2 & D3
C5 releases → D5 & D1

All dongles become free.

Waiting coder:

* C3 finally grabs D3 then D4 → compiles.

The wave continues forever.

---

# 🎯 Final understanding

Timeline pattern:

```
t=0   → first locks
t=1   → C1 & C4 compile
t=200 → C2 & C5 compile
t=400 → C3 compiles
t=600 → cycle repeats
```

There is **always progress**, **multiple coders compile**, and **deadlock never occurs**.

















Perfect — now we rebuild the **real worst-case race** exactly as you described.

We keep EVEN / ODD rule:

* Odd → LEFT then RIGHT
* Even → RIGHT then LEFT

Dongles:
C1 → D1 D2
C2 → D2 D3
C3 → D3 D4
C4 → D4 D5
C5 → D5 D1

---

# 🕒 Worst-case timeline (correct version)

## Step 1 — t = 0ms → first dongle race

Everyone tries to grab their **first** dongle at the same instant.

Contested races happen:

• C1 vs C2 race for **D2** → **C1 wins D2**
• C4 vs C3 race for **D4** → **C4 wins D4**
• C5 takes **D1** (no contention yet)

State after first millisecond:

| Coder | Holds | Waiting    |
| ----- | ----- | ---------- |
| C1    | D2    | waiting D1 |
| C2    | —     | waiting D3 |
| C3    | —     | waiting D4 |
| C4    | D4    | waiting D5 |
| C5    | D1    | waiting D5 |

This is already a **very unlucky distribution**.

Only 3 coders got a first dongle.

---

## Step 2 — t = 1ms → second dongle race

Now coders try to take their **second** dongle.

The big race now is for **D5**:

• C4 vs C5 race for **D5** → **C4 wins D5**

State becomes:

| Coder | Holds   | State               |
| ----- | ------- | ------------------- |
| C1    | D2      | waiting D1          |
| C4    | D4 + D5 | 🔥 starts compiling |
| C5    | D1      | waiting D5          |
| C2    | —       | waiting D3          |
| C3    | —       | waiting D4          |

This is the **worst possible case**:

👉 Only **ONE coder** is compiling
👉 Four coders are blocked

Yet still: the system is alive.

---






































Here is the **clean ordered roadmap of the whole Codexion simulation**, from program start → full runtime loop.

---

# Codexion – Full Simulation Steps

## Step 1 — Starter / bootstrap function

Create a function like:

```
int start_simulation(t_simulation *sim)
```

Its job is to **validate and launch the system**.

Inside this function:

1. Validate edge cases

   * If `number_of_coders <= 0` → exit
   * If `number_of_compiles_required <= 0` → exit

Nothing should start if the simulation has no work to do.

---

## Step 2 — Create all threads (coders)

Loop over coders and create threads:

```
for each coder:
    pthread_create(coder_thread, simulation, &coder)
```

At this moment:

* Threads start immediately
* BUT they must NOT start working yet

They must wait for synchronization.

---

## Step 3 — Global start barrier (is_all_ready)

Add a shared boolean in simulation:

```
int is_all_ready;
```

Initialize it to **false**.

After the creation loop finishes:

```
sim->start_time = get_time_ms();
sim->is_all_ready = true;
```

This acts as a **start barrier** so all threads begin at the same moment.

---

## Step 4 — Waiting function (thread synchronization)

Each thread begins by calling:

```
wait_all_threads_ready(sim)
```

This function spins until the starter finishes creating all threads:

```
while (!sim->is_all_ready)
    i++;
```

Important:

* Threads are already running
* They are just spining here until main says GO

This guarantees **perfect synchronization**.

---

## Step 5 — Simulation start timestamp

Right after the barrier releases:

Every thread now shares the same:

```
sim->start_time
```

All logs will be printed as:

```
timestamp = now - start_time
```

This ensures **consistent timing** across threads.

---

## Step 6 — Join threads (main thread responsibility)

After setting `is_all_ready = true`, the starter function must wait:

```
for each coder:
    pthread_join(coder_thread)
```

Main thread becomes the **observer** and waits until simulation ends.

---

# Thread Life — Simulation Routine

Each coder runs the routine:

```
void *simulation(void *arg)
```

---

## Step 7 — Infinite simulation loop

Inside the routine:

```
while (!sim->is_sim_ended)
```

This loop represents the coder life cycle.

---

## Step 8 — Coder life cycle order

This is the most critical routine in the simulation.

The compile phase is made of **3 big parts**:

1. Take the dongles
2. Compile (update timestamps + counters)
3. Release dongles

---

# Step 1 — Function prototype

Create a function similar to:

```
static void compile(t_coder *c)
```

This function receives a pointer to the coder struct.

---

# Step 2 — Grab the two dongles (lock mutexes)


Each coder already knows:

* first_dongle
* second_dongle

### What to implement

1. Lock first dongle mutex

2. Print status: **"has taken a dongle"**

3. Lock second dongle mutex

4. Print status: **"has taken a dongle"**

Pseudo-flow:

```
lock(first_dongle->mutex)
print_log("has taken a dongle")

lock(second_dongle->mutex)
print_log("has taken a dongle")
```

This represents **taking both dongles required to compile**.

---

# Step 3 — Update last_compile_time (thread safe)

This value will be read by the **monitor thread** to detect timeout/starvation.
So it must be protected with a **per-coder mutex**.

### Add a mutex inside coder struct

Each coder needs an internal mutex:

```
pthread_mutex_t coder_mutex;
```

Initialize it during coder initialization.

---

### Now update last_compile_time safely

Inside `compile()`:

```
lock(coder_mutex)
coder->last_compile_time = get_time_ms()
unlock(coder_mutex)
```

This ensures the monitor reads a consistent timestamp.

---

# Step 4 — Increase compile counter

This value is **not shared with monitor**, only used by the coder itself.

So it does NOT need a mutex.

```
coder->compile_count++
```

---

# Step 5 — Print "is compiling"

Order matters!

We update time and counter FIRST, then print:

```
print_log("is compiling")
```

This guarantees logs reflect real state.

---

# Step 6 — Precise sleep for compile duration


## Why `usleep()` is not reliable

`usleep(100000)` means *“sleep AT LEAST 100ms”*, not exactly 100ms.

Your program is not running alone. The OS scheduler decides when your thread wakes up. because of **Scheduler time slice**, **CPU load**, **Kernel timer resolution**,  **Context switching**

So:

```
usleep(100ms) → real sleep could be:
100ms … 110ms … 130ms … sometimes 150ms
```

For a normal app → OK
For a **death timer simulation** → ❌ breaks logic.

---

## The correct strategy: "Smart sleep"

Instead of one long sleep, we sleep **in small chunks** and repeatedly check the time.

Concept:

```
target_time = now + duration

while (current_time < target_time)
    usleep(small_chunk)
```

This reduces overshoot drastically.

---




















---

# Step 7 — Check if coder finished required compiles

If the simulation has a **compile limit**:

```
if (table->compile_limit > 0
    AND coder->compile_count == table->compile_limit)
```

Then we mark coder as **finished**.

This flag **must be protected by coder_mutex**, because the monitor will read it.

```
lock(coder_mutex)
coder->is_full = true
unlock(coder_mutex)
```

This is equivalent to philosopher becoming “full”.

---

# Step 8 — Release the dongles (unlock mutexes)

VERY IMPORTANT: Always release in the end.

```
unlock(first_dongle->mutex)
unlock(second_dongle->mutex)
```

This allows other coders to compile.

---

