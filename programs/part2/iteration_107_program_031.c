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

/* Public TLS variable - sets TREE_PUBLIC */
__thread int public_tls_var;

/* Weak TLS variable - sets DECL_WEAK */
__thread int weak_tls_var __attribute__((weak));

/* Visibility attributes - sets DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int hidden_tls_var __attribute__((visibility("hidden")));
__thread int protected_tls_var __attribute__((visibility("protected")));

/* Used attribute - influences TREE_USED */
__thread int used_tls_var __attribute__((used));

/* DLL import attribute (Windows-specific) */
#ifdef _WIN32
__thread int imported_tls_var __attribute__((dllimport));
#else
/* On non-Windows, just a regular TLS variable */
__thread int imported_tls_var;
#endif

/* Function to take addresses of TLS variables (affects lowering) */
void take_tls_addresses(void) {
    int *ptr;
    
    ptr = &public_tls_var;
    *ptr = 0xDEADBEEF;
    
    ptr = &weak_tls_var;
    *ptr = 0xCAFEBABE;
    
    ptr = &hidden_tls_var;
    *ptr = 0x12345678;
    
    ptr = &protected_tls_var;
    *ptr = 0x87654321;
    
    ptr = &used_tls_var;
    *ptr = 0x11111111;
    
    ptr = &imported_tls_var;
    *ptr = 0x22222222;
    
    /* Take address of extern variables */
    ptr = &external_tls_var;
    *ptr = 0x33333333;
    
    ptr = &common_tls_var;
    *ptr = 0x44444444;
}

/* Thread function to access TLS from multiple threads */
#ifdef _PTHREAD_H
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Each thread writes unique values to TLS variables */
    public_tls_var = 1000 + thread_id;
    weak_tls_var = 2000 + thread_id;
    hidden_tls_var = 3000 + thread_id;
    protected_tls_var = 4000 + thread_id;
    used_tls_var = 5000 + thread_id;
    imported_tls_var = 6000 + thread_id;
    
    /* Read back and verify */
    int sum = public_tls_var + weak_tls_var + hidden_tls_var + 
              protected_tls_var + used_tls_var + imported_tls_var;
    
    printf("Thread %d: TLS sum = %d\n", thread_id, sum);
    
    return (void*)(long)sum;
}
#endif

/* Compute checksum of all TLS variables to ensure they're live */
int compute_tls_checksum(void) {
    int checksum = 0;
    
    checksum ^= public_tls_var;
    checksum ^= weak_tls_var;
    checksum ^= hidden_tls_var;
    checksum ^= protected_tls_var;
    checksum ^= used_tls_var;
    checksum ^= imported_tls_var;
    checksum ^= external_tls_var;
    checksum ^= common_tls_var;
    
    /* Use variables in non-trivial expressions */
    public_tls_var = (public_tls_var * 3) / 2;
    weak_tls_var = (weak_tls_var << 2) | 1;
    hidden_tls_var = hidden_tls_var ^ 0x55555555;
    protected_tls_var = protected_tls_var + 1;
    used_tls_var = used_tls_var - 100;
    imported_tls_var = imported_tls_var * 2;
    
    return checksum;
}

int main(void) {
    int result = 0;
    
    printf("Testing TLS emulation attribute copying...\n");
    
    /* Initialize TLS variables with distinct values */
    public_tls_var = 1;
    weak_tls_var = 2;
    hidden_tls_var = 3;
    protected_tls_var = 4;
    used_tls_var = 5;
    imported_tls_var = 6;
    
    /* Take addresses to affect lowering */
    take_tls_addresses();
    
    /* Access extern variables */
    external_tls_var = 7;
    common_tls_var = 8;
    
    /* Compute checksum to ensure all variables are used */
    int checksum = compute_tls_checksum();
    printf("Initial TLS checksum: 0x%08X\n", checksum);
    
#ifdef _PTHREAD_H
    /* Test with multiple threads if pthread available */
    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};
    
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }
    
    long thread_results[3];
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], (void**)&thread_results[i]);
        result += (int)thread_results[i];
    }
    
    printf("Thread results sum: %ld\n", result);
#endif
    
    /* Final checksum computation */
    checksum = compute_tls_checksum();
    printf("Final TLS checksum: 0x%08X\n", checksum);
    
    /* Return non-zero if any TLS variable has unexpected value */
    if (public_tls_var == 0 && weak_tls_var == 0 && hidden_tls_var == 0) {
        return 1; /* Likely TLS not working */
    }
    
    return 0;
}
