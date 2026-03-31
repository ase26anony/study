#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    return 42;
}

/* 1. Extern TLS with hidden visibility and used attribute */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak));

/* 3. Public common TLS (implicitly extern) */
__thread int tls_var3;

/* 4. Static TLS with DLL import simulation */
#ifdef _WIN32
    /* For Windows/MinGW */
    __declspec(dllimport) __thread int tls_var4;
#else
    /* For non-Windows, use dllimport-like attribute if supported */
    #ifdef __GNUC__
        __thread int tls_var4 __attribute__((dllimport));
    #else
        __thread int tls_var4;
    #endif
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Another TLS with explicit visibility */
__thread int tls_visible __attribute__((visibility("default")));

/* Define the extern TLS variable */
__thread int tls_var1 = 100;

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Take addresses to ensure ODR-use and prevent optimization */
    int* addr1 = &tls_var1;
    int* addr2 = &tls_var2;
    int* addr3 = &tls_var3;
    int* addr4 = &tls_var4;
    int* addr_init = &tls_init;
    int* addr_visible = &tls_visible;
    
    /* Write thread-specific values */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_init = 5000 + thread_id;
    tls_visible = 6000 + thread_id;
    
    /* Conditional access to prevent dead code elimination */
    volatile int sum = 0;
    if (thread_id % 2 == 0) {
        sum += tls_var1 + tls_var2;
    } else {
        sum += tls_var3 + tls_var4;
    }
    
    /* Always use tls_init and tls_visible */
    sum += tls_init + tls_visible;
    
    /* Return the sum as verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

int main(void) {
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* thread_results[NUM_THREADS];
    
    /* Initialize non-constant TLS variable */
    tls_init = get_initial_value();
    
    /* Initialize main thread's TLS values */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    tls_visible = 6;
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Wait for threads and collect results */
    int total_sum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *thread_results[i];
        free(thread_results[i]);
    }
    
    /* Access TLS from main thread (different instance) */
    int main_tls_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_visible;
    
    /* Print results to ensure execution */
    printf("Main thread TLS sum: %d\n", main_tls_sum);
    printf("All threads result sum: %d\n", total_sum);
    printf("Expected main thread values: 1, 2, 3, 4, 42, 6\n");
    
    /* Verify by checking main thread's TLS values */
    if (tls_var1 == 1 && tls_var2 == 2 && tls_var3 == 3 && 
        tls_var4 == 4 && tls_init == 42 && tls_visible == 6) {
        printf("SUCCESS: TLS emulation appears to be working correctly\n");
        return 0;
    } else {
        printf("ERROR: TLS values corrupted\n");
        return 1;
    }
}
