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

/* 4. TLS with non-constant initializer (forces runtime initialization) */
__thread int tls_init __attribute__((used)) = some_function();

/* 5. Static TLS (internal linkage) */
static __thread int tls_static __attribute__((used));

/* 6. DLL import simulation (for platforms that support it) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport __attribute__((used));
#else
    /* Simulate with visibility attribute */
    extern __thread int tls_dllimport __attribute__((visibility("default"), used));
#endif

/* Define the extern TLS variables */
__thread int tls_var1 = 10;
__thread int tls_var3 = 30;
#ifdef _WIN32
    __thread int tls_dllimport = 60;
#else
    __thread int tls_dllimport __attribute__((visibility("default"))) = 60;
#endif

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Access and modify all TLS variables */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_init = 4000 + thread_id;
    tls_static = 5000 + thread_id;
    tls_dllimport = 6000 + thread_id;
    
    /* Read back values to ensure they're used */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_init + tls_static + tls_dllimport;
    
    /* Conditional use to prevent optimization */
    if (sum > 0) {
        printf("Thread %d: TLS sum = %d\n", thread_id, sum);
    }
    
    /* Return the sum as verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

/* Function that takes address of TLS variables (ODR-use) */
void take_addresses(void) {
    /* Taking addresses forces TLS machinery */
    volatile int* addr1 = &tls_var1;
    volatile int* addr2 = &tls_var2;
    volatile int* addr3 = &tls_var3;
    volatile int* addr4 = &tls_init;
    volatile int* addr5 = &tls_static;
    volatile int* addr6 = &tls_dllimport;
    
    /* Prevent unused variable warnings */
    (void)addr1; (void)addr2; (void)addr3;
    (void)addr4; (void)addr5; (void)addr6;
}

int main(void) {
    pthread_t threads[3];
    int thread_ids[3];
    int* results[3];
    int total_sum = 0;
    
    /* Initialize main thread's TLS values */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_init = 4;
    tls_static = 5;
    tls_dllimport = 6;
    
    /* Force ODR-use of TLS variables */
    take_addresses();
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < 3; i++) {
        thread_ids[i] = i + 1;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Wait for all threads to complete */
    for (int i = 0; i < 3; i++) {
        if (pthread_join(threads[i], (void**)&results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *results[i];
        free(results[i]);
    }
    
    /* Access TLS from main thread (different instance) */
    int main_tls_sum = tls_var1 + tls_var2 + tls_var3 + tls_init + tls_static + tls_dllimport;
    
    printf("Main thread TLS sum = %d\n", main_tls_sum);
    printf("All threads TLS sum = %d\n", total_sum);
    
    /* Final verification */
    if (main_tls_sum == 21) {  /* 1+2+3+4+5+6 = 21 */
        printf("SUCCESS: TLS emulation appears to be working correctly\n");
    } else {
        printf("WARNING: Unexpected TLS values in main thread\n");
    }
    
    return 0;
}
