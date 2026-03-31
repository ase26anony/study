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

/* 4. Static TLS (internal linkage) */
static __thread int tls_var4 __attribute__((used));

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init __attribute__((used)) = get_initial_value();

/* 6. DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dll __attribute__((used));
#else
/* Simulate with visibility attribute */
__thread int tls_dll __attribute__((visibility("default"), used));
#endif

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
    volatile int* addr6 = &tls_dll;
    
    /* Prevent optimization by using addresses */
    (void)addr1; (void)addr2; (void)addr3; (void)addr4; (void)addr5; (void)addr6;
    
    /* Assign thread-specific values */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_dll = 5000 + thread_id;
    
    /* Complex conditional access to prevent optimization */
    if (thread_id % 2 == 0) {
        tls_init = 6000 + thread_id;
    } else {
        tls_init = 7000 + thread_id;
    }
    
    /* Verify values (creates data dependencies) */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_dll;
    
    /* Return the sum for verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

int main(void) {
    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};
    int* results[3];
    
    /* Access TLS variables in main thread first */
    tls_var1 = 999;
    tls_var2 = 1999;
    tls_var3 = 2999;
    tls_var4 = 3999;
    tls_dll = 4999;
    tls_init = get_initial_value() + 100;
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Join threads and collect results */
    int total_sum = 0;
    for (int i = 0; i < 3; i++) {
        if (pthread_join(threads[i], (void**)&results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *results[i];
        free(results[i]);
    }
    
    /* Access TLS in main thread again after threads */
    int main_thread_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_dll;
    
    /* Print results to ensure execution */
    printf("Main thread TLS sum: %d\n", main_thread_sum);
    printf("Sum of all thread results: %d\n", total_sum);
    printf("Expected main thread values: tls_var1=%d, tls_var2=%d, tls_var3=%d\n", 
           tls_var1, tls_var2, tls_var3);
    
    /* Force compiler to keep all TLS variables by taking addresses at end */
    volatile int* keep_alive1 = &tls_var1;
    volatile int* keep_alive2 = &tls_var2;
    volatile int* keep_alive3 = &tls_var3;
    volatile int* keep_alive4 = &tls_var4;
    volatile int* keep_alive5 = &tls_init;
    volatile int* keep_alive6 = &tls_dll;
    
    (void)keep_alive1; (void)keep_alive2; (void)keep_alive3;
    (void)keep_alive4; (void)keep_alive5; (void)keep_alive6;
    
    return 0;
}
