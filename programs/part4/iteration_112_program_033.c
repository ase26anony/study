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

/* 3. Public common TLS with non-constant initializer */
__thread int tls_var3 = 0;  /* Will be initialized in main() */

/* 4. Static TLS (internal linkage) */
static __thread int tls_var4 __attribute__((used));

/* 5. DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var5;
#else
/* Simulate with visibility attribute */
__thread int tls_var5 __attribute__((visibility("default"), used));
#endif

/* Global array to collect results from threads */
static int thread_results[4][5] = {0};
static pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    int base_value = 100 + thread_id * 10;
    
    /* Access and modify all TLS variables */
    tls_var1 = base_value + 1;
    tls_var2 = base_value + 2;
    tls_var3 = base_value + 3;
    tls_var4 = base_value + 4;
    tls_var5 = base_value + 5;
    
    /* Read back values (ensuring ODR-use) */
    int* addrs[5];
    addrs[0] = &tls_var1;  /* Take address to force TLS setup */
    addrs[1] = &tls_var2;
    addrs[2] = &tls_var3;
    addrs[3] = &tls_var4;
    addrs[4] = &tls_var5;
    
    /* Store results with mutex protection */
    pthread_mutex_lock(&result_mutex);
    for (int i = 0; i < 5; i++) {
        thread_results[thread_id][i] = *addrs[i];
    }
    pthread_mutex_unlock(&result_mutex);
    
    /* Conditional use to prevent optimization */
    if (tls_var1 > 0) {
        tls_var2 += thread_id;
    }
    
    return NULL;
}

/* External declaration to force external linkage resolution */
extern __thread int tls_var1;

int main(void) {
    pthread_t threads[4];
    int thread_ids[4];
    
    /* Initialize tls_var3 with non-constant value */
    tls_var3 = get_initial_value();
    
    /* Take addresses of TLS variables in main thread */
    volatile int* force_use = &tls_var1;
    (void)force_use;  /* Suppress unused warning */
    
    /* Create 4 threads */
    for (int i = 0; i < 4; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Wait for all threads to complete */
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    /* Main thread accesses its own TLS instances */
    tls_var1 = 999;
    tls_var2 = 888;
    tls_var3 = 777;
    tls_var4 = 666;
    tls_var5 = 555;
    
    /* Verify thread results */
    printf("Thread TLS results:\n");
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        printf("Thread %d: ", i);
        for (int j = 0; j < 5; j++) {
            printf("%d ", thread_results[i][j]);
            checksum += thread_results[i][j];
        }
        printf("\n");
    }
    
    /* Main thread's TLS values */
    printf("Main thread TLS: %d %d %d %d %d\n", 
           tls_var1, tls_var2, tls_var3, tls_var4, tls_var5);
    
    checksum += tls_var1 + tls_var2 + tls_var3 + tls_var4 + tls_var5;
    printf("Checksum: %d\n", checksum);
    
    /* Additional conditional use to prevent dead code elimination */
    if (checksum > 0) {
        printf("TLS emulation was used (checksum positive)\n");
    }
    
    pthread_mutex_destroy(&result_mutex);
    return 0;
}

/* Definition of the extern TLS variable */
__thread int tls_var1 __attribute__((visibility("hidden"), used)) = 42;

/* Weak definition */
__thread int tls_var2 __attribute__((weak, used)) = 24;
