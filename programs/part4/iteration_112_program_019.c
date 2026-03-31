#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    static atomic_int counter = 0;
    return ++counter;
}

/* 1. Extern TLS with hidden visibility */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS with external linkage */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* Simulate DLL import attribute for non-Windows */
__thread int tls_var4 __attribute__((dllimport, used));
#endif

/* 5. TLS with non-constant initializer - forces emulation complexity */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* Define the extern TLS variable */
__thread int tls_var1 = 42;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Access and modify all TLS variables */
    tls_var1 = 1000 + thread_id;
    tls_var2 = 2000 + thread_id;
    tls_var3 = 3000 + thread_id;
    tls_var4 = 4000 + thread_id;
    tls_init = 5000 + thread_id;
    
    /* Read back and verify (ODR-use through taking address) */
    int* addrs[5];
    addrs[0] = &tls_var1;
    addrs[1] = &tls_var2;
    addrs[2] = &tls_var3;
    addrs[3] = &tls_var4;
    addrs[4] = &tls_init;
    
    /* Conditional access that prevents optimization */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        if (thread_id % 2 == 0) {
            sum += *addrs[i];
        } else {
            sum -= *addrs[i];
        }
    }
    
    /* Store result in heap for main thread to collect */
    int* result = malloc(sizeof(int));
    *result = sum;
    
    return result;
}

int main(void) {
    /* Initialize non-constant TLS variable */
    tls_init = get_initial_value();
    
    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};
    int* thread_results[3];
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Main thread also accesses TLS variables */
    tls_var1 = 999;
    tls_var2 = 888;
    tls_var3 = 777;
    tls_var4 = 666;
    tls_init = 555;
    
    /* Join threads and collect results */
    int total_sum = 0;
    for (int i = 0; i < 3; i++) {
        if (pthread_join(threads[i], (void**)&thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *thread_results[i];
        free(thread_results[i]);
    }
    
    /* Add main thread's TLS values to total */
    int* main_addrs[5];
    main_addrs[0] = &tls_var1;
    main_addrs[1] = &tls_var2;
    main_addrs[2] = &tls_var3;
    main_addrs[3] = &tls_var4;
    main_addrs[4] = &tls_init;
    
    for (int i = 0; i < 5; i++) {
        total_sum += *main_addrs[i];
    }
    
    printf("Total checksum from all TLS accesses: %d\n", total_sum);
    printf("If this prints without crashing, TLS emulation likely worked.\n");
    
    return 0;
}
