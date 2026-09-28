// #include "kernel/types.h"
// #include "user/user.h"
// #include "user/uthread.h"

// #define NUM_THREAD 5
// #define STACK_PAGES 1

// struct thread_arg {
//   uint64 num;
//   uint64 value;
// };

// int threads[NUM_THREAD];
// struct thread_arg arguments[NUM_THREAD];
// volatile int status;
// volatile int expected[NUM_THREAD];
// volatile int *ptr;
// volatile int sbrk_start;
// volatile int sbrk_written[NUM_THREAD];
// volatile int malloc_turn;
// volatile int exec_rejected[NUM_THREAD];

// static void
// start_thread(int index, void (*fn)(void *), uint64 value)
// {
//   arguments[index].num = index;
//   arguments[index].value = value;
//   threads[index] = thread_create(fn, &arguments[index], STACK_PAGES);
//   if(threads[index] < 0){
//     printf("Thread %d create failed\n", index);
//     exit(1);
//   }
// }

// static void
// join_all(void)
// {
//   for(int i = 0; i < NUM_THREAD; i++){
//     if(thread_join() < 0){
//       printf("Thread %d join failed\n", i);
//       exit(1);
//     }
//   }
// }

// // test#1
// void
// thread_basic(void *arg)
// {
//   struct thread_arg *a = (struct thread_arg *)arg;
//   uint64 num = a->num;

//   printf("Thread %lu start\n", num);
//   if(num == 0){
//     pause(20);
//     status = 1;
//   }
//   printf("Thread %lu end\n", num);
//   exit(0);
// }

// // test#2
// void
// thread_inc(void *arg)
// {
//   struct thread_arg *a = (struct thread_arg *)arg;
//   uint64 num = a->num;
//   uint64 iter = a->value;

//   printf("Thread %lu start, iter=%lu\n", num, iter);
//   for(int i = 0; i < iter; i++)
//     expected[num]++;
//   printf("Thread %lu end\n", num);
//   exit(0);
// }

// // test#3
// void
// thread_fork(void *arg)
// {
//   struct thread_arg *a = (struct thread_arg *)arg;
//   uint64 num = a->num;
//   int pid;

//   printf("Thread %lu start\n", num);
//   pid = fork();
//   if(pid < 0){
//     printf("Fork error on thread %lu\n", num);
//     exit(1);
//   }

//   if(pid == 0){
//     printf("Child of thread %lu start\n", num);
//     pause(10);
//     status = 3;
//     printf("Child of thread %lu end\n", num);
//     exit(0);
//   }

//   status = 2;
//   if(wait(0) == -1){
//     printf("Thread %lu lost its child\n", num);
//     exit(1);
//   }
//   printf("Thread %lu end\n", num);
//   exit(0);
// }

// // test#4
// void
// thread_sbrk(void *arg)
// {
//   struct thread_arg *a = (struct thread_arg *)arg;
//   uint64 num = a->num;
//   char *old_break;

//   while(sbrk_start == 0)
//     pause(1);

//   old_break = sbrk(0);
//   if(num == 0){
//     printf("Thread %lu sbrk: old break = %p\n", num, old_break);
//     ptr = (int *)malloc(4096 * sizeof(int) * NUM_THREAD);
//     if(ptr == 0){
//       printf("Thread %lu malloc failed\n", num);
//       exit(1);
//     }
//     printf("Thread %lu sbrk: new break = %p\n", num, sbrk(0));

//     for(int i = 1; i < NUM_THREAD; i++)
//       while(sbrk_written[i] == 0)
//         pause(1);

//     free((void *)ptr);
//     ptr = 0;
//   } else {
//     while(ptr == 0)
//       pause(1);
//     printf("Thread %lu size = %p\n", num, sbrk(0));
//     for(int i = 0; i < 4096; i++)
//       ptr[num * 4096 + i] = num;
//     sbrk_written[num] = 1;
//     while(ptr != 0)
//       pause(1);
//   }

//   // umalloc has no thread lock, so exercise it one thread at a time.
//   while(malloc_turn != num)
//     pause(1);
//   for(int i = 0; i < 100; i++){
//     int *p = (int *)malloc(4096);
//     if(p == 0){
//       printf("Thread %lu malloc failed\n", num);
//       exit(1);
//     }
//     for(int j = 0; j < 4096 / sizeof(int); j++)
//       p[j] = num;
//     for(int j = 0; j < 4096 / sizeof(int); j++){
//       if(p[j] != num){
//         printf("Thread %lu found %d\n", num, p[j]);
//         exit(1);
//       }
//     }
//     free(p);
//   }
//   malloc_turn++;
//   printf("Thread %lu end\n", num);
//   exit(0);
// }

