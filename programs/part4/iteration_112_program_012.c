#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    static atomic_int counter = 0;
    return atomic_fetch_add(&counter, 1) + 42;
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
__thread int tls_var4 __attribute__((dllimport));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. TLS with default visibility explicitly specified */
__thread int tls_visible __attribute__((visibility("default"), used));

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
    int* addr6 = &tls_visible;
    
    /* Prevent optimization by using addresses in conditionals */
    if (addr1 && addr2 && addr3 && addr4 && addr5 && addr6) {
        /* Each thread writes unique values to TLS */
        tls_var1 = 1000 + thread_id;
        tls_var2 = 2000 + thread_id;
        tls_var3 = 3000 + thread_id;
        tls_var4 = 4000 + thread_id;
        tls_init = 5000 + thread_id;
        tls_visible = 6000 + thread_id;
        
        /* Read back and verify (simulate real work) */
        int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_visible;
        
        /* Store result in heap for main thread to collect */
        int* result = malloc(sizeof(int));
        *result = sum;
        
        printf("Thread %d: tls_var1=%d, tls_var2=%d, tls_var3=%d, "
               "tls_var4=%d, tls_init=%d, tls_visible=%d, sum=%d\n",
               thread_id, tls_var1, tls_var2, tls_var3, 
               tls_var4, tls_init, tls_visible, sum);
        
        return result;
    }
    
    return NULL;
}

int main(void) {
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* thread_results[NUM_THREADS];
    
    /* Initialize non-constant TLS variable */
    tls_init = get_initial_value();
    printf("Main: Initialized tls_init to %d\n", tls_init);
    
    /* Initialize main thread's TLS values */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    tls_visible = 6;
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Collect results */
    int total_sum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        void* retval;
        pthread_join(threads[i], &retval);
        
        if (retval) {
            thread_results[i] = (int*)retval;
            total_sum += *thread_results[i];
        }
    }
    
    /* Access TLS from main thread (has its own instance) */
    int main_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_visible;
    
    printf("\nMain thread TLS values: "
           "tls_var1=%d, tls_var2=%d, tls_var3=%d, "
           "tls_var4=%d, tls_init=%d, tls_visible=%d\n",
           tls_var1, tls_var2, tls_var3, tls_var4, tls_init, tls_visible);
    
    printf("Main thread sum: %d\n", main_sum);
    printf("All threads sum: %d\n", total_sum);
    printf("Program completed successfully.\n");
    
    /* Cleanup */
    for (int i = 0; i < NUM_THREADS; i++) {
        if (thread_results[i]) {
            free(thread_results[i]);
        }
    }
    
    return 0;
}
