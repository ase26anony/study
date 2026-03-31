/* test_tls_coverage.c - Program to trigger TLS declaration attribute copying */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    static atomic_int counter = 0;
    return ++counter;
}

/* 1. Extern TLS with hidden visibility and used attribute */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak));

/* 3. Public common TLS (implicitly extern) */
__thread int tls_var3;

/* 4. Static TLS with DLL import simulation */
/* For non-Windows, we'll use a visibility attribute as proxy */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_var4;
#else
    /* Use visibility to ensure DECL_VISIBILITY_SPECIFIED is set */
    __thread int tls_var4 __attribute__((visibility("default")));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Another TLS with explicit external linkage and public visibility */
extern __thread int tls_var5 __attribute__((visibility("default")));

/* Define the external TLS variables */
__thread int tls_var1 = 100;
__thread int tls_var5 = 500;

/* Global array to collect results from threads */
static int thread_results[4][6] = {0};
static atomic_int thread_counter = 0;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    int idx = atomic_fetch_add(&thread_counter, 1);
    
    /* Take addresses to force ODR-use and prevent optimization */
    volatile int* addrs[6];
    addrs[0] = &tls_var1;
    addrs[1] = &tls_var2;
    addrs[2] = &tls_var3;
    addrs[3] = &tls_var4;
    addrs[4] = &tls_init;
    addrs[5] = &tls_var5;
    
    /* Assign thread-specific values to each TLS variable */
    tls_var1 = 1000 + thread_id * 10;
    tls_var2 = 2000 + thread_id * 10;
    tls_var3 = 3000 + thread_id * 10;
    tls_var4 = 4000 + thread_id * 10;
    tls_init = 5000 + thread_id * 10;
    tls_var5 = 6000 + thread_id * 10;
    
    /* Read back and store results (prevents dead code elimination) */
    thread_results[idx][0] = tls_var1;
    thread_results[idx][1] = tls_var2;
    thread_results[idx][2] = tls_var3;
    thread_results[idx][3] = tls_var4;
    thread_results[idx][4] = tls_init;
    thread_results[idx][5] = tls_var5;
    
    /* Conditional access based on thread_id to prevent optimization */
    if (thread_id % 2 == 0) {
        tls_var1 += 1;
        tls_var3 += 1;
        tls_var5 += 1;
    } else {
        tls_var2 += 2;
        tls_var4 += 2;
        tls_init += 2;
    }
    
    /* Force another read after conditional modification */
    thread_results[idx][0] += tls_var1;
    thread_results[idx][1] += tls_var2;
    thread_results[idx][2] += tls_var3;
    thread_results[idx][3] += tls_var4;
    thread_results[idx][4] += tls_init;
    thread_results[idx][5] += tls_var5;
    
    return NULL;
}

/* Function that returns address of TLS variable (forces TLS machinery) */
int* get_tls_address(int which) {
    switch (which) {
        case 0: return &tls_var1;
        case 1: return &tls_var2;
        case 2: return &tls_var3;
        case 3: return &tls_var4;
        case 4: return &tls_init;
        case 5: return &tls_var5;
        default: return NULL;
    }
}

int main(void) {
    pthread_t threads[4];
    int thread_ids[4];
    
    /* Initialize TLS variable with non-constant initializer */
    tls_init = get_initial_value();
    
    /* Force taking addresses in main thread as well */
    volatile int* main_addrs[6];
    for (int i = 0; i < 6; i++) {
        main_addrs[i] = get_tls_address(i);
    }
    
    /* Set main thread's TLS values */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    /* tls_init already set */
    tls_var5 = 5;
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < 4; i++) {
        thread_ids[i] = i + 1;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Wait for all threads to complete */
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    /* Access TLS in main thread again (different instance) */
    int main_tls_sum = 
        tls_var1 + tls_var2 + tls_var3 + 
        tls_var4 + tls_init + tls_var5;
    
    /* Calculate checksum from all thread results */
    int total_checksum = main_tls_sum;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 6; j++) {
            total_checksum += thread_results[i][j];
        }
    }
    
    /* Print results to ensure execution */
    printf("Main thread TLS sum: %d\n", main_tls_sum);
    printf("Total checksum from all threads: %d\n", total_checksum);
    
    /* Verify that TLS variables have thread-local behavior */
    printf("Main thread tls_var1: %d (should be 1)\n", tls_var1);
    printf("Main thread tls_init: %d (should be %d)\n", tls_init, get_initial_value());
    
    return 0;
}
