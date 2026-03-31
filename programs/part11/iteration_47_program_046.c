x₀ = 0
for (int i = 0; i < n; i++) {
    // Phi node needed here
    x₁ = φ(x₀, x₃)  // On first iteration: x₁ = x₀, on subsequent iterations: x₁ = x₃
    if (x₁ == 0) {
        x₂ = 1;
    } else {
        x₃ = 0;
    }
    // Another phi-like merge needed for the two branches
    x₄ = φ(x₂, x₃)  // Merges the two possible definitions of x
}
