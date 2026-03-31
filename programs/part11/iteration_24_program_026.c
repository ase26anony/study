/* Simple test program to generate coverage data */
int main() {
    int x = 0;
    
    /* Some basic code to generate coverage arcs */
    for (int i = 0; i < 10; i++) {
        x += i;
    }
    
    if (x > 0) {
        x = x * 2;
    }
    
    return x > 0 ? 0 : 1;
}
