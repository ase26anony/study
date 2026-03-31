#include <stdio.h>

// Declarations from other files
extern int helper_function(void);
extern int another_helper(void);
extern void assembly_function(void);
extern int problematic_function(void);

int main(void) {
    printf("Main program starting\n");
    
    int result1 = helper_function();
    printf("Helper returned: %d\n", result1);
    
    int result2 = another_helper();
    printf("Another helper returned: %d\n", result2);
    
    assembly_function();
    
    // This will cause linking to fail if problematic_function is undefined
    int problem = problematic_function();
    printf("Problematic function returned: %d\n", problem);
    
    return 0;
}
