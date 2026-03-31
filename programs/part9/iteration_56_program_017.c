// Testing compiler's register allocator
int main() {
    // Warm up
    for (int i = 0; i < 1000; i++) {
        high_pressure_function(10);
    }
    
    // Actual test
    high_pressure_function(1000000);
    return 0;
}
