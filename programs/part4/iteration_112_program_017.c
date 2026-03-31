#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    static atomic_int counter = 0;
    return atomic_fetch_add(&counter, 1) + 1;
}

/* TLS variables with various attributes to trigger declaration cloning */

/* 1. Extern TLS with hidden visibility - will set DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - will set DECL_WEAK */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS (implicitly extern) - will set TREE_PUBLIC, DECL_EXTERNAL, DECL_COMMON */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* On non-Windows, use visibility attribute to ensure DECL_DLLIMPORT_P might be considered */
__thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* Define the extern TLS variable */
__thread int tls_var1 = 100;

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Access all TLS variables in a way that prevents optimization */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_init = 5000 + thread_id;
    
    /* Take addresses to ensure ODR-use */
    volatile int* addrs[] = {
        &tls_var1,
        &tls_var2, 
        &tls_var3,
        &tls_var4,
        &tls_init
    };
    
    /* Use the values in conditional ways */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        if (*(addrs[i]) > 0) {
            sum += *(addrs[i]);
        }
    }
    
    /* Return the sum as verification */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

int main(void) {
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* thread_results[NUM_THREADS];
    
    /* Initialize TLS with non-constant value */
    tls_init = get_initial_value();
    
    /* Initialize main thread's TLS values */
    tls_var1 = 1;
    tls_var2 = 2;
    tls_var3 = 3;
    tls_var4 = 4;
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Wait for threads and collect results */
    int total_sum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *thread_results[i];
        free(thread_results[i]);
    }
    
    /* Access TLS in main thread (has its own instance) */
    int main_tls_sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init;
    
    /* Print verification results */
    printf("Main thread TLS sum: %d\n", main_tls_sum);
    printf("All threads computed sum: %d\n", total_sum);
    
    /* Additional complex access pattern to ensure TLS machinery is used */
    volatile int* volatile_ptr;
    
    /* Conditional compilation to access different TLS variables */
    #ifdef TEST_DLLIMPORT
    volatile_ptr = &tls_var4;
    #else
    volatile_ptr = &tls_var1;
    #endif
    
    /* Force compiler to consider all TLS variables */
    int final_check = 0;
    final_check += (tls_var1 != 0) ? 1 : 0;
    final_check += (tls_var2 != 0) ? 1 : 0;
    final_check += (tls_var3 != 0) ? 1 : 0;
    final_check += (tls_var4 != 0) ? 1 : 0;
    final_check += (tls_init != 0) ? 1 : 0;
    
    printf("TLS variables accessed: %d/5\n", final_check);
    
    return 0;
}
