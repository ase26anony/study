#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    static atomic_int counter = 0;
    return atomic_fetch_add(&counter, 1) + 1;
}

/* TLS variables with various attributes to trigger declaration cloning */

/* 1. Extern TLS with hidden visibility - will set DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - will set DECL_WEAK */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS - will set TREE_PUBLIC, DECL_EXTERNAL, DECL_COMMON */
__thread int tls_var3 __attribute__((used));

/* 4. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init __attribute__((used)) = get_initial_value();

/* 5. For platforms supporting dllimport (Windows/MinGW) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dll __attribute__((used));
#else
    /* Simulate with visibility on non-Windows */
    __thread int tls_dll __attribute__((visibility("default"), used));
#endif

/* Define the extern TLS variable */
__thread int tls_var1 = 42;

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use of all TLS variables by taking addresses */
    volatile int* addrs[] = {
        &tls_var1,
        &tls_var2, 
        &tls_var3,
        &tls_init,
        &tls_dll
    };
    
    /* Prevent optimization from removing TLS accesses */
    if (thread_id % 2 == 0) {
        /* Write unique values to each TLS variable */
        tls_var1 = 1000 + thread_id;
        tls_var2 = 2000 + thread_id;
        tls_var3 = 3000 + thread_id;
        tls_init = 4000 + thread_id;
        tls_dll = 5000 + thread_id;
    } else {
        /* Alternative access pattern */
        tls_var1 = 100 + thread_id;
        tls_var2 = 200 + thread_id;
        tls_var3 = 300 + thread_id;
        tls_init = 400 + thread_id;
        tls_dll = 500 + thread_id;
    }
    
    /* Read back and verify (simulating real use) */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_init + tls_dll;
    
    /* Conditional use that prevents dead code elimination */
    if (sum > 0) {
        int* result = malloc(sizeof(int));
        *result = sum + thread_id;
        return result;
    }
    
    return NULL;
}

int main(void) {
    pthread_t threads[3];
    int thread_ids[3];
    void* thread_results[3];
    int final_checksum = 0;
    
    /* Access TLS from main thread first (forces initialization) */
    tls_var1 = 999;
    tls_var2 = 888;
    tls_var3 = 777;
    tls_init = 666;
    tls_dll = 555;
    
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
        if (pthread_join(threads[i], &thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        
        if (thread_results[i]) {
            final_checksum += *(int*)thread_results[i];
            free(thread_results[i]);
        }
    }
    
    /* Access TLS again in main thread (different instance) */
    int main_thread_sum = tls_var1 + tls_var2 + tls_var3 + tls_init + tls_dll;
    final_checksum += main_thread_sum;
    
    /* Print result to ensure execution */
    printf("TLS test completed. Final checksum: %d\n", final_checksum);
    printf("Main thread TLS sum: %d\n", main_thread_sum);
    
    /* Verify TLS variables have thread-local values */
    printf("Main thread TLS values: var1=%d, var2=%d, var3=%d, init=%d, dll=%d\n",
           tls_var1, tls_var2, tls_var3, tls_init, tls_dll);
    
    return 0;
}
