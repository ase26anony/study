volatile int trigger = 1;

int main() {
    int counter = 0;
    int a = 0, b = 0;
    
    for (int i = 0; i < 100; ++i) {
        // Simple conditional that will compile to a branch
        if (trigger > i) {
            a = 1;  // Good candidate for delay slot
            goto update_counter;
        }
        
        // Some other work
        b = i * 2;
        continue;
        
    update_counter:
        counter += a;
    }
    
    return counter > 50 ? 0 : 1;
}
