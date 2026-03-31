/* Main test file for TLS emulation attribute copying coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For thread testing */
#ifdef USE_PTHREADS
#include <pthread.h>
#include <unistd.h>
#endif

/* Forward declarations for TLS variables defined in other files */
extern __thread int external_tls_var;
extern __thread int common_tls_var;

/* Public TLS variable with external linkage */
__thread int public_tls_var = 100;

/* Weak TLS variable */
__thread int weak_tls_var __attribute__((weak)) = 200;

/* TLS variables with different visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 300;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 400;

/* Used attribute to ensure TREE_USED is set */
__thread int used_tls_var __attribute__((used)) = 500;

/* DLL import attribute (Windows-specific) */
#ifdef _WIN32
__thread int imported_tls_var __attribute__((dllimport));
#else
/* On non-Windows, just a regular TLS variable */
__thread int imported_tls_var = 600;
#endif

/* Function to use all TLS variables, preventing optimization */
static int use_tls_variables(void) {
    int sum = 0;
    
    /* Read from each TLS variable */
    sum += public_tls_var;
    sum += weak_tls_var;
    sum += hidden_tls_var;
    sum += protected_tls_var;
    sum += used_tls_var;
    sum += imported_tls_var;
    sum += external_tls_var;
    sum += common_tls_var;
    
    /* Write to each TLS variable */
    public_tls_var += 1;
    weak_tls_var += 2;
    hidden_tls_var += 3;
    protected_tls_var += 4;
    used_tls_var += 5;
    imported_tls_var += 6;
    
    /* Take addresses (affects code generation) */
    int *ptr1 = &public_tls_var;
    int *ptr2 = &weak_tls_var;
    int *ptr3 = &hidden_tls_var;
    int *ptr4 = &protected_tls_var;
    int *ptr5 = &used_tls_var;
    int *ptr6 = &imported_tls_var;
    int *ptr7 = &external_tls_var;
    int *ptr8 = &common_tls_var;
    
    /* Use pointers to prevent dead store elimination */
    sum += *ptr1 + *ptr2 + *ptr3 + *ptr4 + *ptr5 + *ptr6 + *ptr7 + *ptr8;
    
    return sum;
}

#ifdef USE_PTHREADS
/* Thread function accessing TLS variables */
static void *thread_func(void *arg) {
    int thread_id = *(int *)arg;
    
    /* Each thread gets its own TLS values */
    public_tls_var = 1000 + thread_id;
    weak_tls_var = 2000 + thread_id;
    
    int result = use_tls_variables();
    
    /* Return result through pthread_exit */
    int *retval = malloc(sizeof(int));
    *retval = result + thread_id;
    pthread_exit(retval);
}
#endif

int main(void) {
    int checksum = 0;
    
    printf("Testing TLS emulation with various attributes...\n");
    
    /* Initial use of TLS variables */
    checksum = use_tls_variables();
    printf("Initial checksum: %d\n", checksum);
    
#ifdef USE_PTHREADS
    /* Test with multiple threads if pthreads available */
    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};
    void *thread_results[3];
    
    printf("Creating threads to test TLS...\n");
    
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }
    
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], &thread_results[i]);
        checksum += *(int *)thread_results[i];
        free(thread_results[i]);
    }
    
    printf("Threaded checksum: %d\n", checksum);
#endif
    
    /* Final use in main thread */
    checksum += use_tls_variables();
    
    /* Verify external/common TLS variables */
    printf("External TLS var: %d\n", external_tls_var);
    printf("Common TLS var: %d\n", common_tls_var);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Return non-zero if any TLS variable has unexpected value */
    if (checksum > 1000000) {
        return 1;
    }
    
    return 0;
}
