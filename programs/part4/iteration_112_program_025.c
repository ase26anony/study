#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/* Helper function for non-constant initializer */
static int some_function(void) {
    static atomic_int counter = 0;
    return atomic_fetch_add(&counter, 1) + 1000;
}

/* 1. Extern TLS with hidden visibility */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS with non-constant initializer */
__thread int tls_var3 = 42;  /* Constant initializer */
__thread int tls_var4 = some_function();  /* Non-constant initializer */

/* 4. Static TLS (internal linkage) */
static __thread int tls_var5 __attribute__((used));

/* 5. DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_var6;
#else
    /* Simulate with visibility attribute */
    __thread int tls_var6 __attribute__((visibility("default"), used));
#endif

/* Global counter to track thread completion */
static atomic_int thread_counter = 0;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    int* addr1 = &tls_var1;
    int* addr2 = &tls_var2;
    int* addr3 = &tls_var3;
    int* addr4 = &tls_var4;
    int* addr5 = &tls_var5;
    int* addr6 = &tls_var6;
    
    /* Prevent optimization from removing TLS accesses */
    volatile int* vptr1 = addr1;
    volatile int* vptr2 = addr2;
    volatile int* vptr3 = addr3;
    volatile int* vptr4 = addr4;
    volatile int* vptr5 = addr5;
    volatile int* vptr6 = addr6;
    
    /* Write unique values to each TLS variable */
    *vptr1 = 100 + thread_id;
    *vptr2 = 200 + thread_id;
    *vptr3 = 300 + thread_id;
    *vptr4 = 400 + thread_id;
    *vptr5 = 500 + thread_id;
    *vptr6 = 600 + thread_id;
    
    /* Read back and verify values */
    int sum = 0;
    sum += *vptr1;
    sum += *vptr2;
    sum += *vptr3;
    sum += *vptr4;
    sum += *vptr5;
    sum += *vptr6;
    
    /* Store result in heap for main thread to collect */
    int* result = malloc(sizeof(int));
    *result = sum;
    
    atomic_fetch_add(&thread_counter, 1);
    
    return result;
}

/* External declaration to force external linkage */
extern __thread int tls_var1;

int main(void) {
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* thread_results[NUM_THREADS];
    
    /* Initialize TLS variables in main thread */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    tls_var5 = 5;
    tls_var6 = 6;
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Wait for all threads to complete */
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
    }
    
    /* Verify all threads completed */
    if (thread_counter != NUM_THREADS) {
        fprintf(stderr, "Error: Only %d threads completed\n", thread_counter);
        return 1;
    }
    
    /* Access TLS variables in main thread after threads have run */
    int main_sum = 0;
    main_sum += tls_var1;  /* Should still be 1 (main thread's instance) */
    main_sum += tls_var2;
    main_sum += tls_var3;
    main_sum += tls_var4;
    main_sum += tls_var5;
    main_sum += tls_var6;
    
    /* Calculate total sum from all threads */
    int total_sum = main_sum;
    for (int i = 0; i < NUM_THREADS; i++) {
        total_sum += *thread_results[i];
        free(thread_results[i]);
    }
    
    printf("Main thread TLS sum: %d\n", main_sum);
    printf("Total sum from all threads: %d\n", total_sum);
    printf("Expected total sum formula: main_sum + Σ(2100 + 6*thread_id)\n");
    
    /* Additional conditional use to prevent dead code elimination */
    if (total_sum > 0) {
        printf("TLS test completed successfully!\n");
    } else {
        printf("Error: Invalid total sum\n");
        return 1;
    }
    
    return 0;
}
