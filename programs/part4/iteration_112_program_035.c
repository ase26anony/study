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

/* 1. Extern TLS with hidden visibility - should set DECL_VISIBILITY */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - should set DECL_WEAK */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS - should set DECL_COMMON and TREE_PUBLIC */
__thread int tls_var3 __attribute__((used));

/* 4. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init __attribute__((used)) = (some_function());

/* 5. Static TLS with DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dll_var;
#else
/* Simulate with visibility attribute */
__thread int tls_dll_var __attribute__((visibility("default"), used));
#endif

/* Define the extern TLS variable */
__thread int tls_var1 = 1000;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Access and modify all TLS variables */
    tls_var1 = 100 + thread_id;
    tls_var2 = 200 + thread_id;
    tls_var3 = 300 + thread_id;
    tls_init = 400 + thread_id;
    tls_dll_var = 500 + thread_id;
    
    /* Read back and verify (ODR-use) */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_init + tls_dll_var;
    
    /* Conditional access to prevent optimization */
    if (sum > 0) {
        printf("Thread %d: TLS sum = %d\n", thread_id, sum);
    }
    
    /* Return the sum as verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

/* Function that takes address of TLS variables (forces emulation) */
void take_tls_addresses(void) {
    /* Taking addresses forces TLS emulation setup */
    static volatile int* addrs[5];
    
    addrs[0] = &tls_var1;
    addrs[1] = &tls_var2;
    addrs[2] = &tls_var3;
    addrs[3] = &tls_init;
    addrs[4] = &tls_dll_var;
    
    /* Prevent unused variable warning */
    (void)addrs;
}

int main(void) {
    pthread_t threads[3];
    int thread_ids[3];
    void* thread_results[3];
    int total_sum = 0;
    
    /* Initialize main thread's TLS instances */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_init = 4;
    tls_dll_var = 5;
    
    /* Force address taking (triggers TLS emulation) */
    take_tls_addresses();
    
    /* Create multiple threads to force TLS emulation with pthreads */
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
    }
    
    /* Calculate total from thread results */
    for (int i = 0; i < 3; i++) {
        total_sum += *(int*)thread_results[i];
        free(thread_results[i]);
    }
    
    /* Access main thread's TLS variables (has its own instances) */
    int main_tls_sum = tls_var1 + tls_var2 + tls_var3 + tls_init + tls_dll_var;
    
    printf("Main thread TLS sum = %d\n", main_tls_sum);
    printf("All threads TLS total = %d\n", total_sum);
    printf("Expected main thread sum = 15 (1+2+3+4+5)\n");
    
    /* Verify that main thread TLS is independent */
    if (main_tls_sum == 15) {
        printf("SUCCESS: TLS emulation appears to be working correctly\n");
        return 0;
    } else {
        printf("ERROR: Main thread TLS sum incorrect\n");
        return 1;
    }
}
