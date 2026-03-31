#include <stdio.h>

extern int helper(void);
extern int problematic(void);

int main(void) {
    int result = helper();
    printf("Helper returned: %d\n", result);
    
    /* This will fail to link if problematic.o is included */
    /* int bad = problematic(); */
    
    return 0;
}
