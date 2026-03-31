#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Helper function for non-constant initializer */
static int some_function(void) {
    return 42;
}

/* 1. Extern TLS with hidden visibility */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS with external linkage */
__thread int tls_var3 __attribute__((used));

/* 4. TLS with DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_var4;
#else
    /* Simulate with visibility attribute */
    __thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Static TLS - different linkage */
static __thread int tls_static __attribute__((used));

/* Define the extern TLS variable */
__thread int tls_var1 = 100;

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    int* addr1 = &tls_var1;
    int* addr2 = &tls_var2;
    int* addr3 = &tls_var3;
    int* addr4 = &tls_var4;
    int* addr5 = &tls_init;
    int* addr6 = &tls_static;
    
    /* Prevent optimization from removing TLS accesses */
    volatile int* volatile_addrs[] = {addr1, addr2, addr3, addr4, addr5, addr6};
    
    /* Assign thread-specific values */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_init = 5000 + thread_id;
    tls_static = 6000 + thread_id;
    
    /* Read back and verify (simulate real work) */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_static;
    
    /* Conditional access to prevent dead code elimination */
    if (sum > 0) {
        int* result = malloc(sizeof(int));
        *result = sum;
        return result;
    }
    
    return NULL;
}

/* Function that returns address of TLS variable - forces emulation */
static int* get_tls_address(int selector) {
    switch (selector) {
        case 1: return &tls_var1;
        case 2: return &tls_var2;
        case 3: return &tls_var3;
        case 4: return &tls_var4;
        case 5: return &tls_init;
        case 6: return &tls_static;
        default: return NULL;
    }
}

int main(void) {
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* thread_results[NUM_THREADS];
    
    /* Initialize non-constant TLS variable */
    tls_init = some_function();
    
    /* Initialize main thread's TLS values */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    tls_static = 6;
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Collect results */
    int total_sum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        if (thread_results[i]) {
            total_sum += *thread_results[i];
            free(thread_results[i]);
        }
    }
    
    /* Access TLS from main thread - ensures main thread's TLS is used */
    int main_tls_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_static;
    
    /* Force address-taking in main thread as well */
    volatile int* main_addrs[6];
    for (int i = 1; i <= 6; i++) {
        main_addrs[i-1] = get_tls_address(i);
    }
    
    /* Print results to prevent optimization */
    printf("Main thread TLS sum: %d\n", main_tls_sum);
    printf("Sum of all thread results: %d\n", total_sum);
    printf("Expected main thread values: tls_var1=%d, tls_var2=%d, tls_var3=%d\n", 
           tls_var1, tls_var2, tls_var3);
    
    /* Verification */
    if (main_tls_sum == (1 + 2 + 3 + 4 + tls_init + 6)) {
        printf("TLS emulation test PASSED\n");
        return 0;
    } else {
        printf("TLS emulation test FAILED\n");
        return 1;
    }
}
