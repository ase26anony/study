volatile int trigger = 1;

int main() {
    int counter = 0;
    int a = 0;
    
    for (int i = 0; i < 100; ++i) {
        if (trigger > i) {
            a = 1;  // Good candidate for delay slot
            counter += a;
        } else {
            // Some other work
            counter += 2;
        }
    }
    
    return counter > 50 ? 0 : 1;
}
