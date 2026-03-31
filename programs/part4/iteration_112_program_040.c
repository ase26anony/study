#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/* Helper function for non-constant initializer */
static int some_function(void) {
    static atomic_int counter = 0;
    return atomic_fetch_add(&counter, 1) + 1000;
}

/* 1. Extern TLS with hidden visibility */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public Common TLS (implicitly extern) */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* Simulate DLL import attribute for non-Windows */
__thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* Define the extern TLS variables */
__thread int tls_var1 = 10;
__thread int tls_var2 = 20;
__thread int tls_var3 = 30;
__thread int tls_var4 = 40;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    int* addr1 = &tls_var1;
    int* addr2 = &tls_var2;
    int* addr3 = &tls_var3;
    int* addr4 = &tls_var4;
    int* addr_init = &tls_init;
    
    /* Prevent optimization by using addresses in conditionals */
    if (addr1 && addr2 && addr3 && addr4 && addr_init) {
        /* Each thread writes unique values to TLS */
        tls_var1 = 100 + thread_id;
        tls_var2 = 200 + thread_id;
        tls_var3 = 300 + thread_id;
        tls_var4 = 400 + thread_id;
        tls_init = 500 + thread_id;
        
        /* Read back and verify */
        int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init;
        
        /* Store result in heap to prevent optimization */
        int* result = malloc(sizeof(int));
        *result = sum;
        
        /* Conditional use of addresses to prevent dead code elimination */
        if (thread_id % 2 == 0) {
            volatile int dummy = *addr1 + *addr2;
            (void)dummy;
        }
        
        return result;
    }
    
    return NULL;
}

int main(void) {
    /* Initialize non-constant TLS variable */
    tls_init = some_function();
    
    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};
    int* results[3];
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < 3; i++) {
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
    tls_init = 5000;
    
    /* Join threads and collect results */
    int total_sum = 0;
    for (int i = 0; i < 3; i++) {
        void* retval;
        pthread_join(threads[i], &retval);
        if (retval) {
            results[i] = (int*)retval;
            total_sum += *results[i];
            free(results[i]);
        }
    }
    
    /* Add main thread's TLS values */
    total_sum += tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_init;
    
    /* Print result to ensure execution */
    printf("Total checksum from all TLS accesses: %d\n", total_sum);
    printf("Expected pattern: Each thread's sum = (100+200+300+400+500) + 5*thread_id\n");
    printf("Main thread sum = 1000+2000+3000+4000+5000 = 15000\n");
    
    /* Additional conditional access to force declaration cloning */
    volatile int* volatile_addr = &tls_var1;
    if (total_sum > 0) {
        *volatile_addr = *volatile_addr + 1;
    }
    
    /* Access through function pointer to prevent optimization */
    int (*volatile get_tls)(void) = (int (*)(void))&tls_var2;
    (void)get_tls;
    
    return 0;
}
