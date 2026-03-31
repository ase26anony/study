volatile int trigger = 1;

int main() {
    int counter = 0;
    int a = 0, b = 0;
    
    for (int i = 0; i < 100; ++i) {
        // This comparison and branch could have a delay slot
        if (trigger > i) {
            // Simple instruction that could go in delay slot
            a = i + 1;  // Candidate for delay slot filling
            counter += a;
        } else {
            b = i * 2;
            counter -= b;
        }
    }
    
    return counter > 50 ? 0 : 1;
}