// // test#5
// void
// thread_kill(void *arg)
// {
//   struct thread_arg *a = (struct thread_arg *)arg;

//   printf("Thread %lu start, pid %lu\n", a->num, a->value);
//   if(a->num == 0){
//     pause(1);
//     kill(a->value);
//   }
//   for(;;)
//     pause(1000);
// }

// // test#6a: exec from a non-leader thread must fail.
// void
// thread_exec_rejected(void *arg)
// {
//   struct thread_arg *a = (struct thread_arg *)arg;
//   char *args[] = {"echo", "unexpected exec", 0};

//   if(exec("echo", args) < 0)
//     exec_rejected[a->num] = 1;
//   exit(0);
// }

// // test#6b: these siblings must be removed by a successful leader exec.
// void
// thread_hold(void *arg)
// {
//   (void)arg;
//   for(;;)
//     pause(1000);
// }

// int
// main(int argc, char *argv[])
// {
//   int i, pid, xstatus;

//   (void)argc;
//   (void)argv;

//   printf("\n[TEST#1]\n");
//   for(i = 0; i < NUM_THREAD; i++)
//     start_thread(i, thread_basic, 0);
//   join_all();
//   if(status != 1){
//     printf("TEST#1 Failed\n");
//     exit(1);
//   }
//   printf("TEST#1 Passed\n");

//   printf("\n[TEST#2]\n");
//   for(i = 0; i < NUM_THREAD; i++)
//     start_thread(i, thread_inc, i * 1000);
//   join_all();
//   for(i = 0; i < NUM_THREAD; i++){
//     if(expected[i] != i * 1000){
//       printf("Thread %d expected %d, but got %d\n", i, i * 1000, expected[i]);
//       exit(1);
//     }
//   }
//   printf("TEST#2 Passed\n");

//   printf("\n[TEST#3]\n");
//   status = 0;
//   for(i = 0; i < NUM_THREAD; i++)
//     start_thread(i, thread_fork, 0);
//   join_all();
//   if(status != 2){
//     printf("TEST#3 Failed: unexpected status %d\n", status);
//     exit(1);
//   }
//   printf("TEST#3 Passed\n");

//   printf("\n[TEST#4]\n");
//   sbrk_start = 0;
//   malloc_turn = 0;
//   for(i = 0; i < NUM_THREAD; i++){
//     sbrk_written[i] = 0;
//     start_thread(i, thread_sbrk, 0);
//   }
//   sbrk_start = 1;
//   join_all();
//   printf("TEST#4 Passed\n");

//   printf("\n[TEST#5]\n");
//   pid = fork();
//   if(pid < 0){
//     printf("Fork error\n");
//     exit(1);
//   }
//   if(pid == 0){
//     int leader = getpid();
//     for(i = 0; i < NUM_THREAD; i++)
//       start_thread(i, thread_kill, leader);
//     for(;;)
//       pause(1000);
//   }
//   if(wait(&xstatus) != pid || xstatus != -1){
//     printf("TEST#5 Failed: kill did not terminate group\n");
//     exit(1);
//   }
//   printf("TEST#5 Passed\n");

//   printf("\n[TEST#6]\n");
//   for(i = 0; i < NUM_THREAD; i++){
//     exec_rejected[i] = 0;
//     start_thread(i, thread_exec_rejected, 0);
//   }
//   join_all();
//   for(i = 0; i < NUM_THREAD; i++){
//     if(exec_rejected[i] == 0){
//       printf("TEST#6 Failed: thread exec was accepted\n");
//       exit(1);
//     }
//   }

//   pid = fork();
//   if(pid < 0){
//     printf("Fork error\n");
//     exit(1);
//   }
//   if(pid == 0){
//     char *args[] = {"echo", "leader exec passed", 0};

//     for(i = 0; i < NUM_THREAD; i++)
//       start_thread(i, thread_hold, 0);
//     exec("echo", args);
//     printf("TEST#6 Failed: leader exec returned\n");
//     exit(1);
//   }
//   if(wait(&xstatus) != pid || xstatus != 0){
//     printf("TEST#6 Failed: leader exec did not complete\n");
//     exit(1);
//   }
//   printf("TEST#6 Passed\n");

