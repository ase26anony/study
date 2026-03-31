#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/* Helper function for non-constant initializer */
static int some_function(void) {
    static atomic_int counter = 0;
    return atomic_fetch_add(&counter, 1) + 42;
}

/* TLS variables with various attributes to trigger declaration cloning */

/* 1. Extern TLS with hidden visibility - will set DECL_VISIBILITY_SPECIFIED */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - will set DECL_WEAK */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS - will set DECL_COMMON and TREE_PUBLIC */
__thread int tls_var3 __attribute__((used));

/* 4. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init __attribute__((used)) = some_function();

/* 5. Static TLS - different linkage */
static __thread int tls_static __attribute__((used));

/* For Windows/MinGW targets, uncomment this:
__declspec(dllimport) extern __thread int tls_dllimport __attribute__((used));
*/

/* Define the extern TLS variable */
__thread int tls_var1 = 1000;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    volatile int* addr1 = &tls_var1;
    volatile int* addr2 = &tls_var2;
    volatile int* addr3 = &tls_var3;
    volatile int* addr4 = &tls_init;
    volatile int* addr5 = &tls_static;
    
    /* Prevent optimization by using conditional access */
    if (thread_id % 2 == 0) {
        tls_var1 = 100 + thread_id;
        tls_var2 = 200 + thread_id;
        tls_var3 = 300 + thread_id;
        tls_init = 400 + thread_id;
        tls_static = 500 + thread_id;
    } else {
        tls_var1 = 1000 + thread_id;
        tls_var2 = 2000 + thread_id;
        tls_var3 = 3000 + thread_id;
        tls_init = 4000 + thread_id;
        tls_static = 5000 + thread_id;
    }
    
    /* Complex conditional to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0: sum += tls_var1; break;
            case 1: sum += tls_var2; break;
            case 2: sum += tls_var3; break;
            case 3: sum += tls_init; break;
            default: sum += tls_static; break;
        }
    }
    
    /* Return the sum computed from TLS variables */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

int main(void) {
    const int NUM_THREADS = 4;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* results[NUM_THREADS];
    
    /* Initialize thread IDs */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
    }
    
    /* Create threads - forces TLS emulation setup */
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Access TLS in main thread too */
    tls_var1 = 9999;
    tls_var2 = 8888;
    tls_var3 = 7777;
    tls_init = 6666;
    tls_static = 5555;
    
    /* Join threads and collect results */
    int total_sum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *results[i];
        free(results[i]);
    }
    
    /* Add main thread's TLS values to total */
    total_sum += tls_var1 + tls_var2 + tls_var3 + tls_init + tls_static;
    
    /* Print result to ensure execution */
    printf("Total checksum from all TLS accesses: %d\n", total_sum);
    printf("Main thread TLS values: %d, %d, %d, %d, %d\n", 
           tls_var1, tls_var2, tls_var3, tls_init, tls_static);
    
    /* Verify that TLS variables have thread-local behavior */
    if (tls_var1 == 9999) {
        printf("TLS emulation appears functional (main thread value preserved)\n");
    }
    
    return 0;
}
