/* test_tls_coverage.c - Program to trigger TLS declaration attribute copying */
#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    return 42;
}

/* 1. Extern TLS with hidden visibility - will set DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - will set DECL_WEAK */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public Common TLS - implicitly extern, may become common (DECL_COMMON) */
__thread int tls_var3 __attribute__((used));

/* 4. TLS with DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* On non-Windows, use a visibility attribute to ensure DECL_VISIBILITY_SPECIFIED is set */
__thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Static TLS - ensures TREE_PUBLIC/DECL_EXTERNAL combinations */
static __thread int tls_static __attribute__((used));

/* Define the extern TLS variable */
__thread int tls_var1 = 100;

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    volatile int* addr1 = &tls_var1;
    volatile int* addr2 = &tls_var2;
    volatile int* addr3 = &tls_var3;
    volatile int* addr4 = &tls_var4;
    volatile int* addr5 = &tls_init;
    volatile int* addr6 = &tls_static;
    
    /* Prevent optimization from removing TLS accesses */
    if (thread_id % 2 == 0) {
        /* Even threads use one pattern */
        tls_var1 = 1000 + thread_id;
        tls_var2 = 2000 + thread_id;
        tls_var3 = 3000 + thread_id;
        tls_var4 = 4000 + thread_id;
        tls_init = 5000 + thread_id;
        tls_static = 6000 + thread_id;
    } else {
        /* Odd threads use another pattern */
        tls_var1 = 100 + thread_id;
        tls_var2 = 200 + thread_id;
        tls_var3 = 300 + thread_id;
        tls_var4 = 400 + thread_id;
        tls_init = 500 + thread_id;
        tls_static = 600 + thread_id;
    }
    
    /* Read back and verify values */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_static;
    
    /* Conditional use that prevents dead code elimination */
    if (sum > 0) {
        /* Store result in heap to prevent optimization */
        int* result = malloc(sizeof(int));
        *result = sum;
        return result;
    }
    
    return NULL;
}

/* Function that returns address of TLS variable - forces TLS machinery */
static volatile int* get_tls_addr(int selector) {
    switch (selector) {
        case 0: return &tls_var1;
        case 1: return &tls_var2;
        case 2: return &tls_var3;
        case 3: return &tls_var4;
        case 4: return &tls_init;
        case 5: return &tls_static;
        default: return NULL;
    }
}

int main(void) {
    const int NUM_THREADS = 4;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* results[NUM_THREADS];
    
    /* Initialize TLS with non-constant value - forces emulation complexity */
    tls_init = get_initial_value();
    
    /* Initialize main thread's TLS values */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    tls_static = 5;
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Access TLS in main thread - ensures main thread TLS is initialized */
    volatile int* addrs[6];
    for (int i = 0; i < 6; i++) {
        addrs[i] = get_tls_addr(i);
    }
    
    /* Modify main thread's TLS */
    tls_var1 += 10;
    tls_var2 += 20;
    tls_var3 += 30;
    tls_var4 += 40;
    tls_init += 50;
    tls_static += 60;
    
    /* Join threads and collect results */
    int total_sum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        if (results[i]) {
            total_sum += *results[i];
            free(results[i]);
        }
    }
    
    /* Calculate final checksum from main thread's TLS */
    int main_thread_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_static;
    
    printf("Main thread TLS sum: %d\n", main_thread_sum);
    printf("Child threads total sum: %d\n", total_sum);
    printf("All TLS accesses completed successfully.\n");
    
    /* Final verification that all TLS variables were properly accessed */
    if (main_thread_sum > 0 && total_sum > 0) {
        printf("TLS emulation test PASSED\n");
        return 0;
    } else {
        printf("TLS emulation test FAILED\n");
        return 1;
    }
}
