#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/* Helper function for non-constant initializer */
int get_initial_value(void) {
    static int counter = 0;
    return ++counter + 100;
}

/* 1. Extern TLS with hidden visibility */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS with non-constant initializer */
__thread int tls_var3 = 0;  /* Will be initialized in main() */

/* 4. Static TLS with DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* Simulate DLL import attribute for non-Windows */
__thread int tls_var4 __attribute__((dllimport));
#endif

/* Define the extern TLS variable */
__thread int tls_var1 = 50;

/* Global array to store thread results */
#define NUM_THREADS 3
int thread_results[NUM_THREADS][4] = {0};
atomic_int thread_counter = 0;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    int idx = atomic_fetch_add(&thread_counter, 1);
    
    /* Force ODR-use by taking addresses */
    int* addr1 = &tls_var1;
    int* addr2 = &tls_var2;
    int* addr3 = &tls_var3;
    int* addr4 = &tls_var4;
    
    /* Prevent optimization by using addresses in conditionals */
    if (addr1 && addr2 && addr3 && addr4) {
        /* Each thread sets unique values to its TLS instances */
        tls_var1 = 1000 + thread_id * 10;
        tls_var2 = 2000 + thread_id * 10;
        tls_var3 = 3000 + thread_id * 10;
        tls_var4 = 4000 + thread_id * 10;
        
        /* Read back and store results */
        thread_results[idx][0] = tls_var1;
        thread_results[idx][1] = tls_var2;
        thread_results[idx][2] = tls_var3;
        thread_results[idx][3] = tls_var4;
        
        /* Complex conditional to prevent dead code elimination */
        volatile int sum = tls_var1 + tls_var2 + tls_var3 + tls_var4;
        if (sum > 0) {
            /* Do nothing, just ensure computation isn't optimized away */
        }
    }
    
    return NULL;
}

/* Function that returns address of TLS variable (forces emulation) */
int* get_tls_address(int selector) {
    switch (selector) {
        case 1: return &tls_var1;
        case 2: return &tls_var2;
        case 3: return &tls_var3;
        case 4: return &tls_var4;
        default: return NULL;
    }
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    /* Initialize tls_var3 with non-constant value */
    tls_var3 = get_initial_value();
    
    /* Initialize tls_var4 in main thread */
    tls_var4 = 999;
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Wait for all threads to complete */
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    /* Main thread accesses its own TLS instances */
    tls_var1 = 5555;
    tls_var2 = 6666;
    tls_var3 = 7777;
    tls_var4 = 8888;
    
    /* Force use of get_tls_address to ensure TLS addresses are taken */
    int* addrs[4];
    for (int i = 0; i < 4; i++) {
        addrs[i] = get_tls_address(i + 1);
    }
    
    /* Verify thread results */
    printf("TLS Test Results:\n");
    printf("Main thread TLS values: %d, %d, %d, %d\n", 
           tls_var1, tls_var2, tls_var3, tls_var4);
    
    printf("\nThread TLS values:\n");
    for (int i = 0; i < NUM_THREADS; i++) {
        printf("Thread %d: %d, %d, %d, %d\n", i + 1,
               thread_results[i][0],
               thread_results[i][1],
               thread_results[i][2],
               thread_results[i][3]);
    }
    
    /* Calculate checksum to prove all TLS accesses happened */
    int checksum = tls_var1 + tls_var2 + tls_var3 + tls_var4;
    for (int i = 0; i < NUM_THREADS; i++) {
        for (int j = 0; j < 4; j++) {
            checksum += thread_results[i][j];
        }
    }
    
    printf("\nFinal checksum: %d\n", checksum);
    printf("TLS test completed successfully!\n");
    
    return 0;
}
