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
/* For MinGW/Windows targets, use: __declspec(dllimport) __thread int tls_var4 */
/* For Linux/GCC, we'll use a visibility attribute to simulate similar handling */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_var4;
#else
    /* Use visibility to ensure DECL_VISIBILITY_SPECIFIED is set */
    __thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Another extern TLS with explicit visibility */
extern __thread int tls_var5 __attribute__((visibility("protected"), used));

/* Define the extern TLS variables */
__thread int tls_var1 = 10;
__thread int tls_var5 = 50;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Take addresses to force ODR-use */
    int* addr1 = &tls_var1;
    int* addr2 = &tls_var2;
    int* addr3 = &tls_var3;
    int* addr4 = &tls_var4;
    int* addr5 = &tls_var5;
    int* addr_init = &tls_init;
    
    /* Assign unique values based on thread ID */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_var5 = 5000 + thread_id;
    tls_init = 6000 + thread_id;
    
    /* Conditional access to prevent optimization */
    if (thread_id % 2 == 0) {
        tls_var1 *= 2;
        tls_var3 += thread_id;
    } else {
        tls_var2 /= 2;
        tls_var4 -= thread_id;
    }
    
    /* Complex conditional to ensure all variables are used */
    volatile int sum = 0;
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0: sum += tls_var1; break;
            case 1: sum += tls_var2; break;
            case 2: sum += tls_var3; break;
            case 3: sum += tls_var4; break;
            case 4: sum += tls_var5; break;
            case 5: sum += tls_init; break;
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
    
    /* Initialize TLS variables in main thread */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    tls_var5 = 5;
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Access TLS in main thread (has its own instance) */
    int main_sum = 0;
    main_sum += tls_var1;
    main_sum += tls_var2;
    main_sum += tls_var3;
    main_sum += tls_var4;
    main_sum += tls_var5;
    main_sum += tls_init;
    
    printf("Main thread TLS sum: %d\n", main_sum);
    
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
    printf("Total sum from all threads: %d\n", total_sum);
    
    /* Force external linkage references through function calls */
    void force_references(void) {
        /* These force the compiler to consider the TLS variables as used */
        volatile int dummy = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
        (void)dummy;
    }
    
    force_references();
    
    return 0;
}
