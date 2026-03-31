/* test_tls_coverage.c - Program to trigger TLS declaration attribute copying */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    static atomic_int counter = 0;
    return atomic_fetch_add(&counter, 1) + 42;
}

/* 1. Extern TLS with hidden visibility - will have DECL_VISIBILITY set */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS - will have DECL_WEAK set */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public Common TLS - will have DECL_COMMON and TREE_PUBLIC set */
__thread int tls_var3;

/* 4. Static TLS with DLL import simulation */
/* For MinGW/Windows targets, use: __declspec(dllimport) __thread int tls_var4; */
/* For Linux/Unix, simulate with attribute: */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_var4;
#else
    /* Use a visibility attribute to ensure DECL_VISIBILITY_SPECIFIED is set */
    __thread int tls_var4 __attribute__((visibility("default")));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. Another extern TLS with explicit default visibility */
extern __thread int tls_var5 __attribute__((visibility("default"), used));

/* Define the extern TLS variables */
__thread int tls_var1 = 1;
__thread int tls_var5 = 5;

/* Thread function that accesses all TLS variables */
static void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    int* addr1 = &tls_var1;
    int* addr2 = &tls_var2;
    int* addr3 = &tls_var3;
    int* addr4 = &tls_var4;
    int* addr5 = &tls_var5;
    int* addr_init = &tls_init;
    
    /* Prevent optimization by using volatile */
    volatile int* vptr = NULL;
    
    /* Conditional access to ensure variables aren't optimized away */
    if (thread_id % 2 == 0) {
        vptr = addr1;
        tls_var1 = 100 + thread_id;
    } else {
        vptr = addr2;
        tls_var2 = 200 + thread_id;
    }
    
    /* All threads access these */
    tls_var3 = 300 + thread_id;
    tls_var4 = 400 + thread_id;
    tls_var5 = 500 + thread_id;
    tls_init = 600 + thread_id;
    
    /* Verify writes by reading back */
    int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    
    /* Use the volatile pointer to prevent dead code elimination */
    if (vptr) {
        sum += *vptr;
    }
    
    /* Return the thread-local sum */
    int* result = malloc(sizeof(int));
    *result = sum;
    return result;
}

int main(void) {
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int* thread_results[NUM_THREADS];
    
    /* Initialize TLS with non-constant initializer */
    tls_init = get_initial_value();
    
    /* Force external linkage reference */
    extern int tls_var1;  /* Forward declaration to ensure external linkage is considered */
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Main thread also accesses TLS variables */
    tls_var1 = 1000;
    tls_var2 = 2000;
    tls_var3 = 3000;
    tls_var4 = 4000;
    tls_var5 = 5000;
    
    /* Take addresses in main thread too */
    int* addrs[] = {&tls_var1, &tls_var2, &tls_var3, &tls_var4, &tls_var5, &tls_init};
    
    /* Join threads and collect results */
    int total_sum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], (void**)&thread_results[i]) != 0) {
            perror("pthread_join");
            return 1;
        }
        total_sum += *thread_results[i];
        free(thread_results[i]);
    }
    
    /* Add main thread's TLS values */
    total_sum += tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5 + tls_init;
    
    /* Use the addresses to prevent optimization */
    for (int i = 0; i < 6; i++) {
        total_sum += (*addrs[i] % 7);  /* Arbitrary computation using addresses */
    }
    
    printf("TLS test completed. Checksum: %d\n", total_sum);
    printf("Main thread TLS values: %d, %d, %d, %d, %d, %d\n",
           tls_var1, tls_var2, tls_var3, tls_var4, tls_var5, tls_init);
    
    /* Verify that TLS variables have thread-local storage */
    if (tls_var1 == 1000 && tls_var3 == 3000) {
        printf("TLS appears to be working correctly (main thread values preserved)\n");
    }
    
    return 0;
}
