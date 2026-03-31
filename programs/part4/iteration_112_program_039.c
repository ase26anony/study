/* TLS test program targeting tree-emutls.cc attribute copying logic */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    return 42;
}

/* 1. Extern TLS with hidden visibility - will set DECL_VISIBILITY_SPECIFIED */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - will set DECL_WEAK */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS - will set DECL_COMMON and TREE_PUBLIC */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation (for MinGW/Windows targets) */
/* Using dllimport attribute if supported */
#ifdef __MINGW32__
__declspec(dllimport) __thread int tls_var4 __attribute__((used));
#else
/* On non-Windows, use a visibility attribute to ensure DECL_VISIBILITY is set */
__thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Another extern TLS with explicit external linkage */
extern __thread int tls_extern __attribute__((used));

/* Define the extern TLS variables */
__thread int tls_var1 = 10;
__thread int tls_extern = 50;

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Access all TLS variables in a way that prevents optimization */
    tls_var1 = 100 + thread_id;
    tls_var2 = 200 + thread_id;
    tls_var3 = 300 + thread_id;
    tls_var4 = 400 + thread_id;
    tls_init = 500 + thread_id;
    tls_extern = 600 + thread_id;
    
    /* Read them back and verify (prevents dead store elimination) */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_extern;
    
    /* Conditional use based on thread ID - ensures ODR-use */
    if (thread_id % 2 == 0) {
        /* Take address of TLS variables (forces TLS machinery) */
        int* ptr1 = &tls_var1;
        int* ptr2 = &tls_var2;
        int* ptr3 = &tls_var3;
        (void)ptr1; (void)ptr2; (void)ptr3; /* Suppress unused warnings */
    }
    
    /* Return the sum for verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

int main(void) {
    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};
    void* thread_results[3];
    
    /* Initialize TLS with non-constant value - forces emulation complexity */
    tls_init = get_initial_value();
    
    /* Access TLS in main thread first */
    tls_var1 = 999;
    tls_var3 = 888;
    
    /* Create multiple threads to force TLS emulation setup */
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Join threads and collect results */
    int total_sum = 0;
    for (int i = 0; i < 3; i++) {
        if (pthread_join(threads[i], &thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *(int*)thread_results[i];
        free(thread_results[i]);
    }
    
    /* Access TLS again in main thread after threads have run */
    int main_tls_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_extern;
    
    /* Print results to ensure all code paths are executed */
    printf("Main thread TLS sum: %d\n", main_tls_sum);
    printf("Total thread sums: %d\n", total_sum);
    
    /* Final verification: access all TLS variables one more time */
    volatile int final_check = 
        (tls_var1 != 0) + (tls_var2 != 0) + (tls_var3 != 0) + 
        (tls_var4 != 0) + (tls_init != 0) + (tls_extern != 0);
    
    printf("TLS variables initialized: %d/6\n", final_check);
    
    return 0;
}
