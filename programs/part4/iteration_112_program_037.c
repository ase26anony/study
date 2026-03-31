#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Helper function for non-constant initializer */
int some_function(void) {
    return 42;
}

/* TLS variables with various attributes to trigger declaration cloning */

/* 1. Extern TLS with hidden visibility - will set DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - will set DECL_WEAK */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS - will set TREE_PUBLIC, DECL_EXTERNAL, DECL_COMMON */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* On non-Windows, use a visibility attribute as proxy */
__thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer - complicates initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

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
    int* addr5 = &tls_init;
    
    /* Write thread-specific values to TLS variables */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_init = 5000 + thread_id;
    
    /* Read back and verify (prevent optimization) */
    volatile int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init;
    
    /* Conditional access based on thread ID */
    if (thread_id % 2 == 0) {
        tls_var1 *= 2;
    } else {
        tls_var2 *= 3;
    }
    
    /* Store result in dynamically allocated memory to prevent optimization */
    int* result = malloc(sizeof(int));
    *result = sum + thread_id;
    
    printf("Thread %d: tls_var1=%d, tls_var2=%d, tls_var3=%d, tls_var4=%d, tls_init=%d, sum=%d\n",
           thread_id, tls_var1, tls_var2, tls_var3, tls_var4, tls_init, sum);
    
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
    
    printf("Main thread before creating threads:\n");
    printf("  tls_var1=%d, tls_var2=%d, tls_var3=%d, tls_var4=%d, tls_init=%d\n",
           tls_var1, tls_var2, tls_var3, tls_var4, tls_init);
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Access TLS in main thread while threads are running */
    volatile int main_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init;
    printf("Main thread during thread execution: sum=%d\n", main_sum);
    
    /* Join all threads */
    int total_sum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *thread_results[i];
        free(thread_results[i]);
    }
    
    /* Final access to TLS in main thread */
    printf("Main thread after joining threads:\n");
    printf("  tls_var1=%d, tls_var2=%d, tls_var3=%d, tls_var4=%d, tls_init=%d\n",
           tls_var1, tls_var2, tls_var3, tls_var4, tls_init);
    
    /* Final checksum to prove TLS was used */
    int final_checksum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + total_sum;
    printf("Final checksum: %d\n", final_checksum);
    
    /* Force compiler to keep all TLS variables by taking their addresses at the end */
    asm volatile("" : : "r"(&tls_var1), "r"(&tls_var2), "r"(&tls_var3), "r"(&tls_var4), "r"(&tls_init));
    
    return 0;
}
