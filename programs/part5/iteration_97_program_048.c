/* Main file that includes all complex headers */
#include "complex_types.h"
#include "macro_types.h"
#include "conditional_types.h"
#include "typedef_chains.h"

/* Minimal main function - just enough to compile */
int main(void) {
    /* Use some types to avoid unused warnings */
    struct ComplexStruct cs;
    cs.nested.arr[0][0] = 1;
    
    FunctionPtr fp = &simple_func;
    fp(42);
    
    return 0;
}

/* Simple function for function pointer testing */
void simple_func(int x) {
    (void)x;
}
