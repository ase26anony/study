#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    static atomic_int counter = 0;
    return atomic_fetch_add(&counter, 1) + 42;
}

/* TLS variables with various attributes to trigger declaration cloning */

/* 1. Extern TLS with hidden visibility - should set DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - should set DECL_WEAK */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS (implicitly extern) - should set TREE_PUBLIC, DECL_EXTERNAL, DECL_COMMON */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS - should have different linkage */
static __thread int tls_var4 __attribute__((used));

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init __attribute__((used)) = get_initial_value();

/* 6. For platforms supporting dllimport */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dll __attribute__((used));
#else
    /* Simulate DLL import attribute for non-Windows */
    extern __thread int tls_dll __attribute__((used));
#endif

/* Define the extern TLS variables */
__thread int tls_var1 = 10;
__thread int tls_var2 = 20;
__thread int tls_var3 = 30;
__thread int tls_dll = 60;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Take addresses to force ODR-use and prevent optimization */
    int* addrs[] = {
        &tls_var1,
        &tls_var2, 
        &tls_var3,
        &tls_var4,
        &tls_init,
        &tls_dll
    };
    
    /* Write thread-specific values */
    tls_var1 = 100 + thread_id;
    tls_var2 = 200 + thread_id;
    tls_var3 = 300 + thread_id;
    tls_var4 = 400 + thread_id;
    tls_init = 500 + thread_id;
    tls_dll = 600 + thread_id;
    
    /* Read back and verify (with volatile to prevent optimization) */
    volatile int read_back[6];
    read_back[0] = tls_var1;
    read_back[1] = tls_var2;
    read_back[2] = tls_var3;
    read_back[3] = tls_var4;
    read_back[4] = tls_init;
    read_back[5] = tls_dll;
    
    /* Conditional use based on thread_id to prevent dead code elimination */
    int result = 0;
    if (thread_id % 2 == 0) {
        for (int i = 0; i < 6; i++) {
            result += read_back[i];
        }
    } else {
        for (int i = 0; i < 6; i++) {
            result -= read_back[i];
        }
    }
    
    /* Return the checksum */
    int* ret = malloc(sizeof(int));
    *ret = result;
    return ret;
}

int main(void) {
    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};
    int* results[3];
    
    /* Access TLS in main thread first */
    tls_var1 = 1000;
    tls_var2 = 2000;
    tls_var3 = 3000;
    tls_var4 = 4000;
    tls_init = 5000;
    tls_dll = 6000;
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Join threads and collect results */
    for (int i = 0; i < 3; i++) {
        if (pthread_join(threads[i], (void**)&results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
    }
    
    /* Calculate final checksum */
    int final_checksum = 0;
    
    /* Include main thread's TLS values */
    final_checksum += tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init + tls_dll;
    
    /* Include thread results */
    for (int i = 0; i < 3; i++) {
        final_checksum += *results[i];
        free(results[i]);
    }
    
    printf("TLS test completed successfully. Final checksum: %d\n", final_checksum);
    printf("Main thread TLS values: %d, %d, %d, %d, %d, %d\n", 
           tls_var1, tls_var2, tls_var3, tls_var4, tls_init, tls_dll);
    
    /* Verify that main thread values weren't corrupted by other threads */
    if (tls_var1 == 1000 && tls_var2 == 2000 && tls_var3 == 3000) {
        printf("TLS isolation verified: each thread has its own instance\n");
        return 0;
    } else {
        printf("ERROR: TLS isolation failed!\n");
        return 1;
    }
}
