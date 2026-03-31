/* test_tls_coverage.c - Program to trigger TLS declaration cloning logic */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

/* Helper function for non-constant initializer */
static int get_initial_value(void) {
    return 42;
}

/* 1. Extern TLS with hidden visibility */
extern __thread int tls_var1 __attribute__((visibility("hidden"), used));

/* 2. Weak TLS variable */
__thread int tls_var2 __attribute__((weak, used));

/* 3. Public common TLS (implicitly extern) */
__thread int tls_var3 __attribute__((used));

/* 4. Static TLS with DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_var4;
#else
/* Simulate DLL import attribute for non-Windows */
__thread int tls_var4 __attribute__((dllimport));
#endif

/* 5. TLS with non-constant initializer - forces complex initialization */
__thread int tls_init = 0;  /* Will be initialized in main() */

/* 6. TLS with default visibility */
__thread int tls_visible __attribute__((visibility("default"), used));

/* Define the extern TLS variables */
__thread int tls_var1 = 10;
__thread int tls_var2 = 20;
__thread int tls_var3 = 30;
__thread int tls_var4 = 40;
__thread int tls_visible = 50;

/* Global array to collect results from threads */
static int thread_results[4][6] = {0};
static volatile int result_index = 0;

/* Thread function that accesses all TLS variables */
void* thread_func(void* arg) {
    int thread_id = *(int*)arg;
    int local_idx = __sync_fetch_and_add(&result_index, 1);
    
    /* Take addresses of TLS variables to force ODR-use */
    int* addr1 = &tls_var1;
    int* addr2 = &tls_var2;
    int* addr3 = &tls_var3;
    int* addr4 = &tls_var4;
    int* addr_init = &tls_init;
    int* addr_vis = &tls_visible;
    
    /* Prevent optimization from removing address-taking */
    volatile int dummy = (uintptr_t)addr1 + (uintptr_t)addr2 + 
                        (uintptr_t)addr3 + (uintptr_t)addr4 +
                        (uintptr_t)addr_init + (uintptr_t)addr_vis;
    (void)dummy;
    
    /* Assign thread-specific values to TLS variables */
    tls_var1 = 100 + thread_id;
    tls_var2 = 200 + thread_id;
    tls_var3 = 300 + thread_id;
    tls_var4 = 400 + thread_id;
    tls_init = 500 + thread_id;
    tls_visible = 600 + thread_id;
    
    /* Read back values with conditional logic to prevent optimization */
    if (thread_id % 2 == 0) {
        thread_results[local_idx][0] = tls_var1;
        thread_results[local_idx][1] = tls_var2;
        thread_results[local_idx][2] = tls_var3;
    } else {
        thread_results[local_idx][3] = tls_var4;
        thread_results[local_idx][4] = tls_init;
        thread_results[local_idx][5] = tls_visible;
    }
    
    /* Cross-check values */
    if (tls_var1 != (100 + thread_id) ||
        tls_var2 != (200 + thread_id) ||
        tls_var3 != (300 + thread_id)) {
        /* This should never happen with proper TLS */
        thread_results[local_idx][0] = -1;
    }
    
    return NULL;
}

/* Function that returns address of TLS variable - forces emulation */
int* get_tls_address(int selector) {
    switch (selector) {
        case 0: return &tls_var1;
        case 1: return &tls_var2;
        case 2: return &tls_var3;
        case 3: return &tls_var4;
        case 4: return &tls_init;
        case 5: return &tls_visible;
        default: return NULL;
    }
}

int main(void) {
    pthread_t threads[4];
    int thread_ids[4];
    int i;
    
    /* Initialize TLS with non-constant value */
    tls_init = get_initial_value();
    
    /* Force reference to all TLS variables in main thread */
    volatile int* addrs[6];
    for (i = 0; i < 6; i++) {
        addrs[i] = get_tls_address(i);
    }
    
    /* Create multiple threads to force TLS emulation setup */
    for (i = 0; i < 4; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    /* Main thread also uses TLS variables */
    tls_var1 = 1000;
    tls_var2 = 2000;
    tls_var3 = 3000;
    tls_var4 = 4000;
    tls_init = 5000;
    tls_visible = 6000;
    
    /* Join all threads */
    for (i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    /* Verify TLS isolation by checking main thread's values are unchanged */
    int checksum = 0;
    checksum += (tls_var1 == 1000) ? 1 : 0;
    checksum += (tls_var2 == 2000) ? 2 : 0;
    checksum += (tls_var3 == 3000) ? 4 : 0;
    checksum += (tls_var4 == 4000) ? 8 : 0;
    checksum += (tls_init == 5000) ? 16 : 0;
    checksum += (tls_visible == 6000) ? 32 : 0;
    
    /* Also check thread results */
    int thread_checksum = 0;
    for (i = 0; i < 4; i++) {
        for (int j = 0; j < 6; j++) {
            if (thread_results[i][j] != 0) {
                thread_checksum += thread_results[i][j];
            }
        }
    }
    
    printf("Main thread TLS checksum: %d (expected: 63)\n", checksum);
    printf("Thread results checksum: %d\n", thread_checksum);
    printf("All TLS variables accessed via: ");
    for (i = 0; i < 6; i++) {
        printf("%p ", (void*)addrs[i]);
    }
    printf("\n");
    
    /* Final verification */
    if (checksum == 63) {
        printf("SUCCESS: TLS appears to be working correctly\n");
        return 0;
    } else {
        printf("WARNING: TLS checksum mismatch\n");
        return 1;
    }
}
