#include <stdio.h>

extern int helper(void);
extern int problematic(void);

int main(void) {
    int result = helper();
    printf("Helper returned: %d\n", result);
    
    /* This will cause a link error if problematic() is undefined */
    /* int problem = problematic(); */
    
    return 0;
}
