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

/* 3. Public common TLS with non-constant initializer */
__thread int tls_var3 = 0;  /* May become common */

/* 4. Static TLS with DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* Simulate DLL import attribute for non-Windows */
__thread int tls_var4 __attribute__((visibility("default"), used));
#endif

/* 5. Another TLS with explicit visibility */
__thread int tls_var5 __attribute__((visibility("default"), used));

/* Define the extern TLS variable */
__thread int tls_var1 = 42;

/* Non-constant initialized TLS */
__thread int tls_init_var = 0;

/* Global array to collect results from threads */
static int thread_results[4][5] = {0};
static pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    
    /* Force ODR-use by taking addresses */
    int* addr1 = &tls_var1;
    int* addr2 = &tls_var2;
    int* addr3 = &tls_var3;
    int* addr4 = &tls_var4;
    int* addr5 = &tls_var5;
    
    /* Prevent optimization of address-taking */
    volatile int dummy = (int)((long)addr1 ^ (long)addr2 ^ (long)addr3 ^ 
                               (long)addr4 ^ (long)addr5);
    (void)dummy;
    
    /* Assign unique values to TLS variables */
    tls_var1 = 100 + thread_id;
    tls_var2 = 200 + thread_id;
    tls_var3 = 300 + thread_id;
    tls_var4 = 400 + thread_id;
    tls_var5 = 500 + thread_id;
    
    /* Initialize non-constant TLS if not done */
    if (tls_init_var == 0) {
        tls_init_var = some_function() + thread_id;
    }
    
    /* Read back values (ensures compiler can't optimize away writes) */
    pthread_mutex_lock(&result_mutex);
    thread_results[thread_id][0] = tls_var1;
    thread_results[thread_id][1] = tls_var2;
    thread_results[thread_id][2] = tls_var3;
    thread_results[thread_id][3] = tls_var4;
    thread_results[thread_id][4] = tls_var5;
    pthread_mutex_unlock(&result_mutex);
    
    /* Conditional use to prevent dead code elimination */
    if (tls_var1 > 50) {
        printf("Thread %d: tls_var1 = %d\n", thread_id, tls_var1);
    }
    
    return NULL;
}

int main(void) {
    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};
    
    /* Initialize non-constant TLS in main thread */
    tls_init_var = some_function();
    
    /* Initialize TLS variables in main thread */
    tls_var1 = 42;
    tls_var2 = 43;
    tls_var3 = 44;
    tls_var4 = 45;
    tls_var5 = 46;
    
    /* Create multiple threads to force TLS emulation */
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Main thread also accesses TLS */
    tls_var1 = 99;
    tls_var2 = 98;
    tls_var3 = 97;
    tls_var4 = 96;
    tls_var5 = 95;
    
    /* Store main thread's results */
    thread_results[0][0] = tls_var1;
    thread_results[0][1] = tls_var2;
    thread_results[0][2] = tls_var3;
    thread_results[0][3] = tls_var4;
    thread_results[0][4] = tls_var5;
    
    /* Join all threads */
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    
    /* Verify TLS isolation by computing checksum */
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            checksum ^= thread_results[i][j];
        }
    }
    
    /* Print results to ensure execution */
    printf("Main thread TLS values: %d, %d, %d, %d, %d\n",
           tls_var1, tls_var2, tls_var3, tls_var4, tls_var5);
    printf("tls_init_var = %d\n", tls_init_var);
    printf("Result checksum: 0x%08x\n", checksum);
    
    /* Additional conditional use of TLS addresses */
    if (&tls_var1 != &tls_var2) {
        printf("TLS variables have different addresses (as expected)\n");
    }
    
    return 0;
}
