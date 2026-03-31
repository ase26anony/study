#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    /* For non-Windows, use visibility attribute to simulate similar handling */
    __thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Another TLS with explicit visibility */
__thread int tls_var5 __attribute__((visibility("protected"), used));

/* Define the extern TLS variables */
__thread int tls_var1 = 10;
__thread int tls_var3 = 30;
__thread int tls_var4 = 40;
__thread int tls_var5 = 50;

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
    (void)addr1; (void)addr2; (void)addr3; 
    (void)addr4; (void)addr5; (void)addr_init;
    
    /* Assign thread-specific values */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_var5 = 5000 + thread_id;
    tls_init = 6000 + thread_id;
    
    /* Conditional access to prevent dead code elimination */
    if (thread_id % 2 == 0) {
        tls_var1 *= 2;
        tls_var3 += thread_id;
    } else {
        tls_var2 /= 2;
        tls_var4 -= thread_id;
    }
    
    /* Complex conditional to ensure all variables are used */
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0: tls_var1++; break;
            case 1: tls_var2++; break;
            case 2: tls_var3++; break;
        }
    }
    
    /* Return a checksum of TLS values */
    int* result = malloc(sizeof(int));
    *result = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    
    printf("Thread %d: tls_var1=%d, tls_var2=%d, tls_var3=%d, tls_var4=%d, tls_var5=%d, tls_init=%d\n",
           thread_id, tls_var1, tls_var2, tls_var3, tls_var4, tls_var5, tls_init);
    
    return result;
}

int main(void) {
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* thread_results[NUM_THREADS];
    
    /* Initialize tls_init with non-constant value */
    tls_init = get_initial_value();
    
    printf("Main thread: Initial tls_init = %d\n", tls_init);
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Access TLS in main thread (different instance) */
    tls_var1 = 999;
    tls_var2 = 888;
    tls_var3 = 777;
    tls_var4 = 666;
    tls_var5 = 555;
    tls_init = 444;
    
    /* Force address-taking in main thread too */
    volatile int* addrs[] = {&tls_var1, &tls_var2, &tls_var3, &tls_var4, &tls_var5, &tls_init};
    (void)addrs;
    
    printf("Main thread: tls_var1=%d, tls_var2=%d, tls_var3=%d, tls_var4=%d, tls_var5=%d, tls_init=%d\n",
           tls_var1, tls_var2, tls_var3, tls_var4, tls_var5, tls_init);
    
    /* Join threads and collect results */
    int total_checksum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_checksum += *thread_results[i];
        free(thread_results[i]);
    }
    
    /* Add main thread's values to checksum */
    total_checksum += tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    
    printf("Total checksum from all threads: %d\n", total_checksum);
    printf("Program completed successfully.\n");
    
    return 0;
}
