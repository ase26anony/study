x₀ = 0
for (int i = 0; i < n; i++) {
    // Phi node needed here for x
    x₁ = φ(x₀, x₃)  // At loop entry: x₁ comes from x₀ (first iteration) or x₃ (subsequent iterations)
    if (x₁ == 0) {
        x₂ = 1;
    } else {
        x₃ = 0;
    }
    // Another phi-like merge needed for the value of x at loop end
    x₄ = φ(x₂, x₃)  // Merges the two possible definitions of x from the if/else branches
}
