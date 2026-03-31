volatile int trigger = 1;

int main() {
    int counter = 0;
    int a = 0, b = 0, c = 0;
    
    for (int i = 0; i < 100; ++i) {
        // Some computation that uses b and c
        b = i * 2;
        c = i + 5;
        
        if (trigger > i) {
            // Branch target
            a = 1;  // Good delay slot candidate
            counter += a;
        } else {
            // Fall-through path
            counter += b;
        }
    }
    
    return counter > 50 ? 0 : 1;
}
