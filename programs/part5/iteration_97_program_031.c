/* Main driver that includes all complex headers */
#include "complex_types.h"
#include "macro_types.h"
#include "conditional_types.h"
#include "typedef_chains.h"

/* Minimal main function - compilation is the real test */
int main(void) {
    /* Just enough to use some types and satisfy linker */
    struct Outer outer;
    outer.nested.arr[0][0] = 1;
    
    return 0;
}
