/* Minimal test program to generate coverage data */
int main(void) {
    int x = 1;
    int y = 2;
    int z = x + y;
    
    if (z > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Failure */
    }
}
