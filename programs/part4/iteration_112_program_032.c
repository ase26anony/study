#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Helper function for non-constant initializer */
int some_function(void) {
    return 42;
}

/* 1. Extern TLS with hidden visibility */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS with non-constant initializer */
__thread int tls_var3 = 0;  /* Will be initialized in main() */

/* 4. Static TLS (internal linkage) */
static __thread int tls_var4 __attribute__((used));

/* 5. DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_var5;
#else
    /* Simulate with visibility attribute */
    __thread int tls_var5 __attribute__((visibility("default"), used));
#endif

/* Define the extern TLS variable */
__thread int tls_var1 = 100;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    volatile int* addr1 = &tls_var1;
    volatile int* addr2 = &tls_var2;
    volatile int* addr3 = &tls_var3;
    volatile int* addr4 = &tls_var4;
    volatile int* addr5 = &tls_var5;
    
    /* Assign thread-specific values */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_var5 = 5000 + thread_id;
    
    /* Read back and verify (prevent optimization) */
    if (tls_var1 != (1000 + thread_id) ||
        tls_var2 != (2000 + thread_id) ||
        tls_var3 != (3000 + thread_id) ||
        tls_var4 != (4000 + thread_id) ||
        tls_var5 != (5000 + thread_id)) {
        fprintf(stderr, "Thread %d: TLS verification failed!\n", thread_id);
    }
    
    /* Complex conditional to prevent dead code elimination */
    int checksum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5;
    if (checksum % 2 == 0) {
        printf("Thread %d: checksum = %d (even)\n", thread_id, checksum);
    } else {
        printf("Thread %d: checksum = %d (odd)\n", thread_id, checksum);
    }
    
    return (void*)(long)checksum;
}

int main(void) {
    pthread_t threads[3];
    int thread_ids[3];
    void* thread_results[3];
    int total_checksum = 0;
    
    /* Initialize non-constant TLS variable */
    tls_var3 = some_function();
    
    /* Access all TLS variables in main thread first */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var4 = 4;
    tls_var5 = 5;
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < 3; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
    
    /* Join threads and collect results */
    for (int i = 0; i < 3; i++) {
        if (pthread_join(threads[i], &thread_results[i]) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
        total_checksum += (int)(long)thread_results[i];
    }
    
    /* Access TLS in main thread again (different instance) */
    int main_tls_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5;
    
    printf("Main thread TLS sum: %d\n", main_tls_sum);
    printf("Total checksum from all threads: %d\n", total_checksum);
    
    /* Final verification */
    if (main_tls_sum == (1 + 2 + 42 + 4 + 5)) {
        printf("SUCCESS: TLS emulation appears to be working correctly.\n");
    } else {
        printf("WARNING: Unexpected TLS values in main thread.\n");
    }
    
    return 0;
}
