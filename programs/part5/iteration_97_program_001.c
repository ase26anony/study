/* Main driver that includes all complex headers */
#include "complex_types.h"
#include "macro_types.h"
#include "conditional_types.h"
#include "typedef_chains.h"

/* Minimal main function - just enough to compile */
int main(void) {
    /* Dummy usage to avoid unused variable warnings */
    struct S s = {0};
    complex_func_ptr_t fp = NULL;
    factory_fn f = NULL;
    
    return 0;
}
