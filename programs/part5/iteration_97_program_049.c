/* Main driver that includes all complex headers */
#include "complex_types.h"
#include "macro_types.h"
#include "conditional_types.h"
#include "typedef_chains.h"

/* Minimal main function - compilation is the real test */
int main(void) {
    /* Just enough to use some types and satisfy linker */
    struct Outer o = {0};
    ComplexFuncPtr fp = NULL;
    
    /* Return trivial value */
    return 0;
}
