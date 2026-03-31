// Simplified execution:
counter = 0
For i = 0 to 99:
    if (1 > i) {  // Only true when i = 0
        a = 1;
        counter += a;  // counter becomes 1
    }
    // For i = 1..99: just continue
return (counter > 50) ? 0 : 1;  // counter = 1, so returns 1
