volatile int trigger = 1;
int main() {
    int counter = 0;
    int a = 0, b = 0;
    
    // Move volatile read outside loop to reduce barriers
    int local_trigger = trigger;
    
    for (int i = 0; i < 100; ++i) {
        if (local_trigger > i) {
            // Simple assignment that could go in delay slot
            a = 1;
            counter += a;
        } else {
            // Some other code
            // Empty or minimal code here
        }
    }
    return counter > 50 ? 0 : 1;
}
