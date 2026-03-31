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

/* 1. Extern TLS with hidden visibility - will set DECL_VISIBILITY */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - will set DECL_WEAK */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS - will set TREE_PUBLIC, DECL_COMMON */
__thread int tls_var3 __attribute__((used));

/* 4. TLS with DLL import simulation - will set DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* On non-Windows, use a GCC attribute that might map to DECL_DLLIMPORT_P */
__thread int tls_var4 __attribute__((dllimport));
#endif

/* 5. TLS with non-constant initializer - complicates initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

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
    
    /* Prevent optimization by using the addresses */
    (void)addr1; (void)addr2; (void)addr3; (void)addr4; (void)addr5;
    
    /* Write thread-specific values to TLS variables */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_init = 5000 + thread_id;
    
    /* Read back and verify (simulating real use) */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init;
    
    /* Return the sum as verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

int main(void) {
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* results[NUM_THREADS];
    
    /* Initialize tls_init with non-constant value */
    tls_init = some_function();
    
    /* Force external linkage reference for tls_var1 */
    extern int tls_var1;
    (void)tls_var1;  /* Reference to force external linkage handling */
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Main thread also accesses TLS variables */
    tls_var1 = 999;
    tls_var2 = 1999;
    tls_var3 = 2999;
    tls_var4 = 3999;
    tls_init = 4999;
    
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
    
    /* Add main thread's TLS values */
    total_sum += tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init;
    
    printf("Total sum from all threads and main: %d\n", total_sum);
    printf("Main thread TLS values: %d, %d, %d, %d, %d\n", 
           tls_var1, tls_var2, tls_var3, tls_var4, tls_init);
    
    /* Expected calculation for verification:
       Each thread i sets:
         tls_var1 = 1000 + i
         tls_var2 = 2000 + i
         tls_var3 = 3000 + i
         tls_var4 = 4000 + i
         tls_init = 5000 + i
         Sum = 15000 + 5*i
       
       Threads 0,1,2 sums: 15000 + 15005 + 15010 = 45015
       Main thread sets all to 999,1999,2999,3999,4999 = 14995
       Total = 45015 + 14995 = 60010
    */
    
    return 0;
}
