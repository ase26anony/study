#include <stdio.h>

extern int helper(void);
extern int problematic(void);

int main(void) {
    printf("Helper returned: %d\n", helper());
    
    /* This will fail to link if problematic() is undefined */
    printf("Problematic: %d\n", problematic());
    
    return 0;
}
