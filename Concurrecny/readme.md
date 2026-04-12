## 3. Thread Basics

### `std::thread`

Create and manage OS threads.

#### Creation

```cpp
std::thread t(func, args...);
```

#### Join / Detach

```cpp
t.join();    // wait for completion (must call)
t.detach();  // run independently
```

#### Passing arguments

```cpp
void f(int x, int& y);

int a = 10, b = 20;

std::thread t1(f, a, std::ref(b));   // ref
std::thread t2(f, std::move(a), std::ref(b)); // move
```

#### Thread ID

```cpp
t.get_id();
std::this_thread::get_id();
```

---

## 4. Synchronization Primitives

### 4.1 Mutex family

#### `std::mutex`

Basic mutual exclusion.

```cpp
m.lock();
m.unlock();
```

#### `std::recursive_mutex`

Same thread can lock multiple times.

#### `std::timed_mutex`

```cpp
m.try_lock_for(duration);
m.try_lock_until(time_point);
```

#### `std::shared_mutex`

Reader-writer lock.

```cpp
m.lock();          // write
m.lock_shared();   // read
```

---

### 4.2 Lock wrappers

#### `std::lock_guard`

Simple RAII lock.

```cpp
std::lock_guard<std::mutex> l(m);
```

| Method | Description |
| ------ | ----------- |
| ctor   | locks mutex |
| dtor   | unlocks     |

---

#### `std::unique_lock`

Flexible lock (can defer, unlock, move).

```cpp
std::unique_lock<std::mutex> l(m);
std::unique_lock<std::mutex> l(m, std::defer_lock);
```

| Method      | Description |
| ----------- | ----------- |
| lock()      | acquire     |
| unlock()    | release     |
| try_lock()  | attempt     |
| owns_lock() | check       |

---

#### `std::scoped_lock`

Locks multiple mutexes safely (deadlock-free).

```cpp
std::scoped_lock l(m1, m2);
```

---

## 5. Condition Variables

### `std::condition_variable`

Wait/notify mechanism.

```cpp
std::condition_variable cv;
std::mutex m;
```

#### Wait

```cpp
std::unique_lock<std::mutex> lk(m);
cv.wait(lk);
```

#### Notify

```cpp
cv.notify_one();
cv.notify_all();
```

---

### Spurious wakeups

`wait()` may return without notify → must recheck condition.

---

### Predicate pattern (IMPORTANT)

```cpp
cv.wait(lk, []{ return condition; });
```

---

## 6. Atomics

### 6.1 Basics

#### `std::atomic<T>`

```cpp
std::atomic<int> x = 0;
```

* lock-free: no mutex
* wait-free: guaranteed completion (stronger)

---

### 6.2 Operations

#### load / store

```cpp
x.load();
x.store(10);
```

#### exchange

```cpp
x.exchange(5);
```

#### compare_exchange

```cpp
x.compare_exchange_weak(expected, desired);
x.compare_exchange_strong(expected, desired);
```

| Method | Description         |
| ------ | ------------------- |
| weak   | may fail spuriously |
| strong | no spurious failure |

---

### 6.3 Memory ordering

Used with atomic ops.

```cpp
x.store(1, std::memory_order_release);
x.load(std::memory_order_acquire);
```

| Order   | Meaning                        |
| ------- | ------------------------------ |
| relaxed | no ordering                    |
| acquire | after ops not reordered before |
| release | before ops not reordered after |
| acq_rel | both                           |
| seq_cst | strongest global ordering      |

---

## 7. Thread Coordination / Higher-level constructs

### `std::future`

Represents async result.

```cpp
f.get();   // blocks, returns result
f.wait();  // wait only
```

---

### `std::promise`

Producer side of future.

```cpp
std::promise<int> p;
auto f = p.get_future();

p.set_value(10);
```

---

### `std::async`

Run function asynchronously.

```cpp
auto f = std::async(std::launch::async, func);
```

| Launch policy | Meaning      |
| ------------- | ------------ |
| async         | new thread   |
| deferred      | run on get() |

---

## 12. Modern C++ (C++20+)

### `std::jthread`

RAII thread with stop support.

```cpp
std::jthread t(func);
```

| Feature    | Description      |
| ---------- | ---------------- |
| auto join  | destructor joins |
| stop_token | built-in cancel  |

---

### `std::stop_token`

Cooperative cancellation.

```cpp
void f(std::stop_token st) {
    while (!st.stop_requested()) {}
}
```

---

### `std::latch`

One-time countdown sync.

```cpp
std::latch l(n);

l.count_down();
l.wait();
```

| Method     | Description   |
| ---------- | ------------- |
| count_down | decrement     |
| wait       | block until 0 |

---

### `std::barrier`

Reusable sync point.

```cpp
std::barrier b(n);

b.arrive_and_wait();
```

| Method          | Description       |
| --------------- | ----------------- |
| arrive          | signal arrival    |
| arrive_and_wait | sync              |
| completion fn   | optional callback |
