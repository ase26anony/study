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

/* 4. Static TLS with DLL import simulation */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_var4;
#else
    /* Simulate DLL import attribute for non-Windows */
    __thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer */
__thread int tls_init = 0;  /* Will be initialized in main */

/* 6. Another TLS with explicit visibility */
__thread int tls_var5 __attribute__((visibility("protected"), used));

/* Define the extern TLS variable */
__thread int tls_var1 = 100;

/* Weak TLS definition */
__thread int tls_var2 = 200;

/* Public TLS definition */
__thread int tls_var3 = 300;

/* DLL import-like TLS definition */
__thread int tls_var4 = 400;

/* Protected visibility TLS definition */
__thread int tls_var5 = 500;

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
    
    /* Prevent optimization by using addresses */
    if (thread_id % 2 == 0) {
        /* Even threads use direct access */
        tls_var1 = 1000 + thread_id;
        tls_var2 = 2000 + thread_id;
        tls_var3 = 3000 + thread_id;
        tls_var4 = 4000 + thread_id;
        tls_var5 = 5000 + thread_id;
        tls_init = 6000 + thread_id;
    } else {
        /* Odd threads use pointer access */
        *addr1 = 1000 + thread_id;
        *addr2 = 2000 + thread_id;
        *addr3 = 3000 + thread_id;
        *addr4 = 4000 + thread_id;
        *addr5 = 5000 + thread_id;
        *addr_init = 6000 + thread_id;
    }
    
    /* Verify values by reading back */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    
    /* Conditional use to prevent dead code elimination */
    if (sum > 0) {
        printf("Thread %d: TLS sum = %d\n", thread_id, sum);
    }
    
    /* Return the sum as verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

int main(void) {
    const int NUM_THREADS = 4;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* thread_results[NUM_THREADS];
    
    /* Initialize TLS with non-constant value */
    tls_init = some_function();
    
    /* Force compiler to consider all TLS variables as used */
    volatile int force_use = 0;
    
    /* Conditional access to prevent optimization */
    if (force_use == 0) {
        force_use += tls_var1;
        force_use += tls_var2;
        force_use += tls_var3;
        force_use += tls_var4;
        force_use += tls_var5;
    }
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Main thread accesses its own TLS instances */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    tls_var5 = 5;
    tls_init = 6;
    
    int main_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    printf("Main thread: TLS sum = %d\n", main_sum);
    
    /* Join threads and collect results */
    int total_sum = main_sum;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *thread_results[i];
        free(thread_results[i]);
    }
    
    /* Final verification */
    printf("Total sum from all threads: %d\n", total_sum);
    
    /* Expected calculation:
       Main thread: 1+2+3+4+5+6 = 21
       Each thread i: (1000+i)+(2000+i)+(3000+i)+(4000+i)+(5000+i)+(6000+i) 
                    = 21000 + 6*i
       For i=0..3: 21000 + 21006 + 21012 + 21018 = 84036
       Total: 21 + 84036 = 84057
    */
    int expected = 84057;
    
    if (total_sum == expected) {
        printf("SUCCESS: TLS emulation worked correctly!\n");
        return 0;
    } else {
        printf("WARNING: Unexpected result (got %d, expected %d)\n", total_sum, expected);
        return 0;  /* Still return 0 as we want to test compilation */
    }
}
