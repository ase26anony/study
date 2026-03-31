x_initial = 0
for i = 0 to N-1:
    if i % 2 == 0:
        x_even = 1
    else:
        x_odd = 0
    x_phi = φ(x_even, x_odd)  // This is the phi node
    if x_phi == 0:  // Can't optimize - x_phi isn't constant
        // ...
