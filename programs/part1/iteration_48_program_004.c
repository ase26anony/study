volatile int trigger = 1;

int main() {
    int counter = 0;
    int a = 0, b = 0;
    
    for (int i = 0; i < 100; ++i) {
        // Read volatile to force conditional evaluation
        int condition = trigger;
        
        if (condition > i) {
            // This goto becomes a conditional branch
            goto do_work;
        }
        
        // Fall-through path
        a = 0;  // Could potentially go in delay slot
        continue;
        
    do_work:
        // Work to be done when condition is true
        a = 1;  // Good delay slot candidate
        counter += a;
    }
    
    return counter > 50 ? 0 : 1;
}
