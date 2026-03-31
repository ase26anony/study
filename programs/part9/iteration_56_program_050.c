for (int outer = 0; outer < iterations; ++outer) {
    int x = outer;
    while (x > 0) {                    // Inner loop
        switch (x % 4) {               // Multiple basic blocks
            // ...
            case 3: if (x % 3 == 0) return; // Early return
        }
        x /= 2;
    }
}
