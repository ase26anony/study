#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/* Helper function for non-constant initializer */
static int some_function(void) {
    static atomic_int counter = 0;
    return atomic_fetch_add(&counter, 1) + 42;
}

/* 1. Extern TLS with hidden visibility */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS (implicitly extern) */
__thread int tls_var3 __attribute__((used));

/* 4. TLS with non-constant initializer (forces complex initialization) */
__thread int tls_init __attribute__((used)) = some_function();

/* 5. For platforms supporting DLL import attributes */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dll __attribute__((used));
#elif defined(__MINGW32__) || defined(__CYGWIN__)
    __attribute__((dllimport)) __thread int tls_dll __attribute__((used));
#else
    /* Simulate with visibility on non-Windows */
    __thread int tls_dll __attribute__((visibility("default"), used));
#endif

/* Define the extern TLS variables */
__thread int tls_var1 = 10;
__thread int tls_var2 = 20;
__thread int tls_var3 = 30;
__thread int tls_dll = 40;

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    int* addr1 = &tls_var1;
    int* addr2 = &tls_var2;
    int* addr3 = &tls_var3;
    int* addr4 = &tls_init;
    int* addr5 = &tls_dll;
    
    /* Prevent optimization from removing address-taking */
    (void)addr1; (void)addr2; (void)addr3; (void)addr4; (void)addr5;
    
    /* Assign thread-specific values */
    tls_var1 = 100 + thread_id;
    tls_var2 = 200 + thread_id;
    tls_var3 = 300 + thread_id;
    tls_init = 400 + thread_id;
    tls_dll = 500 + thread_id;
    
    /* Conditional access to prevent dead code elimination */
    volatile int sum = 0;
    if (thread_id % 2 == 0) {
        sum += tls_var1 + tls_var2;
    } else {
        sum += tls_var3 + tls_init;
    }
    
    /* Always use tls_dll to ensure it's referenced */
    sum += tls_dll;
    
    /* Return the sum as verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

int main(void) {
    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};
    int* results[3];
    
    /* Access TLS in main thread first */
    tls_var1 = 999;
    tls_var2 = 888;
    tls_var3 = 777;
    tls_init = 666;
    tls_dll = 555;
    
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
    
    /* Access TLS again in main thread after threads have run */
    volatile int main_tls_sum = 
        tls_var1 + tls_var2 + tls_var3 + tls_init + tls_dll;
    
    /* Print results to ensure execution */
    printf("Main thread TLS sum: %d\n", main_tls_sum);
    printf("All threads result sum: %d\n", total_sum);
    printf("Final tls_init value in main thread: %d\n", tls_init);
    
    /* Take addresses again to ensure variables are used */
    int* addrs[] = {&tls_var1, &tls_var2, &tls_var3, &tls_init, &tls_dll};
    (void)addrs;
    
    return 0;
}
