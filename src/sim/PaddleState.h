#pragma once

struct PaddleState {
    float x = 0.f;
    float y = 0.f;
    float w = 25.f;
    float h = 100.f;
    
    // Helper methods
    float left() const { return x; }
    float right() const { return x + w; }
    float top() const { return y; }
    float bottom() const { return y + h; }
    
    bool intersects(float bx, float by, float br) const {
        // Check if circle (ball) intersects rectangle (paddle)
        float closestX = (bx < x) ? x : ((bx > x + w) ? x + w : bx);
        float closestY = (by < y) ? y : ((by > y + h) ? y + h : by);
        
        float distX = bx - closestX;
        float distY = by - closestY;
        
        return (distX * distX + distY * distY) < (br * br);
    }
};
