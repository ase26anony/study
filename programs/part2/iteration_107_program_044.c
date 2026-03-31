/* Main test file for TLS emulation attribute copying coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For thread testing if pthread is available */
#ifdef _PTHREAD_H
#include <pthread.h>
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

/* DLL import attribute for Windows (conditional) */
#ifdef _WIN32
__thread int imported_tls_var __attribute__((dllimport));
#else
/* On non-Windows, just a regular TLS variable */
__thread int imported_tls_var = 600;
#endif

/* Function to test TLS variable access patterns */
void test_tls_access(void) {
    /* Read from all TLS variables */
    int sum = public_tls_var + weak_tls_var + hidden_tls_var + 
              protected_tls_var + used_tls_var + imported_tls_var +
              external_tls_var + common_tls_var;
    
    /* Write to all TLS variables */
    public_tls_var += 1;
    weak_tls_var += 2;
    hidden_tls_var += 3;
    protected_tls_var += 4;
    used_tls_var += 5;
    imported_tls_var += 6;
    
    /* Take addresses of TLS variables (important for lowering) */
    int *ptrs[] = {
        &public_tls_var,
        &weak_tls_var,
        &hidden_tls_var,
        &protected_tls_var,
        &used_tls_var,
        &imported_tls_var,
        &external_tls_var,
        &common_tls_var
    };
    
    /* Use addresses to prevent optimization */
    for (int i = 0; i < sizeof(ptrs)/sizeof(ptrs[0]); i++) {
        *ptrs[i] += i;
    }
    
    /* Complex expression using TLS variables */
    int result = (public_tls_var * weak_tls_var) - 
                 (hidden_tls_var / (protected_tls_var + 1)) +
                 (used_tls_var ^ imported_tls_var);
    
    printf("TLS test result: %d (sum was %d)\n", result, sum);
}

#ifdef _PTHREAD_H
/* Thread function to test TLS in multiple threads */
void *thread_func(void *arg) {
    int thread_id = *(int*)arg;
    
    /* Each thread gets its own TLS values */
    public_tls_var = 1000 + thread_id;
    weak_tls_var = 2000 + thread_id;
    
    printf("Thread %d: public_tls_var=%d, weak_tls_var=%d\n",
           thread_id, public_tls_var, weak_tls_var);
    
    return NULL;
}

void test_tls_threads(void) {
    pthread_t threads[2];
    int thread_ids[2] = {1, 2};
    
    for (int i = 0; i < 2; i++) {
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }
    
    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }
}
#endif

/* Main function that uses all TLS variables */
int main(void) {
    printf("Testing TLS emulation attribute copying...\n");
    
    /* Initial test */
    test_tls_access();
    
    /* Test with threads if available */
#ifdef _PTHREAD_H
    test_tls_threads();
#endif
    
    /* Final verification */
    printf("Final TLS values:\n");
    printf("  public_tls_var: %d\n", public_tls_var);
    printf("  weak_tls_var: %d\n", weak_tls_var);
    printf("  hidden_tls_var: %d\n", hidden_tls_var);
    printf("  protected_tls_var: %d\n", protected_tls_var);
    printf("  used_tls_var: %d\n", used_tls_var);
    printf("  imported_tls_var: %d\n", imported_tls_var);
    printf("  external_tls_var: %d\n", external_tls_var);
    printf("  common_tls_var: %d\n", common_tls_var);
    
    return 0;
}
