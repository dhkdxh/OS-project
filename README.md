# OS-project

This repository contains three individual projects completed during the undergraduate Operating Systems course. These projects focus on deeply understanding core OS concepts by directly modifying and implementing features in the xv6 operating system, covering scheduling, kernel threads, and virtual memory management.

## 📂 Project Structure and Details
### 1. Project 1: CFS (Completely Fair Scheduler) Implementation
* **Goal:** Replace the default xv6 scheduler with a Completely Fair Scheduler (CFS) based on a Red-Black Tree.
* **Details:**
  * Assigned `nice` values (priority, ranging from -3 to 2) to processes and calculated `vruntime` (virtual runtime) by multiplying the actual CPU execution time by a weight derived from the nice value.
  * Managed Runnable processes using a Red-Black Tree, scheduling the process with the smallest `vruntime` to run next.
  * Applied fine-grained lock designs (`cfs_lock`, `tickslock`, etc.) to prevent Race Conditions and Deadlocks in a Multiprocessor environment.

### 2. Project 2: Kernel-Level Thread Implementation
* **Goal:** Build a kernel-level thread support system where multiple threads share the same memory address space.
* **Details:**
  * Separated the memory management domain into an `mm_struct` from the existing `proc` structure to allow threads to share the address space, introducing a reference count (`refcount`) based memory management approach.
  * Implemented the `clone()` system call to create threads while sharing the address space, and the `join()` system call to safely reclaim the user stack and resources of terminated threads.
  * Completely redesigned existing system calls (`exit`, `wait`, `kill`, `exec`) to ensure that when a thread terminates or overwrites a program, all sibling threads within the same group are correctly cleaned up.

### 3. Project 3: Memory-Mapped Files Implementation
* **Goal:** Introduce the `mmap` feature to map files directly into memory, enhancing I/O efficiency.
* **Details:**
  * Introduced the Virtual Memory Area (VMA) structure to manage mapping information, implementing the `mmap` and `munmap` system calls.
  * Applied Lazy Allocation techniques to prevent physical memory waste. Instead of allocating physical memory immediately upon mapping, the system allocates physical pages and reads file data only when a Page Fault (`mmapfault`) occurs during actual memory access.
  * Ensured that memory regions mapped with the `MAP_SHARED` flag safely write back modifications to the file upon calling `munmap` or during process termination (`kexit`). Additionally, modified `fork` to allow child processes to correctly inherit and share the parent's VMA information.
