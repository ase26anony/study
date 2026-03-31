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

/* Function to test TLS variable access */
void test_tls_access(void) {
    /* Read and write to each TLS variable */
    public_tls_var += 1;
    weak_tls_var += 2;
    hidden_tls_var += 3;
    protected_tls_var += 4;
    used_tls_var += 5;
    imported_tls_var += 6;
    
    /* Take addresses to force address-taking code generation */
    int *public_ptr = &public_tls_var;
    int *weak_ptr = &weak_tls_var;
    int *hidden_ptr = &hidden_tls_var;
    int *protected_ptr = &protected_tls_var;
    int *used_ptr = &used_tls_var;
    int *imported_ptr = &imported_tls_var;
    
    /* Use pointers to prevent optimization */
    *public_ptr += 1;
    *weak_ptr += 1;
    *hidden_ptr += 1;
    *protected_ptr += 1;
    *used_ptr += 1;
    *imported_ptr += 1;
    
    /* Access external/common variables */
    external_tls_var = 42;
    common_tls_var = 84;
}

#ifdef USE_PTHREADS
/* Thread function to test TLS in multiple threads */
void *thread_func(void *arg) {
    int thread_id = *(int *)arg;
    
    /* Each thread gets its own TLS values */
    public_tls_var = 1000 + thread_id;
    weak_tls_var = 2000 + thread_id;
    hidden_tls_var = 3000 + thread_id;
    
    /* Return the sum of TLS values from this thread */
    int *result = malloc(sizeof(int));
    *result = public_tls_var + weak_tls_var + hidden_tls_var;
    
    return result;
}
#endif

int main(void) {
    printf("Testing TLS emulation attribute copying...\n");
    
    /* Initial test */
    test_tls_access();
    
    /* Print initial values */
    printf("public_tls_var: %d\n", public_tls_var);
    printf("weak_tls_var: %d\n", weak_tls_var);
    printf("hidden_tls_var: %d\n", hidden_tls_var);
    printf("protected_tls_var: %d\n", protected_tls_var);
    printf("used_tls_var: %d\n", used_tls_var);
    printf("imported_tls_var: %d\n", imported_tls_var);
    
    /* Access external/common variables */
    printf("external_tls_var (from other file): %d\n", external_tls_var);
    printf("common_tls_var (from other file): %d\n", common_tls_var);
    
    /* Complex expression to prevent optimization */
    int checksum = public_tls_var + weak_tls_var + hidden_tls_var +
                   protected_tls_var + used_tls_var + imported_tls_var +
                   external_tls_var + common_tls_var;
    
    printf("Checksum: %d\n", checksum);
    
#ifdef USE_PTHREADS
    /* Test with multiple threads */
    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};
    int *thread_results[3];
    
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }
    
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], (void **)&thread_results[i]);
        printf("Thread %d result: %d\n", i+1, *thread_results[i]);
        free(thread_results[i]);
    }
#endif
    
    /* Final verification */
    test_tls_access();
    printf("Final public_tls_var: %d\n", public_tls_var);
    
    return checksum != 0 ? 0 : 1;
}
