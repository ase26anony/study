/* test_tls_emulation.c - Program to trigger TLS declaration cloning logic */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    return 42;
}

/* 1. Extern TLS with hidden visibility - will have DECL_VISIBILITY set */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - will have DECL_WEAK set */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS - will have DECL_COMMON and TREE_PUBLIC set */
__thread int tls_var3 __attribute__((used));

/* 4. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init __attribute__((used)) = get_initial_value();

/* 5. Static TLS - will have different linkage */
static __thread int tls_static __attribute__((used));

/* For Windows/MinGW compatibility */
#ifdef _WIN32
    #define DLL_IMPORT __declspec(dllimport)
#else
    /* Simulate DLL import attribute for non-Windows */
    #define DLL_IMPORT __attribute__((dllimport))
#endif

/* 6. TLS with DLL import attribute - for DECL_DLLIMPORT_P */
extern DLL_IMPORT __thread int tls_dllimport __attribute__((used));

/* Define the extern TLS variables */
__thread int tls_var1 = 100;
__thread int tls_var2 = 200;
__thread int tls_var3 = 300;
__thread int tls_dllimport = 400;

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    volatile int* addrs[] = {
        &tls_var1,
        &tls_var2, 
        &tls_var3,
        &tls_init,
        &tls_static,
        &tls_dllimport
    };
    
    /* Prevent optimization by using addresses */
    (void)addrs;
    
    /* Assign thread-specific values */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_init = 4000 + thread_id;
    tls_static = 5000 + thread_id;
    tls_dllimport = 6000 + thread_id;
    
    /* Read back and verify (prevent dead code elimination) */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_init + tls_static + tls_dllimport;
    
    /* Return the sum as verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

/* Function that conditionally uses TLS variables */
static void conditional_tls_use(int condition) {
    if (condition) {
        /* Force use of TLS variables in different basic blocks */
        tls_var1++;
        tls_var2++;
    } else {
        tls_var3++;
        tls_init++;
    }
    
    /* Always use these */
    tls_static++;
    tls_dllimport++;
}

int main(void) {
    pthread_t threads[3];
    int thread_ids[3];
    int* results[3];
    int final_sum = 0;
    
    /* Initialize main thread's TLS values */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_init = get_initial_value(); /* Re-initialize */
    tls_static = 5;
    tls_dllimport = 6;
    
    /* Call conditional function to ensure TLS variables are used */
    conditional_tls_use(1);
    conditional_tls_use(0);
    
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
        final_sum += *results[i];
        free(results[i]);
    }
    
    /* Access TLS in main thread (different instance) */
    int main_tls_sum = tls_var1 + tls_var2 + tls_var3 + tls_init + tls_static + tls_dllimport;
    
    /* Print results to ensure all code paths are executed */
    printf("Main thread TLS sum: %d\n", main_tls_sum);
    printf("Sum of all thread results: %d\n", final_sum);
    printf("Total (main + threads): %d\n", main_tls_sum + final_sum);
    
    /* Verify expected pattern */
    if ((main_tls_sum > 0) && (final_sum > 0)) {
        printf("TLS emulation test PASSED\n");
        return 0;
    } else {
        printf("TLS emulation test FAILED\n");
        return 1;
    }
}
