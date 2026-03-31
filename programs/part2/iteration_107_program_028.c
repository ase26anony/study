/* Main test file for TLS emulation attribute coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations for TLS variables defined in other files */
extern __thread int external_tls_var;
extern __thread int common_tls_var;

/* Public TLS variable - sets TREE_PUBLIC */
__thread int public_tls_var = 100;

/* Weak TLS variable */
__thread int weak_tls_var __attribute__((weak)) = 200;

/* Visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 300;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 400;

/* Used attribute - influences TREE_USED */
__thread int used_tls_var __attribute__((used)) = 500;

/* DLL import attribute (Windows-specific) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls_var;
#else
/* On non-Windows, we'll just declare it normally */
__thread int imported_tls_var = 600;
#endif

/* Function to take addresses of TLS variables (affects lowering) */
void* get_tls_addresses(void) {
    static void* addresses[8];
    
    addresses[0] = &public_tls_var;
    addresses[1] = &weak_tls_var;
    addresses[2] = &hidden_tls_var;
    addresses[3] = &protected_tls_var;
    addresses[4] = &used_tls_var;
    addresses[5] = &imported_tls_var;
    addresses[6] = &external_tls_var;
    addresses[7] = &common_tls_var;
    
    return addresses;
}

/* Complex usage to prevent optimization */
int compute_checksum(void) {
    int sum = 0;
    
    sum += public_tls_var;
    sum += weak_tls_var * 2;
    sum += hidden_tls_var * 3;
    sum += protected_tls_var * 4;
    sum += used_tls_var * 5;
    sum += imported_tls_var * 6;
    sum += external_tls_var * 7;
    sum += common_tls_var * 8;
    
    /* Modify TLS variables */
    public_tls_var += 1;
    weak_tls_var += 2;
    hidden_tls_var += 3;
    protected_tls_var += 4;
    used_tls_var += 5;
    imported_tls_var += 6;
    
    return sum;
}

#ifdef USE_THREADS
#include <pthread.h>

/* Thread function accessing TLS */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Each thread gets its own TLS values */
    public_tls_var = 1000 + thread_id;
    weak_tls_var = 2000 + thread_id;
    
    int local_sum = compute_checksum();
    
    printf("Thread %d: public_tls_var = %d, checksum = %d\n", 
           thread_id, public_tls_var, local_sum);
    
    return (void*)(long)local_sum;
}

void test_with_threads(void) {
    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};
    
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }
    
    for (int i = 0; i < 3; i++) {
        void* result;
        pthread_join(threads[i], &result);
        printf("Thread %d returned checksum: %ld\n", i+1, (long)result);
    }
}
#endif

int main(void) {
    printf("Testing TLS emulation with various attributes\n");
    
    /* Initialize external variables */
    external_tls_var = 700;
    common_tls_var = 800;
    
    /* Get addresses to force address-taking */
    get_tls_addresses();
    
    /* Compute initial checksum */
    int checksum1 = compute_checksum();
    printf("Initial checksum: %d\n", checksum1);
    
    /* Compute again to show modification */
    int checksum2 = compute_checksum();
    printf("Modified checksum: %d\n", checksum2);
    
    /* Print all values */
    printf("\nTLS variable values:\n");
    printf("public_tls_var: %d\n", public_tls_var);
    printf("weak_tls_var: %d\n", weak_tls_var);
    printf("hidden_tls_var: %d\n", hidden_tls_var);
    printf("protected_tls_var: %d\n", protected_tls_var);
    printf("used_tls_var: %d\n", used_tls_var);
    printf("imported_tls_var: %d\n", imported_tls_var);
    printf("external_tls_var: %d\n", external_tls_var);
    printf("common_tls_var: %d\n", common_tls_var);
    
#ifdef USE_THREADS
    printf("\nTesting with threads:\n");
    test_with_threads();
#endif
    
    /* Final verification */
    int final_sum = public_tls_var + weak_tls_var + hidden_tls_var +
                   protected_tls_var + used_tls_var + imported_tls_var +
                   external_tls_var + common_tls_var;
    
    printf("\nFinal sum of all TLS variables: %d\n", final_sum);
    
    return (final_sum > 0) ? 0 : 1;
}
