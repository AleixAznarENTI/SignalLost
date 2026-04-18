#include "Game.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <stdexcept>

// ----------------------------------------------------------------
// Constructor: initialize all systems, load resources, generate the map and position the player
// ----------------------------------------------------------------
Game::Game()
    : m_window(sf::VideoMode({ 800u, 600u }), "Signal Lost")
    , m_map(MAP_W, MAP_H)
    , m_player({ 0.f, 0.f }, TILE_SIZE)   // posición provisional, reset() la fija
    , m_camera(m_window, TILE_SIZE)
    , m_renderer(m_window, TILE_SIZE)
    , m_flashlight(m_window.getSize(), 160.f)
    , m_energy(100.f, 5.f)
    , m_hud(m_window, m_font)
    , m_particles(200)
    , m_state(GameState::Playing)
    , m_prevState(GameState::Playing)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    m_window.setFramerateLimit(60);

    if (!m_font.openFromFile("assets/font.ttf"))
        throw std::runtime_error("Unable to load assets/font.ttf");

    if (!m_audio.loadAll("assets/"))
        throw std::runtime_error("Unable to load audio files");

    reset();             // genera el mapa y posiciona al jugador
    m_audio.playAmbient();
}

// ----------------------------------------------------------------
// run: the main game loop — input → update → render
// ----------------------------------------------------------------
void Game::run() {
    while (m_window.isOpen()) {
        float dt = m_clock.restart().asSeconds();

		// Big dt protects against physics glitches and particles flying away, but also against audio desync and other time-based issues.
        if (dt > 0.1f) dt = 0.1f;

        processInput();
        update(dt);
        render();
    }
}

// ----------------------------------------------------------------
// processInput: eventos y teclado
// ----------------------------------------------------------------
void Game::processInput() {
    while (const auto event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            m_window.close();

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::R &&
                m_state != GameState::Playing) {
                reset();
            }
            if (key->code == sf::Keyboard::Key::Escape)
                m_window.close();
        }
    }
}

// ----------------------------------------------------------------
// update: game logic
// ----------------------------------------------------------------
void Game::update(float dt) {
    if (m_state != GameState::Playing) return;

    // --- Movement ---
    sf::Vector2f velocity(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) velocity.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) velocity.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) velocity.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) velocity.x += 1.f;

    m_player.move(velocity, dt, m_map);


    // --- Systems ---
    m_energy.update(dt);
    m_audio.update(m_energy.getPercentage());
    m_particles.emit(m_player.getPosition(), dt);
    m_particles.update(dt);
    m_camera.follow(m_player.getPosition());

    sf::Vector2i playerTile(
        static_cast<int>(m_player.getPosition().x / TILE_SIZE),
        static_cast<int>(m_player.getPosition().y / TILE_SIZE)
    );

    RoomType currentRoom = m_map.getRoomTypeAt(playerTile.x, playerTile.y);

    if (currentRoom == RoomType::Danger) 
		m_energy.applyPenalty(15.f * dt);

    float flickerRadius = 160.f * (0.5f + 0.5f * m_energy.getPercentage());

    if (currentRoom == RoomType::Danger) {
        float dangerFlicker = 1.f + 0.15f * std::sin(
			m_clock.getElapsedTime().asSeconds() * 15.f
        );
        m_flashlight.setRadius(flickerRadius * dangerFlicker);
    }
    // Radius of the flashlight reduces with low energy
    m_flashlight.setRadius(flickerRadius);
    m_flashlight.update(m_player.getPosition(), m_camera.getView(), m_window);

    // --- Batteries ---
    for (auto& battery : m_batteries) {
        if (battery.isCollected()) continue;

        sf::Vector2f diff = m_player.getPosition() - battery.getPosition();
		float distSq = diff.x * diff.x + diff.y * diff.y;
		float thresholdSq = (TILE_SIZE * 0.8f) * (TILE_SIZE * 0.8f);
        if (distSq < thresholdSq) {
            battery.collect();
            m_energy.restore(battery.getRestoreAmount());
            m_audio.playBatteryPickup();
        }
    }

    // --- End conditions ---
    sf::Vector2f diff = m_player.getPosition() - m_signalPos;
	float distSigSq = diff.x * diff.x + diff.y * diff.y;
	float sigThreshSq = (TILE_SIZE * 1.2f) * (TILE_SIZE * 1.2f);
    
    if (distSigSq < sigThreshSq)  m_state = GameState::Victory;
    if (m_energy.isDepleted())        m_state = GameState::GameOver;

    // Reaction to state change (triggers only once)
    if (m_state != m_prevState) {
        if (m_state == GameState::Victory)  m_audio.playSignalFound();
        if (m_state == GameState::GameOver) m_audio.playGameOver();
        m_prevState = m_state;
    }
}

// ----------------------------------------------------------------
// render: draw everything
// ----------------------------------------------------------------
void Game::render() {
    m_window.clear(sf::Color(10, 10, 20));
    m_window.setView(m_camera.getView());

    m_renderer.drawMap();
    m_renderer.drawBatteries(m_batteries);
    m_renderer.drawSignal(m_signalPos);
    m_renderer.drawPlayer(m_player.getPosition());
    m_particles.draw(m_window);

	m_flashlight.draw(m_window);        // always the last thing in the world layer, so it appears on top of everything else
    m_hud.draw(m_energy.getPercentage(), m_state);

    if (m_state != GameState::Playing)
        drawEndScreen();

    m_window.display();
}

// ----------------------------------------------------------------
// reset: restarts the game by regenerating the map, repositioning the player, resetting energy and batteries, and playing ambient music
// ----------------------------------------------------------------
void Game::reset() {
    m_audio.stopAll();

    m_map.generate(3000);
	m_renderer.bakeMap(m_map);

    m_player = Player(sf::Vector2f(m_map.getStartPosition()) * TILE_SIZE, TILE_SIZE);
    m_energy = EnergySystem(100.f, 5.f);
    m_batteries = m_map.getBatteries();
    m_signalPos = sf::Vector2f(m_map.getSignalPosition()) * TILE_SIZE;
    m_state = GameState::Playing;
    m_prevState = GameState::Playing;

    m_audio.playAmbient();
}

// ----------------------------------------------------------------
// drawEndScreen: restart text when the game is over or won, drawn on top of the default view so it doesn't move with the camera
// ----------------------------------------------------------------
void Game::drawEndScreen() {
    sf::View prev = m_window.getView();
    m_window.setView(m_window.getDefaultView());

    sf::Text hint(m_font, "Press R to restart  |  Escape to quit", 16);
    hint.setFillColor(sf::Color(180, 180, 180));

    sf::FloatRect b = hint.getLocalBounds();
    hint.setOrigin({ b.size.x / 2.f, 0.f });
    hint.setPosition({
        m_window.getSize().x / 2.f,
        m_window.getSize().y / 2.f + 40.f
        });

    m_window.draw(hint);
    m_window.setView(prev);
}