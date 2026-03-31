#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Helper function for non-constant initializer */
int some_function(void) {
    return 42;
}

/* 1. Extern TLS with hidden visibility */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS (implicitly extern) */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_var4;
#else
    /* Simulate DLL import attribute for non-Windows */
    __thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer */
__thread int tls_init = 0;  /* Will be initialized in main */

/* 6. Another TLS with explicit visibility */
__thread int tls_var5 __attribute__((visibility("protected"), used));

/* Define the extern TLS variable */
__thread int tls_var1 = 100;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Access all TLS variables in a way that prevents optimization */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_var5 = 5000 + thread_id;
    tls_init = 6000 + thread_id;
    
    /* Take addresses to force ODR-use */
    int* addrs[] = {
        &tls_var1,
        &tls_var2,
        &tls_var3,
        &tls_var4,
        &tls_var5,
        &tls_init
    };
    
    /* Conditional access based on thread_id */
    volatile int sum = 0;
    for (int i = 0; i < 6; i++) {
        if (thread_id % 2 == i % 2) {
            sum += *addrs[i];
        }
    }
    
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
    tls_init = some_function();
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Main thread accesses its own TLS instances */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    tls_var5 = 5;
    
    /* Take addresses in main thread too */
    volatile int* main_addrs[] = {
        &tls_var1, &tls_var2, &tls_var3,
        &tls_var4, &tls_var5, &tls_init
    };
    
    /* Join threads and collect results */
    int total_sum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *thread_results[i];
        free(thread_results[i]);
    }
    
    /* Calculate main thread's sum */
    int main_sum = 0;
    for (int i = 0; i < 6; i++) {
        main_sum += *main_addrs[i];
    }
    
    /* Print results to prove TLS machinery was used */
    printf("Main thread TLS sum: %d\n", main_sum);
    printf("All worker threads TLS sum: %d\n", total_sum);
    printf("Total checksum: %d\n", main_sum + total_sum);
    
    /* Additional verification: each thread should have different TLS values */
    printf("\nTLS verification:\n");
    printf("Main thread tls_var1: %d (should be 1)\n", tls_var1);
    printf("Main thread tls_init: %d (should be 42)\n", tls_init);
    
    return 0;
}
