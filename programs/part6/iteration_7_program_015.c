int a, b, c;
int original_a = a;  // Store original if needed
if (a > 0) {
    b = 10;
    // Use a different variable if you need to store 5
    c = 5;  
} else {
    b = 20;
}
// If you really need a = 5, do it after the conditional logic
// a = (a > 0) ? 5 : a;
