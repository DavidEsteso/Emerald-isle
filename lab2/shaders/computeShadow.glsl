float computeShadow(sampler2D shadowMap, vec3 projectedCoords) {

    // Get closest depth value from light's perspective
    float closestDepthFromLight = texture(shadowMap, projectedCoords.xy).r;

    // Get depth of current fragment from light's perspective
    float currentFragmentDepth = projectedCoords.z;

    // Check whether current fragment position is in shadow
    float shadowFactor = (projectedCoords.z >= 0 && currentFragmentDepth >= closestDepthFromLight + 1e-3) ? 0.2 : 1.0;

    return shadowFactor;
}
