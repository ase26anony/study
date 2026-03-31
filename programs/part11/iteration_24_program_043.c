/* Minimal test program to generate coverage data */
int main() {
    int x = 0;
    
    /* Simple conditional to generate some coverage data */
    if (x == 0) {
        x = 1;
    } else {
        x = 2;
    }
    
    /* Loop to generate more coverage data */
    for (int i = 0; i < 3; i++) {
        x += i;
    }
    
    return x;
}