//   printf("\nAll tests passed. Great job!!\n");
//   exit(0);
// }


#include "kernel/types.h"
#include "user/user.h"
#include "kernel/riscv.h"
#include "user/uthread.h"

int shared_value = 0;
volatile int thread_exec_failed = 0;

//DO NOTHING.
void
dummy(void *arg) {
  exit(0);
}

//TEST FOR SHARED MEMORY BETWEEN THREADS.
void
th_add(void *arg){
  int n = (int)(uint64)arg;
  shared_value += n;
  exit(0);
}

//TEST FOR KILLING GROUP LEADER.
// DO NOTHING. JUST WAIT UNTIL KILLED BY GROUP LEADER.
void
th_nothing(void *arg){
  while(1);
}

//TEST FOR EXITING GROUP LEADER.
// pause and exit.
void
th_exit(void *arg){
  pause(100);
  exit(0);
}

void
th_exec(void *arg){
  char *args[] = { "echo", "[FAIL] THREAD EXEC EXECUTED.", 0 };
  exec("echo", args);
  thread_exec_failed = 1; // if exec fails, this line will be executed.
  exit(0);
}

void
th_concurrent_sbrk(void *arg){
  // each thread expands and shrinks the heap 100 times.
  for(int i = 0; i < 100; i++){
    void *mem = sbrk(PGSIZE);
    if(mem == (void *)-1){
      printf("[FAIL] CONCURRENT SBRK ALLOCATION FAILED.\n");
      exit(1);
    }
    
  
    char *ptr = (char *)mem;
    ptr[0] = 'X'; 
    ptr[4095] = 'Y';

    // shrink the heap back.
    // sbrk(-PGSIZE);
  }
  exit(0);
}

void
test1(){
  printf("\n=== TEST 1. THREAD CREATION/JOIN TEST === \n");
  int created = thread_create(dummy, (void *)0, 1);
  int joined = thread_join();

  printf("THREAD CREATED PID : %d\n", created);
  printf("THREAD JOINED PID : %d\n", joined);

  if(created == joined && created != -1){
    printf("TEST 1 PASS\n");
  }else{
    printf("TEST 1 FAILED\n");
  }
}

void
test2(){
  printf("\n=== TEST 2. MAXIMUM THREAD TEST === \n");
  for(int i = 0; i < 7; i++){
    thread_create(dummy, (void *)0, 1);
  }

  int fail_tid = thread_create(dummy, (void *)0, 1); //MUST BE FAILED.
  if(fail_tid >= 0){
    printf("TEST 2 FAILED.\n");
  }else{
    printf("TEST 2 PASS \n");
  }

  for(int i = 0; i < 7; i++){
    thread_join();
  }
}

void
test3(){
  printf("\n=== TEST 3. SHARED MEMORY TEST === \n");
  shared_value = 0;
  for(int i = 0; i < 5; i++){
    int tid = thread_create(th_add, (void *)10, 1);
    if(tid < 0){
      printf("THREAD CREATION FAILED AT ITERATION %d\n", i);
    }
  }

  for(int i = 0; i < 5; i++){
    thread_join();
  }

  if(shared_value != 50){
    printf("TEST 3 FAILED. EXPECTED VALUE : 50, ACTUAL VALUE : %d\n", shared_value);
    exit(1);
  }else{
    printf("TEST 3 PASS. SHARED VALUE : %d\n", shared_value);
  }
  
}

void
test4(){
  printf("\n=== TEST 4. THREAD CREATION STRESS TEST === \n");
  printf("CREATING THREADS...\n");
  unsigned char failed = 0;
  for(int i = 1; i < 1001; i++){
    int tid = thread_create(dummy, (void *)0, 1);
    if(tid < 0){
      failed = 1;
      break;
    }
    thread_join();
    if(i % 100 == 0){
      printf("%d THREADS CREATED AND JOINED\n", i);
    }
  }

  if(!failed){
    printf("TEST 4 PASS.\n");
  }else{
    printf("TEST 4 FAILED.\n");
  }
}

