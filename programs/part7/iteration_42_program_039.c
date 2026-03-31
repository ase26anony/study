#ifdef __GNUC__
#define TLS __thread
#else
#define TLS /* empty if not GCC */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Declarations for TLS variables from other files */
#ifdef __GNUC__
extern TLS int public_tls;
extern TLS int weak_tls;
extern TLS int imported_tls;
extern TLS int external_tls;
#endif

/* Function prototypes from other files */
void init_tls_vars(int seed);
int compute_tls_sum(void);
void use_common_tls(int val);

/* Local TLS variables with different attributes */
#ifdef __GNUC__
/* Public TLS variable */
TLS int main_public_tls;

/* Static (private linkage) TLS variable */
static TLS int main_static_tls;

/* Weak TLS variable */
TLS __attribute__((weak)) int main_weak_tls = 0;

/* External declaration (will be defined later in this file) */
extern TLS int main_external_tls;

/* Definition of the external TLS variable */
TLS int main_external_tls = 0;

/* DLL import attribute simulation (for Windows/MinGW) */
#ifdef _WIN32
TLS __attribute__((dllimport)) int main_dllimport_tls;
#else
/* On non-Windows, just a regular TLS variable */
TLS int main_dllimport_tls;
#endif
#endif

/* Opaque function to prevent optimization */
void use_value(int val) {
    volatile int sink = val;
    (void)sink;
}

void use_pointer(void* ptr) {
    volatile void* sink = ptr;
    (void)sink;
}

int main(int argc, char** argv) {
    srand(time(NULL));
    
    /* Initialize with non-constant values */
    int base_value = argc > 1 ? atoi(argv[1]) : rand();
    
#ifdef __GNUC__
    /* Initialize all TLS variables with non-constant values */
    main_public_tls = base_value + 1;
    main_static_tls = base_value + 2;
    main_weak_tls = base_value + 3;
    main_external_tls = base_value + 4;
    main_dllimport_tls = base_value + 5;
    
    /* Use the variables to ensure TREE_USED is set */
    use_value(main_public_tls);
    use_value(main_static_tls);
    use_value(main_weak_tls);
    use_value(main_external_tls);
    use_value(main_dllimport_tls);
    
    /* Take addresses to ensure variables are fully referenced */
    use_pointer(&main_public_tls);
    use_pointer(&main_static_tls);
    use_pointer(&main_weak_tls);
    use_pointer(&main_external_tls);
    use_pointer(&main_dllimport_tls);
#endif
    
    /* Initialize TLS variables in other compilation units */
    init_tls_vars(base_value + 100);
    
    /* Use common TLS variables */
    use_common_tls(base_value + 200);
    
    /* Compute checksum from all TLS variables */
    int checksum = 0;
    
#ifdef __GNUC__
    checksum += main_public_tls;
    checksum += main_static_tls;
    checksum += main_weak_tls;
    checksum += main_external_tls;
    checksum += main_dllimport_tls;
#endif
    
    checksum += compute_tls_sum();
    
    printf("TLS checksum: %d\n", checksum % 1000);
    
    return 0;
}
