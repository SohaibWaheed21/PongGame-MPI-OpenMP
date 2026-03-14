#pragma once

struct BallState {
    int id = 0;
    int type = 0; // 0=Common, 1=Rare, 2=Epic, 3=Legendary
    int pointsValue = 1;
    
    // Color as RGBA (0-255)
    unsigned char colorR = 255;
    unsigned char colorG = 255;
    unsigned char colorB = 0;
    unsigned char colorA = 255;
    
    // Physics state
    float x = 0.f;
    float y = 0.f;
    float vx = 45.f;
    float vy = 45.f;
    float radius = 10.f;
    float speedMultiplier = 1.0f;
    
    // Collision state
    int cooldownFrames = 0;
    
    // Helper methods
    void decrementCooldown() {
        if (cooldownFrames > 0) cooldownFrames--;
    }
    
    bool isInCooldown() const {
        return cooldownFrames > 0;
    }
    
    void resetCooldown(int frames) {
        cooldownFrames = frames;
    }
};
