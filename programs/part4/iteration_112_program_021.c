/* test_tls_emulation.c - Program to exercise TLS declaration cloning */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    static int counter = 0;
    return ++counter + 1000; /* Non-constant, changes each call */
}

/* ========== TLS VARIABLES WITH VARIOUS ATTRIBUTES ========== */

/* 1. Extern TLS with hidden visibility - will set DECL_VISIBILITY */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - will set DECL_WEAK */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS - will set DECL_COMMON and TREE_PUBLIC */
__thread int tls_var3; /* Implicitly extern, may become common */

/* 4. Static TLS with DLL import simulation */
/* For MinGW/Windows targets, use: __declspec(dllimport) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* On non-Windows, simulate with visibility attribute */
__thread int tls_var4 __attribute__((visibility("default")));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0; /* Will be initialized in main() */

/* 6. Another extern TLS with explicit visibility */
extern __thread int tls_var5 __attribute__((visibility("protected"), used));

/* Define the extern TLS variables */
__thread int tls_var1 = 1;
__thread int tls_var5 = 5;

/* ========== THREAD FUNCTION ========== */

/* Structure to pass data to threads */
typedef struct {
    int thread_id;
    int* results; /* Array to store verification results */
} thread_data_t;

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    int tid = data->thread_id;
    
    /* Take addresses of TLS variables (ODR-use) */
    int* addr1 = &tls_var1;
    int* addr2 = &tls_var2;
    int* addr3 = &tls_var3;
    int* addr4 = &tls_var4;
    int* addr5 = &tls_var5;
    int* addr_init = &tls_init;
    
    /* Prevent optimization from removing address-taking */
    volatile int dummy = (uintptr_t)addr1 + (uintptr_t)addr2 + 
                        (uintptr_t)addr3 + (uintptr_t)addr4 + 
                        (uintptr_t)addr5 + (uintptr_t)addr_init;
    (void)dummy;
    
    /* Assign thread-specific values to TLS variables */
    tls_var1 = 100 + tid;
    tls_var2 = 200 + tid;
    tls_var3 = 300 + tid;
    tls_var4 = 400 + tid;
    tls_var5 = 500 + tid;
    tls_init = 600 + tid;
    
    /* Read back and verify values */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    int expected = (100+200+300+400+500+600) + (6 * tid);
    
    /* Store verification result */
    data->results[tid] = (sum == expected) ? 1 : 0;
    
    /* Conditional access to force compiler to keep all variables */
    if (tls_var1 > 0 && tls_var2 > 0 && tls_var3 > 0 && 
        tls_var4 > 0 && tls_var5 > 0 && tls_init > 0) {
        /* Do nothing, just ensure variables are accessed */
    }
    
    return NULL;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];
    thread_data_t thread_data[NUM_THREADS];
    int results[NUM_THREADS];
    int i;
    
    /* Initialize TLS variable with non-constant value */
    tls_init = get_initial_value();
    
    /* Initialize thread data */
    for (i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].results = results;
        results[i] = 0;
    }
    
    /* Create threads */
    for (i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, &thread_data[i]) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }
    
    /* Main thread also accesses TLS variables */
    tls_var1 = 100 + NUM_THREADS;  /* Main thread gets ID = NUM_THREADS */
    tls_var2 = 200 + NUM_THREADS;
    tls_var3 = 300 + NUM_THREADS;
    tls_var4 = 400 + NUM_THREADS;
    tls_var5 = 500 + NUM_THREADS;
    tls_init = 600 + NUM_THREADS;
    
    /* Join threads */
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    /* Verify all thread results */
    int all_ok = 1;
    for (i = 0; i < NUM_THREADS; i++) {
        if (results[i] != 1) {
            printf("Thread %d failed TLS verification\n", i);
            all_ok = 0;
        }
    }
    
    /* Calculate checksum from main thread's TLS values */
    int main_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    int main_expected = (100+200+300+400+500+600) + (6 * NUM_THREADS);
    
    printf("Main thread TLS sum: %d (expected: %d)\n", main_sum, main_expected);
    
    if (all_ok && main_sum == main_expected) {
        printf("SUCCESS: All TLS operations verified correctly\n");
        return EXIT_SUCCESS;
    } else {
        printf("FAILURE: TLS verification failed\n");
        return EXIT_FAILURE;
    }
}
