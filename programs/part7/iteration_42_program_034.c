#ifdef __GNUC__
#define TLS __thread
#else
#define TLS /* empty for non-GCC compilers */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External declarations from other files */
extern TLS int public_tls;
extern TLS int weak_tls;
extern TLS int external_tls;
extern TLS int common_tls;

/* Function prototypes from other files */
void init_file1_tls(int seed);
int compute_file1_sum(void);
void modify_file2_tls(int factor);
int get_file2_result(void);

/* Main file TLS variables with various attributes */
#ifdef __GNUC__
/* Public TLS variable */
TLS int public_main_tls;

/* Static (private linkage) TLS variable */
static TLS int static_main_tls;

/* Weak TLS variable */
TLS __attribute__((weak)) int weak_main_tls = 0;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
TLS __attribute__((dllimport)) int imported_tls;
#else
/* Simulate with regular TLS on non-Windows */
TLS int imported_tls;
#endif

/* Common symbol behavior (no initializer) */
TLS int common_main_tls;
#endif

/* Opaque function to prevent optimization */
void use_value(int value) {
    volatile int sink = value;
    (void)sink;
}

/* Function that takes address to ensure TREE_USED */
void take_addresses(void) {
#ifdef __GNUC__
    use_value((int)&public_main_tls);
    use_value((int)&static_main_tls);
    use_value((int)&weak_main_tls);
    use_value((int)&imported_tls);
    use_value((int)&common_main_tls);
#endif
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
#ifdef __GNUC__
    /* Initialize TLS variables with non-constant values */
    public_main_tls = rand() % 100;
    static_main_tls = argc + (rand() % 50);
    weak_main_tls = seed % 77;
    imported_tls = (seed * 3) % 99;
    common_main_tls = (argc * seed) % 88;
    
    /* Force usage to prevent optimization */
    take_addresses();
    
    /* Use the variables in computations */
    int sum = public_main_tls + static_main_tls + weak_main_tls + 
               imported_tls + common_main_tls;
    
    printf("Main file TLS sum: %d\n", sum);
#endif
    
    /* Initialize and use TLS from other files */
    init_file1_tls(seed);
    int file1_sum = compute_file1_sum();
    printf("File1 TLS sum: %d\n", file1_sum);
    
    modify_file2_tls(seed % 10 + 1);
    int file2_result = get_file2_result();
    printf("File2 result: %d\n", file2_result);
    
    /* Final checksum using all accessible TLS variables */
    int final_checksum = 0;
#ifdef __GNUC__
    final_checksum = (public_main_tls + static_main_tls + 
                      weak_main_tls + imported_tls + 
                      common_main_tls) % 256;
#endif
    
    /* Use external TLS variables */
    final_checksum = (final_checksum + file1_sum + file2_result) % 256;
    
    printf("Final checksum: 0x%02x\n", final_checksum);
    
    return final_checksum;
}
