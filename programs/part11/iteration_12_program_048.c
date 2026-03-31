#include <stdio.h>

// Declarations from other files
extern int helper_function(void);
extern int asm_helper(void);
extern void problematic_function(void);

int main(void) {
    printf("Main program starting\n");
    
    int result1 = helper_function();
    printf("Helper function returned: %d\n", result1);
    
    int result2 = asm_helper();
    printf("Assembly helper returned: %d\n", result2);
    
    // This will cause a link error if error.c is included
    // problematic_function();
    
    return 0;
}
