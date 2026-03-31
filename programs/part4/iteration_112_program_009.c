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

/* 4. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 5. Static TLS with DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dll;
#else
/* On non-Windows, use a visibility attribute as alternative */
__thread int tls_dll __attribute__((visibility("default"), used));
#endif

/* Define the extern TLS variable */
__thread int tls_var1 = 100;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Access and modify all TLS variables */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_init = 4000 + thread_id;
    
    /* Take addresses to ensure ODR-use */
    int* ptr1 = &tls_var1;
    int* ptr2 = &tls_var2;
    int* ptr3 = &tls_var3;
    int* ptr4 = &tls_init;
    
    /* Conditional access to prevent optimization */
    if (thread_id % 2 == 0) {
        tls_var1 += 1;
        tls_var2 += 2;
    } else {
        tls_var3 += 3;
        tls_init += 4;
    }
    
    /* Read back and verify values */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_init;
    
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
    tls_var1 = 999;
    tls_var2 = 888;
    tls_var3 = 777;
    
    /* Take addresses in main thread too */
    volatile int* main_ptr1 = &tls_var1;
    volatile int* main_ptr2 = &tls_var2;
    
    /* Join threads and collect results */
    for (int i = 0; i < 3; i++) {
        if (pthread_join(threads[i], (void**)&results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *results[i];
        free(results[i]);
    }
    
    /* Access TLS in main thread after joins */
    int main_tls_sum = tls_var1 + tls_var2 + tls_var3 + tls_init;
    
    /* Print results to ensure execution */
    printf("Main thread TLS sum: %d\n", main_tls_sum);
    printf("Sum of all thread results: %d\n", total_sum);
    printf("All TLS accesses completed successfully.\n");
    
    /* Additional conditional compilation to ensure all attributes are considered */
    #ifdef __GNUC__
    /* Force reference to tls_dll with conditional compilation */
    if (total_sum > 0) {
        tls_dll = main_tls_sum;
        printf("tls_dll set to: %d\n", tls_dll);
    }
    #endif
    
    return 0;
}
