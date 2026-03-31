#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Helper function for non-constant initializer */
int some_function(void) {
    return 42;
}

/* 1. Extern TLS with hidden visibility and used attribute */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak));

/* 3. Public common TLS (implicitly extern) */
__thread int tls_var3;

/* 4. Static TLS with DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* Simulate DLL import attribute for non-Windows */
__thread int tls_var4 __attribute__((visibility("default")));
#endif

/* 5. TLS with non-constant initializer to engage emulation logic */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Another TLS with explicit visibility */
__thread int tls_var5 __attribute__((visibility("default"), used));

/* Define the extern TLS variable */
__thread int tls_var1 = 10;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Access all TLS variables in a way that prevents optimization */
    tls_var1 = 100 + thread_id;
    tls_var2 = 200 + thread_id;
    tls_var3 = 300 + thread_id;
    tls_var4 = 400 + thread_id;
    tls_var5 = 500 + thread_id;
    tls_init = 600 + thread_id;
    
    /* Take addresses to force ODR-use */
    volatile int* addrs[] = {
        &tls_var1,
        &tls_var2, 
        &tls_var3,
        &tls_var4,
        &tls_var5,
        &tls_init
    };
    
    /* Conditional access based on thread_id */
    if (thread_id % 2 == 0) {
        tls_var1 *= 2;
        tls_var3 += thread_id;
    } else {
        tls_var2 /= 2;
        tls_var4 -= thread_id;
    }
    
    /* Verify values by reading them back */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    
    /* Store result in heap to prevent optimization */
    int* result = malloc(sizeof(int));
    *result = sum;
    
    printf("Thread %d: TLS sum = %d\n", thread_id, sum);
    printf("  Addresses: var1=%p, var2=%p, var3=%p, var4=%p, var5=%p, init=%p\n",
           &tls_var1, &tls_var2, &tls_var3, &tls_var4, &tls_var5, &tls_init);
    
    return result;
}

int main(void) {
    pthread_t threads[3];
    int thread_ids[3];
    int* results[3];
    int final_sum = 0;
    
    /* Initialize TLS with non-constant value */
    tls_init = some_function();
    
    /* Initialize main thread's TLS values */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    tls_var5 = 5;
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < 3; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Join threads and collect results */
    for (int i = 0; i < 3; i++) {
        if (pthread_join(threads[i], (void**)&results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        final_sum += *results[i];
        free(results[i]);
    }
    
    /* Access TLS from main thread (has its own instance) */
    int main_tls_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    
    printf("\nMain thread TLS sum = %d\n", main_tls_sum);
    printf("All threads TLS sum total = %d\n", final_sum);
    printf("Main thread TLS addresses:\n");
    printf("  var1=%p, var2=%p, var3=%p, var4=%p, var5=%p, init=%p\n",
           &tls_var1, &tls_var2, &tls_var3, &tls_var4, &tls_var5, &tls_init);
    
    /* Force compiler to keep all TLS variables by using their addresses */
    volatile int dummy = 0;
    if (dummy) {  /* Always false, but compiler doesn't know */
        dummy += tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    }
    
    return 0;
}
