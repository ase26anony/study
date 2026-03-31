int a, b, c;
int original_a = a;  // Store original if needed
if (a > 0) {
    b = 10;
    // Use a different variable instead of modifying 'a'
    int processed_value = 5;  
    // Or if you must update 'a', do it consciously
    a = 5;  // Only if this is intentional and documented
} else {
    b = 20;
}
