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

/* 3. Public common TLS (implicitly extern) */
__thread int tls_var3 __attribute__((used));

/* 4. TLS with non-constant initializer (forces complex initialization) */
__thread int tls_init __attribute__((used)) = some_function();

/* 5. Static TLS (internal linkage) */
static __thread int tls_static __attribute__((used));

/* 6. DLL import simulation (for platforms that support it) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport __attribute__((used));
#else
    /* Simulate with visibility attribute */
    extern __thread int tls_dllimport __attribute__((visibility("default"), used));
#endif

/* Define the extern TLS variables to satisfy linkage */
__thread int tls_var1 = 42;
__thread int tls_var3 = 100;
#ifdef _WIN32
    __thread int tls_dllimport = 200;
#else
    __thread int tls_dllimport __attribute__((visibility("default"))) = 200;
#endif

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    volatile int* addrs[6];
    addrs[0] = &tls_var1;
    addrs[1] = &tls_var2;
    addrs[2] = &tls_var3;
    addrs[3] = &tls_init;
    addrs[4] = &tls_static;
    addrs[5] = &tls_dllimport;
    
    /* Assign unique values based on thread ID */
    tls_var1 = 1000 + thread_id * 10;
    tls_var2 = 2000 + thread_id * 10;
    tls_var3 = 3000 + thread_id * 10;
    tls_init = 4000 + thread_id * 10;
    tls_static = 5000 + thread_id * 10;
    tls_dllimport = 6000 + thread_id * 10;
    
    /* Conditional access to prevent optimization */
    int sum = 0;
    if (thread_id % 2 == 0) {
        sum += tls_var1 + tls_var3;
    } else {
        sum += tls_var2 + tls_dllimport;
    }
    
    /* Always use tls_init and tls_static */
    sum += tls_init * 2;
    sum += tls_static;
    
    /* Return the sum as verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

int main(void) {
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* results[NUM_THREADS];
    
    /* Initialize thread IDs */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;
    }
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Wait for threads to complete */
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
    }
    
    /* Main thread accesses its own TLS instances */
    tls_var1 = 9999;
    tls_var2 = 8888;
    tls_var3 = 7777;
    tls_init = 6666;
    tls_static = 5555;
    tls_dllimport = 4444;
    
    /* Calculate checksum from all thread results and main thread values */
    int total_checksum = 0;
    
    /* Add results from worker threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        total_checksum += *results[i];
        free(results[i]);
    }
    
    /* Add main thread TLS values */
    total_checksum += tls_var1 + tls_var2 + tls_var3 + tls_init + tls_static + tls_dllimport;
    
    /* Force use of all TLS variable addresses in main thread too */
    volatile int* main_addrs[6];
    main_addrs[0] = &tls_var1;
    main_addrs[1] = &tls_var2;
    main_addrs[2] = &tls_var3;
    main_addrs[3] = &tls_init;
    main_addrs[4] = &tls_static;
    main_addrs[5] = &tls_dllimport;
    
    /* Print verification result */
    printf("TLS test completed successfully!\n");
    printf("Total checksum: %d\n", total_checksum);
    printf("Main thread TLS values: %d, %d, %d, %d, %d, %d\n",
           tls_var1, tls_var2, tls_var3, tls_init, tls_static, tls_dllimport);
    
    /* Additional conditional use to prevent dead code elimination */
    if (total_checksum > 0) {
        printf("Checksum is positive - TLS appears functional\n");
    }
    
    return 0;
}
