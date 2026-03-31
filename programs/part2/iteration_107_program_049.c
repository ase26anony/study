/* Main test file for TLS emulation attribute copying coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

/* Forward declarations for TLS variables defined in other files */
extern __thread int external_tls_var;
extern __thread int common_tls_var;

/* Public TLS variable with external linkage */
__thread int public_tls_var = 100;

/* Weak TLS variable - may be overridden by another definition */
__thread int weak_tls_var __attribute__((weak)) = 200;

/* TLS variables with different visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 300;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 400;

/* Used attribute to ensure TREE_USED is set */
__thread int used_tls_var __attribute__((used)) = 500;

/* DLL import attribute (Windows-specific) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls_var;
#else
/* On non-Windows, we'll just define it normally */
__thread int imported_tls_var = 600;
#endif

/* Thread function that accesses all TLS variables */
#ifdef _WIN32
DWORD WINAPI thread_func(LPVOID arg)
#else
void* thread_func(void* arg)
#endif
{
    int thread_id = *(int*)arg;
    
    /* Access all TLS variables to ensure they're used */
    public_tls_var += thread_id;
    weak_tls_var += thread_id * 2;
    hidden_tls_var += thread_id * 3;
    protected_tls_var += thread_id * 4;
    used_tls_var += thread_id * 5;
    imported_tls_var += thread_id * 6;
    
    /* Access external/common TLS variables */
    external_tls_var += thread_id * 7;
    common_tls_var += thread_id * 8;
    
    /* Take addresses to force address-taking code generation */
    int* ptr1 = &public_tls_var;
    int* ptr2 = &weak_tls_var;
    int* ptr3 = &hidden_tls_var;
    int* ptr4 = &protected_tls_var;
    int* ptr5 = &used_tls_var;
    int* ptr6 = &imported_tls_var;
    int* ptr7 = &external_tls_var;
    int* ptr8 = &common_tls_var;
    
    /* Use pointers to prevent optimization */
    *ptr1 += 1;
    *ptr2 += 1;
    *ptr3 += 1;
    *ptr4 += 1;
    *ptr5 += 1;
    *ptr6 += 1;
    *ptr7 += 1;
    *ptr8 += 1;
    
    /* Compute checksum for verification */
    int checksum = public_tls_var + weak_tls_var + hidden_tls_var +
                   protected_tls_var + used_tls_var + imported_tls_var +
                   external_tls_var + common_tls_var;
    
    printf("Thread %d: TLS checksum = %d\n", thread_id, checksum);
    
#ifdef _WIN32
    return 0;
#else
    return (void*)(long)checksum;
#endif
}

int main(void) {
    int i;
    const int num_threads = 3;
    
    printf("Main thread starting TLS emulation test...\n");
    
    /* Initial access to all TLS variables */
    printf("Initial values:\n");
    printf("  public_tls_var = %d\n", public_tls_var);
    printf("  weak_tls_var = %d\n", weak_tls_var);
    printf("  hidden_tls_var = %d\n", hidden_tls_var);
    printf("  protected_tls_var = %d\n", protected_tls_var);
    printf("  used_tls_var = %d\n", used_tls_var);
    printf("  imported_tls_var = %d\n", imported_tls_var);
    printf("  external_tls_var = %d\n", external_tls_var);
    printf("  common_tls_var = %d\n", common_tls_var);
    
    /* Create threads to test TLS in multi-threaded context */
#ifdef _WIN32
    HANDLE threads[num_threads];
    DWORD thread_ids[num_threads];
    
    for (i = 0; i < num_threads; i++) {
        thread_ids[i] = i + 1;
        threads[i] = CreateThread(NULL, 0, thread_func, &thread_ids[i], 0, NULL);
        if (threads[i] == NULL) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return 1;
        }
    }
    
    /* Wait for all threads to complete */
    WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);
    
    /* Close thread handles */
    for (i = 0; i < num_threads; i++) {
        CloseHandle(threads[i]);
    }
#else
    pthread_t threads[num_threads];
    int thread_args[num_threads];
    
    for (i = 0; i < num_threads; i++) {
        thread_args[i] = i + 1;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_args[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return 1;
        }
    }
    
    /* Wait for all threads to complete */
    void* thread_results[num_threads];
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], &thread_results[i]);
        printf("Thread %d returned checksum: %ld\n", 
               i + 1, (long)thread_results[i]);
    }
#endif
    
    /* Final access in main thread */
    printf("\nFinal values in main thread:\n");
    printf("  public_tls_var = %d\n", public_tls_var);
    printf("  weak_tls_var = %d\n", weak_tls_var);
    printf("  hidden_tls_var = %d\n", hidden_tls_var);
    printf("  protected_tls_var = %d\n", protected_tls_var);
    printf("  used_tls_var = %d\n", used_tls_var);
    printf("  imported_tls_var = %d\n", imported_tls_var);
    printf("  external_tls_var = %d\n", external_tls_var);
    printf("  common_tls_var = %d\n", common_tls_var);
    
    /* Final checksum */
    int final_checksum = public_tls_var + weak_tls_var + hidden_tls_var +
                        protected_tls_var + used_tls_var + imported_tls_var +
                        external_tls_var + common_tls_var;
    
    printf("\nFinal checksum: %d\n", final_checksum);
    
    return 0;
}
