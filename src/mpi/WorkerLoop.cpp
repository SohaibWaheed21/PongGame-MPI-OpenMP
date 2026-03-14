#include "WorkerLoop.h"
#include "Messages.h"
#include "../sim/Simulation.h"
#include "../sim/Events.h"
#include "MpiCompat.h"
#include <array>
#include <iostream>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif

WorkerLoop::WorkerLoop() {
    initBalls();
}

void WorkerLoop::initBalls() {
    float startX = Constants::GAME_WIDTH / 2.f;
    float startY = Constants::GAME_HEIGHT / 4.f; // Arbitrary initial Y
    
    for (int i = 0; i < Constants::NUM_BALLS; ++i) {
        Simulation::initializeBallState(m_ballStates[i], i, i % 4, startX, startY, Constants::BALL_RADIUS);
    }
}

void WorkerLoop::receiveTickInput() {
    TickInput input;
    MPI_Recv(&input, sizeof(TickInput), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    m_paddle1 = fromMessage(input.paddle1);
    m_paddle2 = fromMessage(input.paddle2);
    m_gameWidth = input.gameWidth;
    m_gameHeight = input.gameHeight;
    m_dt = input.dt;
    m_running = (input.running != 0);
    
    // Handle respawns
    for (int i = 0; i < Constants::NUM_BALLS; ++i) {
        if (input.respawnFlags[i] != 0) {
            Simulation::respawnBall(m_ballStates[i], input.respawnX[i], input.respawnY[i]);
        }
    }
}

void WorkerLoop::sendTickOutput() {
    TickOutput output;
    
    // Copy ball states
    for (int i = 0; i < Constants::NUM_BALLS; ++i) {
        output.balls[i] = toMessage(m_ballStates[i]);
    }
    
    output.eventCount = 0;
    
    MPI_Send(&output, sizeof(TickOutput), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
}

void WorkerLoop::simulateTick() {
    TickOutput output;

    output.eventCount = 0;

#ifdef _OPENMP
    std::vector<std::vector<PackedEvent>> threadLocalEvents(static_cast<std::size_t>(omp_get_max_threads()));

#pragma omp parallel
    {
        const int threadId = omp_get_thread_num();
        auto& localEvents = threadLocalEvents[static_cast<std::size_t>(threadId)];

#pragma omp for
        for (int i = 0; i < Constants::NUM_BALLS; ++i) {
            BallState ball = m_ballStates[i];
            auto events = Simulation::simulateBallTick(
                ball,
                m_paddle1,
                m_paddle2,
                m_gameWidth,
                m_gameHeight,
                m_dt
            );

            m_ballStates[i] = ball;
            output.balls[i] = toMessage(ball);

            for (const auto& event : events) {
                PackedEvent packed;
                if (event.type == EventType::Hit) {
                    packed.type = 1;
                    packed.data1 = event.hit.paddleId;
                    packed.data2 = event.hit.ballId;
                }
                else if (event.type == EventType::Score) {
                    packed.type = 2;
                    packed.data1 = event.score.playerId;
                    packed.data2 = event.score.ballId;
                    packed.data3 = event.score.points;
                }
                if (packed.type != 0) {
                    localEvents.push_back(packed);
                }
            }
        }
    }

    for (const auto& localEvents : threadLocalEvents) {
        for (const auto& packed : localEvents) {
            if (output.eventCount >= TickOutput::MAX_EVENTS) break;
            const int idx = output.eventCount;
            output.eventTypes[idx] = packed.type;
            output.eventData1[idx] = packed.data1;
            output.eventData2[idx] = packed.data2;
            output.eventData3[idx] = packed.data3;
            output.eventCount++;
        }
        if (output.eventCount >= TickOutput::MAX_EVENTS) break;
    }
#else
    for (int i = 0; i < Constants::NUM_BALLS; ++i) {
        BallState ball = m_ballStates[i];
        auto events = Simulation::simulateBallTick(
            ball,
            m_paddle1,
            m_paddle2,
            m_gameWidth,
            m_gameHeight,
            m_dt
        );

        m_ballStates[i] = ball;
        output.balls[i] = toMessage(ball);

        for (const auto& event : events) {
            if (output.eventCount >= TickOutput::MAX_EVENTS) break;

            const int idx = output.eventCount;
            if (event.type == EventType::Hit) {
                output.eventTypes[idx] = 1;
                output.eventData1[idx] = event.hit.paddleId;
                output.eventData2[idx] = event.hit.ballId;
                output.eventData3[idx] = 0;
                output.eventCount++;
            }
            else if (event.type == EventType::Score) {
                output.eventTypes[idx] = 2;
                output.eventData1[idx] = event.score.playerId;
                output.eventData2[idx] = event.score.ballId;
                output.eventData3[idx] = event.score.points;
                output.eventCount++;
            }
        }
    }
#endif

    ++m_tickNumber;
    if (Constants::DEBUG_LOGGING && (m_tickNumber % Constants::DEBUG_LOG_EVERY_N_TICKS == 0)) {
#ifdef _OPENMP
        std::cout << "[rank 1] tick=" << m_tickNumber << " events=" << output.eventCount
                  << " threads=" << omp_get_max_threads() << "\n";
#else
        std::cout << "[rank 1] tick=" << m_tickNumber << " events=" << output.eventCount << " threads=1\n";
#endif
    }

    // Send output to master
    MPI_Send(&output, sizeof(TickOutput), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
}

void WorkerLoop::run() {
    while (true) {
        receiveTickInput();
        
        if (!m_running) {
            // Master signaled shutdown
            break;
        }
        
        simulateTick();
    }
}
