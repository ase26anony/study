/* test_tls_coverage.c - Program to trigger TLS declaration cloning logic */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    return 42;
}

/* 1. Extern TLS with hidden visibility */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS (implicitly extern) */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation */
#ifdef _WIN32
    /* For Windows/MinGW targets */
    __declspec(dllimport) __thread int tls_var4;
#else
    /* For non-Windows, use visibility to simulate some attribute */
    __thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Another TLS with explicit external linkage */
extern __thread int tls_extern __attribute__((visibility("protected")));

/* Define the external TLS variable */
__thread int tls_extern = 100;

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Take addresses of TLS variables to force ODR-use */
    int* addr1 = &tls_var1;
    int* addr2 = &tls_var2;
    int* addr3 = &tls_var3;
    int* addr4 = &tls_var4;
    int* addr5 = &tls_init;
    int* addr6 = &tls_extern;
    
    /* Prevent optimization from removing the address-taking */
    volatile int dummy = (uintptr_t)addr1 + (uintptr_t)addr2 + 
                        (uintptr_t)addr3 + (uintptr_t)addr4 +
                        (uintptr_t)addr5 + (uintptr_t)addr6;
    (void)dummy;
    
    /* Assign thread-specific values to TLS variables */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_init = 5000 + thread_id;
    tls_extern = 6000 + thread_id;
    
    /* Read back and verify values */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_extern;
    
    /* Complex conditional to prevent optimization */
    if (sum > 0) {
        int* result = malloc(sizeof(int));
        *result = sum;
        return result;
    }
    
    return NULL;
}

/* Function that returns address of TLS variable - forces TLS machinery */
static int* get_tls_address(int selector) {
    switch (selector) {
        case 1: return &tls_var1;
        case 2: return &tls_var2;
        case 3: return &tls_var3;
        case 4: return &tls_var4;
        case 5: return &tls_init;
        case 6: return &tls_extern;
        default: return NULL;
    }
}

int main(void) {
    pthread_t threads[3];
    int thread_ids[3];
    int* thread_results[3];
    int final_sum = 0;
    
    /* Initialize TLS with non-constant value */
    tls_init = get_initial_value();
    
    /* Initialize other TLS variables in main thread */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    tls_extern = 5;
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < 3; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Main thread also accesses TLS variables */
    int main_tls_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_extern;
    
    /* Force address-taking in main thread too */
    volatile int* main_addr = get_tls_address(1);
    (void)main_addr;
    
    /* Join threads and collect results */
    for (int i = 0; i < 3; i++) {
        if (pthread_join(threads[i], (void**)&thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
    }
    
    /* Calculate final checksum */
    final_sum += main_tls_sum;
    for (int i = 0; i < 3; i++) {
        if (thread_results[i]) {
            final_sum += *thread_results[i];
            free(thread_results[i]);
        }
    }
    
    /* Access TLS variables one more time with conditional */
    if (final_sum > 0) {
        tls_var1 = final_sum % 1000;
        tls_var2 = (final_sum / 1000) % 1000;
        tls_var3 = (final_sum / 1000000) % 1000;
    }
    
    /* Print result to ensure execution */
    printf("TLS test completed. Final checksum: %d\n", final_sum);
    printf("Main thread TLS values: var1=%d, var2=%d, var3=%d, var4=%d, init=%d, extern=%d\n",
           tls_var1, tls_var2, tls_var3, tls_var4, tls_init, tls_extern);
    
    return 0;
}
