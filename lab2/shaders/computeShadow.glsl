float computeShadow(sampler2D shadowMap, vec3 projectedCoords) {

    float closestDepthFromLight = texture(shadowMap, projectedCoords.xy).r;

    float currentFragmentDepth = projectedCoords.z;

    float shadowFactor = (projectedCoords.z >= 0 && currentFragmentDepth >= closestDepthFromLight + 1e-3) ? 0.2 : 1.0;

    return shadowFactor;
}
