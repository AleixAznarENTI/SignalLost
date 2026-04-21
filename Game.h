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
        Map            m_map;
        Player         m_player;
        Camera         m_camera;
        Renderer       m_renderer;
        Flashlight     m_flashlight;
        EnergySystem   m_energy;
        HUD            m_hud;
        AudioManager   m_audio;
        ParticleSystem m_particles;
        Minimap        m_minimap;

        // --- Game state ---
        std::vector<Battery> m_batteries;
        sf::Vector2f         m_signalPos;
        GameState            m_state;
        GameState            m_prevState;
        sf::Clock            m_clock;

        // --- The three phases of the game loop ---
        void processInput();
        void update(float dt);
        void render();

        // --- Helpers ---
        void initMap();
        void reset();
        void drawEndScreen();  // "Press R" text in victory/gameover

        void updateMovement(float dt);
        void updateRoomEffects(float dt);
        void updateBatteries();
        void checkEndConditions();

        static constexpr int   MAP_W = 60;
        static constexpr int   MAP_H = 60;
        static constexpr float TILE_SIZE = 24.f;
};




