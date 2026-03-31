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

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Another TLS with explicit visibility */
__thread int tls_var5 __attribute__((visibility("protected"), used));

/* Define the extern TLS variable */
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
    
    /* Force ODR-use by taking addresses */
    int* addrs[] = {
        &tls_var1,
        &tls_var2,
        &tls_var3,
        &tls_var4,
        &tls_var5,
        &tls_init
    };
    
    /* Conditional access to prevent optimization */
    if (thread_id % 2 == 0) {
        tls_var1 *= 2;
        tls_var3 += thread_id;
    } else {
        tls_var2 /= 2;
        tls_var4 -= thread_id;
    }
    
    /* Verify values were stored correctly */
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
    int total_sum = 0;
    
    /* Initialize non-constant TLS variable */
    tls_init = some_function();
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < 3; i++) {
        thread_ids[i] = i + 1;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Main thread also accesses TLS variables */
    tls_var1 = 1000;
    tls_var2 = 2000;
    tls_var3 = 3000;
    tls_var4 = 4000;
    tls_var5 = 5000;
    
    /* Join threads and collect results */
    for (int i = 0; i < 3; i++) {
        if (pthread_join(threads[i], (void**)&results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *results[i];
        free(results[i]);
    }
    
    /* Add main thread's TLS values */
    total_sum += tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    
    /* Print result to ensure execution */
    printf("Total checksum from all threads: %d\n", total_sum);
    printf("Main thread TLS values: %d, %d, %d, %d, %d, %d\n",
           tls_var1, tls_var2, tls_var3, tls_var4, tls_var5, tls_init);
    
    /* Additional conditional compilation to force different code paths */
#ifdef TEST_DLL
    /* Simulate DLL import usage */
    extern __thread int external_tls __attribute__((dllimport));
    printf("External TLS address: %p\n", (void*)&external_tls);
#endif
    
    return 0;
}
