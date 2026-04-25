#pragma once
#include <SFML/Graphics.hpp>
#include "Map.h"
#include "Player.h"
#include "Camera.h"
#include "Renderer.h"
#include "Flashlight.h"
#include "EnergySystem.h"
#include "HUD.h"
#include "AudioManager.h"
#include "ParticleSystem.h"
#include "Battery.h"
#include "GameState.h"
#include "Minimap.h"
#include "HazardZoneSystem.h"
#include "Enemy.h"
#include "StatsTracker.h"
#include "PostProcess.h"
#include "PowerUpSystem.h"
#include "PauseMenu.h"

class Game {
    public:
        Game();

		// Starts game loop — wont return until the window is closed
        void run();

    private:
        // --- Window and shared resources ---
        sf::RenderWindow m_window;
        sf::Font         m_font;

        // --- Systems ---
        Map              m_map;
        Player           m_player;
        Camera           m_camera;
        Renderer         m_renderer;
        Flashlight       m_flashlight;
        EnergySystem     m_energy;
        HUD              m_hud;
        AudioManager     m_audio;
        ParticleSystem   m_particles;
        Minimap          m_minimap;
        StatsTracker     m_stats;
        PostProcess      m_postProcess;
        PowerUpSystem    m_powerUps;
        HazardZoneSystem m_hazards;
        PauseMenu        m_pauseMenu;

        // --- Entities ---
        std::vector<Battery> m_batteries;
        std::vector<Enemy>   m_enemies;

        // --- Game state ---
        sf::Vector2f m_signalPos;
        GameState    m_state;
        GameState    m_prevState;
        sf::Clock    m_clock;
        HazardType   m_currentHazard = HazardType::None;
        HazardType   m_prevHazard = HazardType::None;
        sf::Color    m_currentLightColor = sf::Color(255, 240, 200);

        // --- Volumes ---
        float m_masterVolume = 0.7f;
        float m_musicVolume = 0.4f;

        // --- Timers ---
        float m_wakingTimer = 0.f;
        float m_winTimer = 0.f;
        float m_slowMotionTimer = 0.f;
        float m_gameOverFadeTimer = 0.f;

        // --- Flags ---
        bool m_enemyAlertPlayed = false;

        // --- The three phases of the game loop ---
        void processInput();
        void update(float dt);
        void render();

        // --- Helpers ---
        void initMap();
        void reset();
        void spawnEnemies();

        // --- Update subsystems ---
        void updateMovement(float dt);
        void updateRoomEffects(float dt, sf::Vector2i playerTile);
        void updateHazardEffects(float dt, sf::Vector2i playerTile);
        void updateEnemies(float dt);
        void updateBatteries(float dt);
        void updatePowerUps(float dt);
        void updateEnemyProximity();
        void checkEndConditions();

        // --- Constants ---
        static constexpr int   MAP_W = 60;
        static constexpr int   MAP_H = 60;
        static constexpr float TILE_SIZE = 24.f;
        static constexpr float WAKING_DURATION = 2.5f;
        static constexpr float WIN_DURATION = 3.5f;
        static constexpr float SLOW_DURATION = 0.8f;
        static constexpr float SLOW_FACTOR = 0.12f;
        static constexpr float GAMEOVER_FADE_DURATION = 1.5f;
};