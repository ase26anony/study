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

/* 3. Public common TLS with external linkage */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* Simulate similar attribute on non-Windows */
__thread int tls_var4 __attribute__((visibility("default")));
#endif

/* 5. TLS with non-constant initializer to engage emulation logic */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Another TLS with explicit visibility */
__thread int tls_var5 __attribute__((visibility("protected"), used));

/* Define the extern TLS variable */
__thread int tls_var1 = 100;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    int* addr1 = &tls_var1;
    int* addr2 = &tls_var2;
    int* addr3 = &tls_var3;
    int* addr4 = &tls_var4;
    int* addr5 = &tls_var5;
    int* addr_init = &tls_init;
    
    /* Prevent optimization from removing TLS accesses */
    volatile int* volatile_addrs[] = {
        (volatile int*)addr1,
        (volatile int*)addr2,
        (volatile int*)addr3,
        (volatile int*)addr4,
        (volatile int*)addr5,
        (volatile int*)addr_init
    };
    
    /* Write thread-specific values to TLS variables */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_var5 = 5000 + thread_id;
    tls_init = 6000 + thread_id;
    
    /* Conditional access to prevent dead code elimination */
    if (thread_id % 2 == 0) {
        tls_var1 += 1;
        tls_var3 += 1;
        tls_var5 += 1;
    } else {
        tls_var2 += 2;
        tls_var4 += 2;
        tls_init += 2;
    }
    
    /* Read back and verify values */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    
    /* Store result in dynamically allocated memory to return */
    int* result = malloc(sizeof(int));
    *result = sum;
    
    /* Additional conditional use of addresses */
    if (volatile_addrs[0][0] != tls_var1) {
        printf("Error: TLS access mismatch in thread %d\n", thread_id);
    }
    
    return result;
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
    tls_var5 = 5;
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
    
    /* Access TLS in main thread (has its own instance) */
    int main_tls_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    printf("Main thread TLS sum: %d\n", main_tls_sum);
    
    /* Join threads and collect results */
    int total_sum = main_tls_sum;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&thread_results[i]) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
        total_sum += *thread_results[i];
        free(thread_results[i]);
    }
    
    /* Final verification */
    printf("Total sum from all threads: %d\n", total_sum);
    
    /* Additional conditional access to force TLS machinery */
    if (total_sum > 0) {
        /* Take addresses again to ensure TLS symbols are used */
        int* addrs[] = {&tls_var1, &tls_var2, &tls_var3, &tls_var4, &tls_var5, &tls_init};
        
        /* Use addresses in a way that can't be optimized away */
        int checksum = 0;
        for (int i = 0; i < 6; i++) {
            checksum ^= (int)(uintptr_t)addrs[i];
        }
        printf("TLS addresses checksum: 0x%x\n", checksum);
    }
    
    return 0;
}