void
test5(){
  printf("\n=== TEST 5. GROUP LEADER EXIT TEST === \n");
  int pid = fork();
  if(pid < 0){
    printf("FORK FAILED\n");
    exit(1);
  }else if(pid == 0){
    for(int i = 0; i < 7; i++) thread_create(th_nothing, (void *)0, 1);
    exit(0);
  }else{
    wait(0);
    
    int create_count = 0;
    for(int i = 0; i < 70; i++){
      int child_pid = fork();
      if(child_pid < 0){
        break;
      }else if(child_pid == 0){
        exit(0);
      }else{
        create_count++;
      }
    }

    for(int i = 0; i < create_count; i++){
      wait(0);
    }

    if(create_count < 60) printf("TEST 5 FAILED\n");
    else printf("TEST 5 PASS\n");
  }
}

void
test6(){
  printf("\n=== TEST 6. THREAD GROUP EXEC TEST === \n");
  int pid = fork();
  if(pid < 0){
    printf("FORK FAILED\n");
    exit(1);
  }else if(pid == 0){
    for(int i = 0; i < 6; i++) thread_create(th_nothing, (void *)0, 1);
    thread_create(th_exec, (void *)0, 1); //must be failed.
    pause(20);
    if(thread_exec_failed){
      printf("[PASS] THREAD EXEC FAILED AS EXPECTED\n");
    }
    exec("echo", (char *[]){ "echo", "[PASS] LEADER EXEC PASSED.", 0});
    printf("[FAIL] LEADER EXEC FAILED.\n");
    exit(1);
  }else{
    wait(0);
    
    int create_count = 0;
    for(int i = 0; i < 70; i++){
      int child_pid = fork();
      if(child_pid < 0){
        break;
      }else if(child_pid == 0){
        exit(0);
      }else{
        create_count++;
      }
    }

    for(int i = 0; i < create_count; i++){
      wait(0);
    }

    if(create_count < 60) printf("[FAIL] EXEC RESOURCE MANAGEMENT\n");
    else printf("[PASS] EXEC RESOURCE MANAGEMENT\n");
  }
}

void
test7(){
  printf("\n=== TEST 7. KILL TEST === \n");
  int pid = fork();
  if(pid < 0){
    printf("FORK FAILED\n");
    exit(1);
  }else if(pid == 0){
    for(int i = 0; i < 6; i++) thread_create(th_nothing, (void *)0, 1);
    int tid = thread_create(th_nothing, (void *)0, 1);
    kill(tid); // kill one of the sibling threads.

    printf("[FAIL] KILL TEST FAILED\n"); // this should not be printed.
    exit(1);
  }else{
    wait(0);

    int create_count = 0;
    for(int i = 0; i < 70; i++){
      int child_pid = fork();
      if(child_pid < 0){
        break;
      }else if(child_pid == 0){
        exit(0);
      }else{
        create_count++;
      }
    }

    for(int i = 0; i < create_count; i++){
      wait(0);
    }

    if(create_count > 60) printf("[PASS] KILL TEST PASSED\n");
  }
}

void
test8(){
  printf("\n=== TEST 8. WAIT TEST === \n");
  int tid = -1;
  int pid = fork();
  if(pid < 0){
    printf("FORK FAILED\n");
    exit(1);
  }else if(pid == 0){
    tid = thread_create(th_exit, (void *)0, 1); // create a thread which will exit soon.
    pause(10); // wait until the thread exits.
    exit(0);
  }else{
    int result = wait(0);
    if(result < 0 || result == tid){
      printf("[FAIL] WAIT TEST FAILED\n");
    }else if(result == pid){
      printf("[PASS] SUCCESSFULLY WAITED FOR GROUP LEADER PROCESS\n");
    }else{
      printf("[FAIL] WAIT TEST FAILED. WAIT RETURNED UNKNOWN PID : %d\n", result);
    } 
  }
}

void
test9(){
  printf("\n=== TEST 9. CONCURRENCY STRESS TEST === \n");
  
  int created = 0;

  for(int i = 0; i < 5; i++){
    int tid = thread_create(th_concurrent_sbrk, (void *)0, 1);
    if(tid > 0){
      created++;
    }else{
      printf("THREAD CREATION FAILED AT ITERATION %d\n", i);
    }
  }

  for(int i = 0; i < created; i++){
    thread_join();
  }

  if(created == 5){
    printf("[PASS] CONCURRENCY SBRK TEST PASSED. NO KERNEL PANIC.\n");
  }else{
    printf("[FAIL] FAILED TO CREATE TARGET NUMBER OF THREADS.\n");
  }
}

int
main(void){
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
  test7();
  test8();
  test9();
  // exec("usertests", (char *[]){ "usertests", 0});
}
