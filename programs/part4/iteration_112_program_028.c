/* test_tls_emulation.c - Program to trigger TLS declaration cloning logic */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    static atomic_int counter = 0;
    return atomic_fetch_add(&counter, 1) + 42;
}

/* ========== TLS VARIABLES WITH VARIOUS ATTRIBUTES ========== */

/* 1. Extern TLS with hidden visibility - will trigger DECL_VISIBILITY copying */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - will trigger DECL_WEAK copying */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS (implicitly extern) - triggers DECL_COMMON, TREE_PUBLIC */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation */
/* For MinGW/Windows targets, use: __declspec(dllimport) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_var4;
#else
    /* Simulate with visibility and used attributes */
    __thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer - complicates initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Another extern TLS with default visibility */
extern __thread int tls_var5 __attribute__((visibility("default")));

/* Define the extern TLS variables */
__thread int tls_var1 = 1;
__thread int tls_var5 = 5;

/* ========== THREAD FUNCTION ========== */

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    volatile int* addr1 = &tls_var1;
    volatile int* addr2 = &tls_var2;
    volatile int* addr3 = &tls_var3;
    volatile int* addr4 = &tls_var4;
    volatile int* addr5 = &tls_var5;
    volatile int* addr_init = &tls_init;
    
    /* Prevent optimization by using addresses */
    (void)addr1; (void)addr2; (void)addr3; 
    (void)addr4; (void)addr5; (void)addr_init;
    
    /* Assign thread-specific values to TLS variables */
    tls_var1 = 1000 + thread_id * 10;
    tls_var2 = 2000 + thread_id * 10;
    tls_var3 = 3000 + thread_id * 10;
    tls_var4 = 4000 + thread_id * 10;
    tls_var5 = 5000 + thread_id * 10;
    tls_init = 6000 + thread_id * 10;
    
    /* Complex conditional access pattern to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        if (i == 0) sum += tls_var1;
        else if (i == 1) sum += tls_var2;
        else if (i == 2) sum += tls_var3;
        
        /* Mix in other variables based on conditions */
        if (thread_id % 2 == 0) {
            sum += tls_var4;
        } else {
            sum += tls_var5;
        }
        
        sum += tls_init;
    }
    
    /* Return the sum as verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* thread_results[NUM_THREADS];
    
    /* Initialize TLS with non-constant value */
    tls_init = get_initial_value();
    
    /* Initialize main thread's TLS values */
    tls_var1 = 100;
    tls_var2 = 200;
    tls_var3 = 300;
    tls_var4 = 400;
    tls_var5 = 500;
    
    printf("Main thread TLS init value: %d\n", tls_init);
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Access TLS in main thread with complex pattern */
    int main_sum = 0;
    for (int i = 0; i < 5; i++) {
        /* Conditional access to force TLS machinery */
        switch (i % 3) {
            case 0: main_sum += tls_var1; break;
            case 1: main_sum += tls_var2; break;
            case 2: main_sum += tls_var3; break;
        }
        
        if (i % 2 == 0) {
            main_sum += tls_var4;
        } else {
            main_sum += tls_var5;
        }
        
        main_sum += tls_init;
    }
    
    printf("Main thread TLS checksum: %d\n", main_sum);
    
    /* Join threads and collect results */
    int total_sum = main_sum;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *thread_results[i];
        free(thread_results[i]);
    }
    
    /* Final verification */
    printf("Total checksum from all threads: %d\n", total_sum);
    printf("Expected range: > 0 (verifies TLS was actually used)\n");
    
    /* Final access to all TLS variables to ensure they're referenced */
    volatile int final_check = 
        tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    
    return (final_check > 0) ? 0 : 1;
}
