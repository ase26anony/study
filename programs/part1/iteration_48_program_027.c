int main() {
    int counter = 0;
    int a = 0, b = 0;
    
    for (int i = 0; i < 100; ++i) {
        if (i < 50) {
            goto target_label;
        }
        // Some other code
        continue;
    target_label:
        a = 1;  // Potential delay slot instruction
        counter += a;
    }
    
    return counter > 50 ? 0 : 1;
}
