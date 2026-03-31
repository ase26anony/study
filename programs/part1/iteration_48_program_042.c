volatile int trigger = 1;

int main() {
    int counter = 0;
    int a = 0, b = 0;
    
    for (int i = 0; i < 100; ++i) {
        // Force compiler to keep the conditional
        if (trigger > i) {
            // This branch could have a delay slot
            a = 1;  // Good candidate for delay slot
            goto target_label;
        }
        
        // Some work that uses 'a'
        a = i * 2;
        counter += a;
        continue;
        
    target_label:
        // Use the value set in what could be the delay slot
        counter += a;
    }
    
    return counter > 50 ? 0 : 1;
}
