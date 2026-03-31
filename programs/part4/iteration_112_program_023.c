/* test_tls_emulation.c - Program to trigger TLS declaration cloning with various attributes */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    return 42;
}

/* 1. Extern TLS with hidden visibility */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS (implicitly extern, may become common) */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation (for platforms supporting it) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_var4;
#else
    /* Simulate DLL import attribute for non-Windows */
    __thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer to engage emulation logic */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Another TLS with explicit visibility */
__thread int tls_var5 __attribute__((visibility("protected"), used));

/* Define the extern TLS variable */
__thread int tls_var1 = 100;

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    volatile int* addrs[6];
    addrs[0] = &tls_var1;
    addrs[1] = &tls_var2;
    addrs[2] = &tls_var3;
    addrs[3] = &tls_var4;
    addrs[4] = &tls_init;
    addrs[5] = &tls_var5;
    
    /* Prevent optimization from removing TLS accesses */
    if (thread_id % 2 == 0) {
        /* Write unique values to each TLS variable */
        tls_var1 = 1000 + thread_id;
        tls_var2 = 2000 + thread_id;
        tls_var3 = 3000 + thread_id;
        tls_var4 = 4000 + thread_id;
        tls_init = 5000 + thread_id;
        tls_var5 = 6000 + thread_id;
    } else {
        /* Different access pattern for odd threads */
        tls_var1 = 100 + thread_id;
        tls_var2 = 200 + thread_id;
        tls_var3 = 300 + thread_id;
        tls_var4 = 400 + thread_id;
        tls_init = 500 + thread_id;
        tls_var5 = 600 + thread_id;
    }
    
    /* Read back and verify values */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_var5;
    
    /* Conditional use of addresses to prevent dead code elimination */
    if (addrs[0] != NULL) {
        sum += *addrs[0];
    }
    
    /* Return the thread-local sum */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

/* Function that references TLS variables in a way that can't be optimized out */
static void use_tls_variables(void) {
    volatile static int counter = 0;
    
    /* Conditional access pattern */
    if (counter++ % 3 == 0) {
        tls_var1++;
    } else if (counter % 3 == 1) {
        tls_var2++;
    } else {
        tls_var3++;
    }
    
    /* Mix in other variables */
    tls_var4 = tls_var1 + tls_var2;
    tls_var5 = tls_var3 + tls_var4;
    tls_init = tls_var5 + counter;
}

int main(void) {
    const int NUM_THREADS = 4;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* thread_results[NUM_THREADS];
    
    /* Initialize TLS with non-constant value to engage emulation */
    tls_init = get_initial_value();
    
    /* Use TLS variables in main thread before creating other threads */
    use_tls_variables();
    
    /* Create multiple threads to force TLS emulation setup */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Access TLS in main thread (has its own instance) */
    tls_var1 = 9999;
    tls_var2 = 8888;
    tls_var3 = 7777;
    tls_var4 = 6666;
    tls_var5 = 5555;
    tls_init = 4444;
    
    /* Join all threads and collect results */
    int total_sum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *thread_results[i];
        free(thread_results[i]);
    }
    
    /* Calculate checksum from main thread's TLS values */
    int main_thread_sum = tls_var1 + tls_var2 + tls_var3 + 
                         tls_var4 + tls_var5 + tls_init;
    
    /* Final verification output */
    printf("Main thread TLS sum: %d\n", main_thread_sum);
    printf("All child threads TLS sum: %d\n", total_sum);
    printf("Total checksum: %d\n", main_thread_sum + total_sum);
    
    /* Additional use of TLS variables to ensure they're not optimized away */
    if (main_thread_sum > 0) {
        tls_var1 = main_thread_sum % 1000;
        tls_var2 = total_sum % 1000;
    }
    
    return 0;
}
