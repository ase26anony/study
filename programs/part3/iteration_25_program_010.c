x₀ = 0
for i = 0; i < N; i++:
    x₁ = φ(x₀, x₃)  // Phi node merging values from previous iteration
    if i % 2 == 0:
        x₂ = 1
    else:
        x₃ = 0
    if x₁ == 0:  // Can't determine statically - depends on phi node
        // ...
