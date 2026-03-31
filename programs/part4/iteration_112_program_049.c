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

/* 3. Public common TLS (implicitly extern) */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* On non-Windows, use visibility to simulate similar attribute handling */
__thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Another TLS with explicit visibility */
__thread int tls_var5 __attribute__((visibility("hidden"), used));

/* Define the extern TLS variable (to satisfy linker) */
__thread int tls_var1 = 10;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Access and modify all TLS variables */
    tls_var1 = 100 + thread_id;
    tls_var2 = 200 + thread_id;
    tls_var3 = 300 + thread_id;
    tls_var4 = 400 + thread_id;
    tls_var5 = 500 + thread_id;
    tls_init = 600 + thread_id;
    
    /* Read back and verify (ODR-use through taking address) */
    int* addrs[] = {&tls_var1, &tls_var2, &tls_var3, &tls_var4, &tls_var5, &tls_init};
    int sum = 0;
    
    for (int i = 0; i < 6; i++) {
        sum += *addrs[i];  /* Force address-taking */
    }
    
    /* Conditional access to prevent optimization */
    if (sum > 0) {
        printf("Thread %d: TLS sum = %d\n", thread_id, sum);
    }
    
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
    
    /* Access TLS variables in main thread first */
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
            return 1;
        }
    }
    
    /* Wait for threads to complete */
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
    }
    
    /* Verify TLS isolation: main thread's values should be unchanged */
    int main_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    printf("Main thread TLS sum = %d\n", main_sum);
    
    /* Calculate and verify thread results */
    int total_sum = main_sum;
    for (int i = 0; i < NUM_THREADS; i++) {
        total_sum += *thread_results[i];
        free(thread_results[i]);
    }
    
    printf("Total sum from all threads = %d\n", total_sum);
    
    /* Expected calculation for verification:
       Main thread: 1+2+3+4+5+42 = 57
       Thread 0: (100+0)+(200+0)+(300+0)+(400+0)+(500+0)+(600+0) = 2100
       Thread 1: (100+1)+(200+1)+(300+1)+(400+1)+(500+1)+(600+1) = 2106
       Thread 2: (100+2)+(200+2)+(300+2)+(400+2)+(500+2)+(600+2) = 2112
       Total: 57 + 2100 + 2106 + 2112 = 6375
    */
    
    if (total_sum == 6375) {
        printf("SUCCESS: TLS emulation working correctly\n");
        return 0;
    } else {
        printf("ERROR: Unexpected result %d\n", total_sum);
        return 1;
    }
}
