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
    /* For non-Windows, use visibility attribute to ensure DECL_VISIBILITY_SPECIFIED is set */
    __thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Static TLS variable (not extern) to affect TREE_PUBLIC/DECL_EXTERNAL */
static __thread int tls_static __attribute__((used));

/* Define the extern TLS variable */
__thread int tls_var1 = 100;

/* Define the weak TLS variable */
__thread int tls_var2 = 200;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    int* addr1 = &tls_var1;
    int* addr2 = &tls_var2;
    int* addr3 = &tls_var3;
    int* addr4 = &tls_var4;
    int* addr_init = &tls_init;
    int* addr_static = &tls_static;
    
    /* Prevent optimization by using volatile */
    volatile int* v1 = addr1;
    volatile int* v2 = addr2;
    volatile int* v3 = addr3;
    volatile int* v4 = addr4;
    volatile int* v_init = addr_init;
    volatile int* v_static = addr_static;
    
    /* Assign thread-specific values */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_init = 5000 + thread_id;
    tls_static = 6000 + thread_id;
    
    /* Conditional access to prevent dead code elimination */
    if (thread_id % 2 == 0) {
        /* Even threads read and modify */
        tls_var1 += 1;
        tls_var2 += 2;
    } else {
        /* Odd threads read and modify differently */
        tls_var3 += 3;
        tls_var4 += 4;
    }
    
    /* Complex conditional to ensure all variables are used */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        if (i == 0) {
            sum += tls_var1 + tls_var2;
        } else {
            sum += tls_var3 + tls_var4 + tls_init + tls_static;
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
    
    /* Access TLS in main thread (has its own instance) */
    int main_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_static;
    
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
    
    /* Print results to ensure execution */
    printf("Main thread TLS sum: %d\n", main_sum);
    printf("Total sum from all threads: %d\n", total_sum);
    
    /* Additional verification by taking addresses in main thread */
    int* addrs[] = {&tls_var1, &tls_var2, &tls_var3, &tls_var4, &tls_init, &tls_static};
    printf("TLS addresses in main thread:\n");
    for (int i = 0; i < 6; i++) {
        printf("  Variable %d: %p\n", i + 1, (void*)addrs[i]);
    }
    
    return 0;
}
