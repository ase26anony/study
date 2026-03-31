#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

/* Helper function for non-constant initializer */
int get_initial_value(void) {
    return 42;
}

/* Function to prevent optimization */
void use_value(int val) {
    /* Prevent dead code elimination */
    volatile int dummy = val;
    (void)dummy;
}

/* ========== TLS VARIABLES WITH VARIOUS ATTRIBUTES ========== */

/* 1. Extern TLS with hidden visibility - will trigger DECL_VISIBILITY copying */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - will trigger DECL_WEAK copying */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS (implicitly extern) - will trigger DECL_COMMON copying */
__thread int tls_var3 __attribute__((used));

/* 4. TLS with non-constant initializer - complicates initialization */
__thread int tls_init __attribute__((used)) = get_initial_value();

/* 5. Static TLS - ensures TREE_PUBLIC/DECL_EXTERNAL handling */
static __thread int tls_static __attribute__((used));

/* For Windows/MinGW compatibility - simulate DLL import attribute */
#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
#else
    /* GCC extension for dllimport simulation */
    #define DLL_IMPORT __attribute__((dllimport))
#endif

/* 6. TLS with DLL import attribute - triggers DECL_DLLIMPORT_P */
extern DLL_IMPORT __thread int tls_dll __attribute__((used));

/* Define the extern TLS variables (simulating separate translation unit) */
__thread int tls_var1 = 100;
__thread int tls_var2 = 200;
__thread int tls_var3 = 300;
__thread int tls_dll = 400;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    volatile int* addr1 = &tls_var1;
    volatile int* addr2 = &tls_var2;
    volatile int* addr3 = &tls_var3;
    volatile int* addr4 = &tls_init;
    volatile int* addr5 = &tls_static;
    volatile int* addr6 = &tls_dll;
    
    /* Assign thread-specific values */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_init = 4200 + thread_id;  /* Modify the initialized value */
    tls_static = 5000 + thread_id;
    tls_dll = 6000 + thread_id;
    
    /* Conditional access to prevent optimization */
    if (thread_id % 2 == 0) {
        tls_var1 *= 2;
        tls_dll += 100;
    } else {
        tls_var2 /= 2;
        tls_init -= 50;
    }
    
    /* Read back and verify values */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_init + tls_static + tls_dll;
    
    /* Use the values to prevent optimization */
    use_value(tls_var1);
    use_value(tls_var2);
    use_value(tls_var3);
    use_value(tls_init);
    use_value(tls_static);
    use_value(tls_dll);
    
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
    
    /* Initialize thread IDs */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;
    }
    
    /* Create threads - each will have its own TLS instances */
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Main thread also accesses TLS variables */
    tls_var1 = 999;
    tls_var2 = 888;
    tls_var3 = 777;
    tls_init = 666;
    tls_static = 555;
    tls_dll = 444;
    
    /* Force address-taking in main thread too */
    volatile int* main_addrs[] = {
        &tls_var1, &tls_var2, &tls_var3,
        &tls_init, &tls_static, &tls_dll
    };
    (void)main_addrs;  /* Suppress unused warning */
    
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
    
    /* Calculate main thread's TLS sum */
    int main_sum = tls_var1 + tls_var2 + tls_var3 + tls_init + tls_static + tls_dll;
    
    /* Print results to ensure execution */
    printf("Main thread TLS sum: %d\n", main_sum);
    printf("All worker threads TLS sum: %d\n", total_sum);
    printf("Program completed successfully.\n");
    
    /* Final verification - access all TLS variables one more time */
    if (tls_var1 != 999 || tls_var2 != 888 || tls_var3 != 777 ||
        tls_init != 666 || tls_static != 555 || tls_dll != 444) {
        printf("Warning: TLS values may have been corrupted!\n");
    }
    
    return 0;
}
