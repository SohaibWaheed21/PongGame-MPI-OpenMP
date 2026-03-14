#pragma once

enum class EventType {
    None,
    Hit,
    Score
};

struct HitEvent {
    int paddleId = 0; // 1 or 2
    int ballId = 0;
};

struct ScoreEvent {
    int playerId = 0; // 1 or 2
    int points = 0;
    int ballId = 0;
};

struct GameEvent {
    EventType type = EventType::None;
    
    // Just use separate fields instead of union to avoid constructor issues
    HitEvent hit;
    ScoreEvent score;
    
    // Default constructor
    GameEvent() = default;
    
    // Factory methods
    static GameEvent createHitEvent(int paddleId, int ballId) {
        GameEvent e;
        e.type = EventType::Hit;
        e.hit.paddleId = paddleId;
        e.hit.ballId = ballId;
        return e;
    }
    
    static GameEvent createScoreEvent(int playerId, int points, int ballId) {
        GameEvent e;
        e.type = EventType::Score;
        e.score.playerId = playerId;
        e.score.points = points;
        e.score.ballId = ballId;
        return e;
    }
};
