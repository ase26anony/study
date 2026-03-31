volatile int trigger = 1;
int main() {
    int counter = 0;
    int a = 0, b = 0;
    
    for (int i = 0; i < 100; ++i) {
        // Read volatile variable once
        int t = trigger;
        
        if (t > i) {
            // This branch could have its delay slot filled
            a = 1;  // Good delay slot candidate
            counter += a;
        } else {
            // Some other code
            b = i * 2;
        }
    }
    
    return counter > 50 ? 0 : 1;
}
