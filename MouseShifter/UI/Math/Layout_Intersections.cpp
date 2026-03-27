struct Intersection
{
    int x, y;   // center of diamond
    int radius; // distance from center to diamond corners
};
std::vector<Intersection> intersections;

void ComputeIntersections()
{
    intersections.clear();
    int railCount = is16GearSet ? 5 : 4;

    for (int i = 0; i < railCount; ++i)
    {
        Intersection inter;
        inter.x = railX[i].x;
        inter.y = centerY;
        inter.radius = int(baseIntersectionRadius * diagonalAssist); // scaled by diagonalAssist
        intersections.push_back(inter);
    }

    // Update vertical threshold too
    enterVerticalThreshold = int(baseEnterVerticalThreshold * diagonalAssist);
}

// --- Determine if inside diamond intersection with diagonal assist ---
bool IsInsideIntersection(int x, int y, double diagonalAssist = 1.0)
{
    for (auto& inter : intersections)
    {
        int dx = x - inter.x;
        int dy = y - inter.y;

        // --- Detect if vertical rail (top/bottom gear exists) ---
        bool hasTop = false;
        bool hasBottom = false;
        for (auto& kv : lowerGearPositions)
        {
            if (kv.second.x == inter.x)
            {
                if (kv.second.y == topY) hasTop = true;
                if (kv.second.y == bottomY) hasBottom = true;
            }
        }

        double radiusX = inter.radius * diagonalAssist;

        // Asymmetric vertical radii with hard cutoff
        double radiusY_top = 0.0;
        double radiusY_bottom = 0.0;

        if (hasTop && hasBottom)
        {
            radiusY_top = radiusY_bottom = inter.radius * diagonalAssist * 1.5;
        }
        else if (hasTop)
        {
            radiusY_top = inter.radius * diagonalAssist * 1.5;
            radiusY_bottom = inter.radius * diagonalAssist * 0.4; // small buffer zone
        }
        else if (hasBottom)
        {
            radiusY_top = inter.radius * diagonalAssist * 0.4; // small buffer zone
            radiusY_bottom = inter.radius * diagonalAssist * 1.5;
        }

        else
        {
            radiusY_top = radiusY_bottom = 0.0; // no gear, no intersection
        }

        // If dy is in the “empty” direction, skip this intersection
        if ((y < inter.y && radiusY_top == 0.0) || (y > inter.y && radiusY_bottom == 0.0))
            continue;

        // Normalize dy asymmetrically
        double normalizedDY = (dy < 0) ? -dy / radiusY_top : dy / radiusY_bottom;
        double normalizedDX = dx / radiusX;

        double distance = sqrt(normalizedDX * normalizedDX + normalizedDY * normalizedDY);

        if (distance <= 1.0)
            return true;
    }

    return false;
}