#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Helper function for non-constant initializer */
static int some_function(void) {
    return 42;
}

/* 1. Extern TLS with hidden visibility */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS (implicitly extern) */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation */
/* For MinGW/Windows targets, use: __declspec(dllimport) __thread int tls_var4 */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4 __attribute__((used));
#else
/* Simulate DLL import attribute for non-Windows */
__thread int tls_var4 __attribute__((used));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = some_function();

/* 6. Additional TLS with explicit visibility */
__thread int tls_var5 __attribute__((visibility("default"), used));

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
    volatile int* addr5 = &tls_var5;
    volatile int* addr_init = &tls_init;
    
    /* Prevent optimization by using addresses */
    (void)addr1; (void)addr2; (void)addr3; (void)addr4; (void)addr5; (void)addr_init;
    
    /* Write unique values to each TLS variable */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_var5 = 5000 + thread_id;
    
    /* Conditional access to prevent dead code elimination */
    if (thread_id % 2 == 0) {
        tls_init = 6000 + thread_id;
    } else {
        tls_init = 7000 + thread_id;
    }
    
    /* Read back and verify (simulate actual use) */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    
    /* Return the sum for verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

int main(void) {
    pthread_t threads[3];
    int thread_ids[3];
    int* results[3];
    int final_sum = 0;
    
    /* Initialize tls_var2 and tls_var3 in main thread */
    tls_var2 = 999;
    tls_var3 = 888;
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < 3; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Main thread accesses its own TLS instances */
    tls_var1 = 111;
    tls_var2 = 222;
    tls_var3 = 333;
    tls_var4 = 444;
    tls_var5 = 555;
    tls_init = 666;
    
    /* Join threads and collect results */
    for (int i = 0; i < 3; i++) {
        if (pthread_join(threads[i], (void**)&results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        final_sum += *results[i];
        free(results[i]);
    }
    
    /* Calculate main thread's TLS sum */
    int main_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    
    /* Print results to ensure execution */
    printf("Main thread TLS sum: %d\n", main_sum);
    printf("Sum of all thread results: %d\n", final_sum);
    printf("Total (main + threads): %d\n", main_sum + final_sum);
    
    /* Additional conditional access to prevent optimization */
    volatile int check = 0;
    if (main_sum > 1000) {
        check = tls_var1;
    } else {
        check = tls_var2;
    }
    
    printf("Final check value: %d\n", check);
    
    return 0;
}
