volatile int trigger = 1;

int main() {
    int counter = 0;
    int a = 0, b = 0, c = 0;
    
    // Simple conditional branch that could have its delay slot filled
    if (trigger > 0) {
        // This could be moved into the delay slot
        a = 1;
        counter += a;
    } else {
        b = 2;
        counter += b;
    }
    
    // Another example with a function call
    if (trigger < 10) {
        // These independent instructions could fill delay slots
        c = trigger * 2;
        a = c + 1;
        counter += a;
    }
    
    return counter > 5 ? 0 : 1;
}
