#include <stdio.h>

// Declarations from other files
extern int helper_function(void);
extern int another_helper(void);
extern void assembly_function(void);

int main() {
    printf("Main program starting\n");
    
    int result1 = helper_function();
    printf("Helper function returned: %d\n", result1);
    
    int result2 = another_helper();
    printf("Another helper returned: %d\n", result2);
    
    assembly_function();
    
    return 0;
}
