volatile int trigger = 1;
int main() {
    int counter = 0;
    int a = 0, b = 0;
    for (int i = 0; i < 100; ++i) {
        if (trigger > i) {
            // Branch to target_label - delay slot candidate follows
            goto target_label;
        }
        // Some other code that doesn't modify 'a'
        b = i * 2;
        
    target_label:
        a = 1;  // Good delay slot candidate
        counter += a;
    }
    return counter > 50 ? 0 : 1;
}
