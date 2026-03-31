// Chain of dependencies
a = trigger; b = a + 1; c = b * 2; d = c - a; e = d / 3;

// Cross-type usage
fa = a * 0.5;  // Integer to float conversion

// Unpredictable modifications
switch(x % 4) {
    case 0: a += b;  // Can't predict which path
    // ...
}
