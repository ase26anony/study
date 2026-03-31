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

/* Weak TLS variable - can be overridden */
__thread int weak_tls_var __attribute__((weak)) = 200;

/* TLS variables with different visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 300;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 400;

/* Used attribute to ensure TREE_USED is set */
__thread int used_tls_var __attribute__((used)) = 500;

/* DLL import attribute for Windows targets */
#ifdef _WIN32
__thread int imported_tls_var __attribute__((dllimport));
#else
/* On non-Windows, just a regular TLS variable */
__thread int imported_tls_var = 600;
#endif

/* Function to test TLS variable access */
void test_tls_access(void) {
    /* Read from and write to each TLS variable */
    public_tls_var += 1;
    weak_tls_var *= 2;
    hidden_tls_var -= 10;
    protected_tls_var /= 2;
    used_tls_var ^= 0xFF;
    
    /* Take addresses to force address-taking code generation */
    int *public_ptr = &public_tls_var;
    int *weak_ptr = &weak_tls_var;
    int *hidden_ptr = &hidden_tls_var;
    int *protected_ptr = &protected_tls_var;
    int *used_ptr = &used_tls_var;
    int *imported_ptr = &imported_tls_var;
    
    /* Use pointers to prevent optimization */
    *public_ptr += 1;
    *weak_ptr += 2;
    
    /* Access external/common TLS variables */
    external_tls_var = 999;
    common_tls_var = 888;
    
    /* Complex expressions with TLS variables */
    int result = public_tls_var + weak_tls_var + hidden_tls_var + 
                 protected_tls_var + used_tls_var + imported_tls_var +
                 external_tls_var + common_tls_var;
    
    printf("TLS sum result: %d\n", result);
    printf("Public TLS: %d (addr: %p)\n", public_tls_var, public_ptr);
    printf("Weak TLS: %d\n", weak_tls_var);
    printf("Hidden TLS: %d\n", hidden_tls_var);
    printf("Protected TLS: %d\n", protected_tls_var);
    printf("Used TLS: %d\n", used_tls_var);
    printf("Imported TLS: %d\n", imported_tls_var);
    printf("External TLS: %d\n", external_tls_var);
    printf("Common TLS: %d\n", common_tls_var);
}

#ifdef _PTHREAD_H
/* Thread function to test TLS in multiple threads */
void *thread_func(void *arg) {
    int thread_id = *(int*)arg;
    
    /* Each thread gets its own TLS instance */
    public_tls_var = 1000 + thread_id;
    weak_tls_var = 2000 + thread_id;
    
    printf("Thread %d: public_tls_var = %d, weak_tls_var = %d\n",
           thread_id, public_tls_var, weak_tls_var);
    
    /* Take address in thread context */
    int *local_ptr = &public_tls_var;
    *local_ptr += thread_id * 100;
    
    return NULL;
}

void test_threaded_tls(void) {
    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};
    
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }
    
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
}
#endif

/* Main function that uses all TLS variables */
int main(void) {
    printf("Testing TLS emulation attribute copying...\n");
    
    /* Initial test */
    test_tls_access();
    
    /* Modify and test again */
    public_tls_var = 1234;
    weak_tls_var = 5678;
    hidden_tls_var = 9012;
    protected_tls_var = 3456;
    used_tls_var = 7890;
    imported_tls_var = 1357;
    
    test_tls_access();
    
#ifdef _PTHREAD_H
    /* Test with threads if available */
    printf("\nTesting TLS with threads...\n");
    test_threaded_tls();
#endif
    
    /* Final verification */
    printf("\nFinal TLS values:\n");
    printf("public_tls_var = %d\n", public_tls_var);
    printf("weak_tls_var = %d\n", weak_tls_var);
    printf("hidden_tls_var = %d\n", hidden_tls_var);
    printf("protected_tls_var = %d\n", protected_tls_var);
    printf("used_tls_var = %d\n", used_tls_var);
    printf("imported_tls_var = %d\n", imported_tls_var);
    printf("external_tls_var = %d\n", external_tls_var);
    printf("common_tls_var = %d\n", common_tls_var);
    
    /* Calculate checksum to ensure all variables are used */
    unsigned int checksum = 
        public_tls_var ^ weak_tls_var ^ hidden_tls_var ^ 
        protected_tls_var ^ used_tls_var ^ imported_tls_var ^
        external_tls_var ^ common_tls_var;
    
    printf("Checksum: 0x%08X\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}
