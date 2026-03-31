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
    /* For Windows/MinGW */
    __declspec(dllimport) __thread int tls_var4;
#else
    /* For non-Windows, use visibility as proxy */
    __thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer */
__thread int tls_init = 0;

/* Define the extern TLS variable */
__thread int tls_var1 = 10;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    volatile int* addr1 = &tls_var1;
    volatile int* addr2 = &tls_var2;
    volatile int* addr3 = &tls_var3;
    volatile int* addr4 = &tls_var4;
    volatile int* addr_init = &tls_init;
    
    /* Write thread-specific values */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    
    /* Conditional access to prevent optimization */
    if (thread_id % 2 == 0) {
        tls_init = some_function() + thread_id;
    } else {
        tls_init = thread_id;
    }
    
    /* Read back and verify (simulate real use) */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init;
    
    /* Return the sum for verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

int main(void) {
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* thread_results[NUM_THREADS];
    
    /* Initialize non-constant TLS variable in main thread */
    tls_init = some_function();
    
    /* Initialize main thread's TLS variables */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
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
    
    /* Access TLS from main thread (different instance) */
    int main_thread_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init;
    
    /* Print results to ensure execution */
    printf("Main thread TLS sum: %d\n", main_thread_sum);
    printf("All threads TLS sum total: %d\n", total_sum);
    printf("Expected pattern: each thread's sum = (1000+2000+3000+4000) + thread_id*5 + tls_init\n");
    
    /* Force external linkage references */
    extern void force_external_refs(void);
    force_external_refs();
    
    return 0;
}

/* Force external references to prevent optimization */
void force_external_refs(void) {
    /* This function forces the compiler to consider all TLS variables as used */
    volatile int dummy = 0;
    
    /* Conditional compilation to use all variables */
    #ifdef FORCE_USE
    dummy += tls_var1;
    dummy += tls_var2;
    dummy += tls_var3;
    dummy += tls_var4;
    dummy += tls_init;
    #endif
    
    /* Prevent unused variable warning */
    (void)dummy;
}
