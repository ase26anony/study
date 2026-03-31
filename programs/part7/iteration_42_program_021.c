/* tls_defs.c - Contains TLS variable definitions with diverse attributes */

#ifdef __GNUC__

#include <stdio.h>
#include <stdlib.h>

/* Public TLS variable with initialization */
__thread int public_tls = 0;

/* Static (private linkage) TLS variable */
static __thread int static_tls = 0;

/* Weak TLS variable - can be overridden by strong definition elsewhere */
__thread __attribute__((weak)) int weak_tls = 0;

/* Common symbol behavior (when compiled with -fcommon) */
__thread int common_tls;  /* No initializer for common symbol behavior */

/* Function to initialize and use TLS variables */
void init_and_use_tls(int base_value) {
    /* Initialize with non-constant values to prevent optimization */
    public_tls = base_value + 1;
    static_tls = base_value + 2;
    weak_tls = base_value + 3;
    common_tls = base_value + 4;
    
    /* Use the variables in non-trivial ways */
    public_tls *= 2;
    static_tls += public_tls;
    weak_tls = static_tls - weak_tls;
    common_tls = (common_tls * 3) % 100;
    
    /* Take addresses to ensure they're marked as used */
    printf("Addresses in tls_defs: public=%p, static=%p, weak=%p, common=%p\n",
           (void*)&public_tls, (void*)&static_tls, 
           (void*)&weak_tls, (void*)&common_tls);
}

/* Function that returns a checksum of TLS values */
int get_tls_checksum(void) {
    return (public_tls + static_tls + weak_tls + common_tls) % 256;
}

/* Another public function that uses TLS */
void modify_tls_values(int multiplier) {
    public_tls *= multiplier;
    static_tls += multiplier;
    weak_tls -= multiplier;
    common_tls = (common_tls * multiplier) % 1000;
}

#endif /* __GNUC__ */
