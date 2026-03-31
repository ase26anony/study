#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    static atomic_int counter = 0;
    return atomic_fetch_add(&counter, 1) + 42;
}

/* TLS variables with various attributes to trigger declaration cloning */

/* 1. Extern TLS with hidden visibility - will set DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - will set DECL_WEAK */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS - will set TREE_PUBLIC, DECL_EXTERNAL, DECL_COMMON */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* Simulate DLL import attribute for non-Windows */
__thread int tls_var4 __attribute__((dllimport));
#endif

/* 5. TLS with non-constant initializer - complicates initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* Define the extern TLS variable */
__thread int tls_var1 = 100;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    volatile int* addr1 = &tls_var1;
    volatile int* addr2 = &tls_var2;
    volatile int* addr3 = &tls_var3;
    volatile int* addr4 = &tls_var4;
    volatile int* addr5 = &tls_init;
    
    /* Prevent optimization by using addresses */
    (void)addr1; (void)addr2; (void)addr3; (void)addr4; (void)addr5;
    
    /* Assign unique values to each TLS variable */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_init = 5000 + thread_id;
    
    /* Conditional access to prevent dead code elimination */
    if (thread_id % 2 == 0) {
        tls_var1 += 1;
        tls_var3 += 1;
    } else {
        tls_var2 += 2;
        tls_var4 += 2;
    }
    
    /* Verify values by reading them back */
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
    
    /* Initialize non-constant TLS variable */
    tls_init = get_initial_value();
    
    /* Force reference to all TLS variables in main thread */
    volatile int* addrs[] = {&tls_var1, &tls_var2, &tls_var3, &tls_var4, &tls_init};
    for (int i = 0; i < 5; i++) {
        (void)addrs[i];
    }
    
    /* Set main thread's TLS values */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
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
    
    /* Access TLS in main thread after threads have run */
    int main_thread_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init;
    
    /* Print results to ensure execution */
    printf("Main thread TLS sum: %d\n", main_thread_sum);
    printf("All threads TLS sum total: %d\n", total_sum);
    printf("Expected main thread values: tls_var1=%d, tls_var2=%d, tls_var3=%d, tls_var4=%d, tls_init=%d\n",
           tls_var1, tls_var2, tls_var3, tls_var4, tls_init);
    
    /* Verification - each thread should have different TLS values */
    if (main_thread_sum == (1 + 2 + 3 + 4 + tls_init)) {
        printf("TLS emulation test PASSED - main thread preserved its values\n");
    } else {
        printf("TLS emulation test FAILED - main thread values corrupted\n");
    }
    
    return 0;
}
