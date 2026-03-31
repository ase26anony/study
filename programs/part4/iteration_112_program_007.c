#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Helper function for non-constant initializer */
static int some_function(void) {
    return 42;
}

/* TLS variables with various attributes to trigger declaration cloning */

/* 1. Extern TLS with hidden visibility - should set DECL_VISIBILITY */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - should set DECL_WEAK */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS (implicitly extern) - should set DECL_COMMON */
__thread int tls_var3 __attribute__((used));

/* 4. TLS with non-constant initializer - complicates initialization */
__thread int tls_init = some_function();

/* 5. Static TLS - different linkage */
static __thread int tls_static __attribute__((used));

/* For Windows/MinGW targets, uncomment this:
   __declspec(dllimport) __thread int tls_dllimport __attribute__((used));
*/

/* Define the extern TLS variable */
__thread int tls_var1 = 100;

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    volatile int* addr1 = &tls_var1;
    volatile int* addr2 = &tls_var2;
    volatile int* addr3 = &tls_var3;
    volatile int* addr4 = &tls_init;
    volatile int* addr5 = &tls_static;
    
    /* Prevent optimization by using addresses */
    (void)addr1;
    (void)addr2;
    (void)addr3;
    (void)addr4;
    (void)addr5;
    
    /* Assign thread-specific values to TLS variables */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_init = 4000 + thread_id;
    tls_static = 5000 + thread_id;
    
    /* Conditional access to prevent dead code elimination */
    if (thread_id % 2 == 0) {
        tls_var1 *= 2;
    } else {
        tls_var2 += 100;
    }
    
    /* Verify values by reading them back */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_init + tls_static;
    
    /* Return the sum for verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

int main(void) {
    pthread_t threads[3];
    int thread_ids[3];
    int* results[3];
    int total_sum = 0;
    
    /* Initialize TLS variable in main thread */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_init = 4;
    tls_static = 5;
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < 3; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Join threads and collect results */
    for (int i = 0; i < 3; i++) {
        if (pthread_join(threads[i], (void**)&results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *results[i];
        free(results[i]);
    }
    
    /* Access TLS variables in main thread (has its own instance) */
    int main_tls_sum = tls_var1 + tls_var2 + tls_var3 + tls_init + tls_static;
    
    /* Print results to ensure execution */
    printf("Main thread TLS sum: %d\n", main_tls_sum);
    printf("Sum of all thread results: %d\n", total_sum);
    
    /* Verify that main thread values are unchanged from initialization */
    if (main_tls_sum == (1 + 2 + 3 + 4 + 5)) {
        printf("TLS isolation working correctly - main thread values preserved\n");
    } else {
        printf("Warning: TLS isolation may not be working properly\n");
    }
    
    return 0;
}
