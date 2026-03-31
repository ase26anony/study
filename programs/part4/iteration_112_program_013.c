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

/* 1. Extern TLS with hidden visibility - will set DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - will set DECL_WEAK */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS - will set TREE_PUBLIC, DECL_EXTERNAL, DECL_COMMON */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation (for MinGW/Windows targets) */
/* Use conditional compilation to avoid errors on non-Windows platforms */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* Simulate with visibility attribute */
__thread int tls_var4 __attribute__((visibility("default")));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0; /* Will be initialized in main() */

/* Define the extern TLS variable */
__thread int tls_var1 = 100;

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Access and modify all TLS variables */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_init = 5000 + thread_id;
    
    /* Take addresses to force ODR-use */
    volatile int* addr1 = &tls_var1;
    volatile int* addr2 = &tls_var2;
    volatile int* addr3 = &tls_var3;
    volatile int* addr4 = &tls_var4;
    volatile int* addr5 = &tls_init;
    
    /* Conditional access to prevent optimization */
    if (thread_id % 2 == 0) {
        *addr1 += 1;
        *addr3 += 1;
    } else {
        *addr2 += 1;
        *addr4 += 1;
    }
    
    /* Verify values */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init;
    
    /* Return the sum for verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

int main(void) {
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* thread_results[NUM_THREADS];
    
    /* Initialize non-constant TLS variable */
    tls_init = some_function();
    
    /* Initialize TLS variables in main thread */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Access TLS in main thread (different instance) */
    tls_var1 = 10;
    tls_var2 = 20;
    tls_var3 = 30;
    tls_var4 = 40;
    tls_init = 50;
    
    /* Take addresses in main thread too */
    volatile int* addrs[] = {&tls_var1, &tls_var2, &tls_var3, &tls_var4, &tls_init};
    for (int i = 0; i < 5; i++) {
        *(addrs[i]) += i;
    }
    
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
    
    /* Calculate main thread's sum */
    int main_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init;
    
    /* Print results to ensure all TLS accesses happened */
    printf("Main thread TLS sum: %d\n", main_sum);
    printf("All threads TLS sum total: %d\n", total_sum);
    printf("Combined checksum: %d\n", main_sum + total_sum);
    
    /* Additional conditional compilation to use all variables */
    #ifdef TEST_COMPILE
    /* Force references to all TLS variables in dead code */
    if (0) {
        printf("%p %p %p %p %p\n", 
               (void*)&tls_var1, 
               (void*)&tls_var2, 
               (void*)&tls_var3, 
               (void*)&tls_var4, 
               (void*)&tls_init);
    }
    #endif
    
    return 0;
}
