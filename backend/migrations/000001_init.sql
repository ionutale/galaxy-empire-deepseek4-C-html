CREATE TABLE IF NOT EXISTS users (
    id BIGSERIAL PRIMARY KEY,
    username VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    server_id INT NOT NULL DEFAULT 1,
    power_rank_score BIGINT DEFAULT 0,
    dark_matter NUMERIC(24,4) DEFAULT 0,
    credits NUMERIC(24,4) DEFAULT 0,
    home_planet_id BIGINT DEFAULT 0,
    last_daily_claim TIMESTAMP WITH TIME ZONE,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS coordinates (
    id BIGSERIAL PRIMARY KEY,
    server_id INT NOT NULL DEFAULT 1,
    galaxy_num INT NOT NULL,
    system_num INT NOT NULL,
    position_num INT NOT NULL,
    is_moon BOOLEAN DEFAULT FALSE,
    UNIQUE(server_id, galaxy_num, system_num, position_num, is_moon)
);

CREATE TABLE IF NOT EXISTS planets (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT REFERENCES users(id) ON DELETE SET NULL,
    coordinate_id BIGINT REFERENCES coordinates(id),
    is_moon BOOLEAN DEFAULT FALSE,
    planet_size INT DEFAULT 178,
    temperature INT DEFAULT 20,
    metal NUMERIC(24,4) DEFAULT 500,
    crystal NUMERIC(24,4) DEFAULT 500,
    gas NUMERIC(24,4) DEFAULT 500,
    energy NUMERIC(24,4) DEFAULT 100,
    last_updated TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    last_resource_update TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    metal_mine_level INT DEFAULT 1,
    crystal_mine_level INT DEFAULT 1,
    gas_mine_level INT DEFAULT 1,
    solar_plant_level INT DEFAULT 1,
    fusion_reactor_level INT DEFAULT 0,
    robotics_level INT DEFAULT 1,
    nanite_factory_level INT DEFAULT 0,
    shipyard_level INT DEFAULT 1,
    research_lab_level INT DEFAULT 1,
    radar_level INT DEFAULT 0,
    alliance_depot_level INT DEFAULT 0,
    metal_storage_level INT DEFAULT 1,
    crystal_storage_level INT DEFAULT 1,
    gas_storage_level INT DEFAULT 1,
    moon_base_level INT DEFAULT 0,
    robotics_moon_level INT DEFAULT 0,
    shipyard_moon_level INT DEFAULT 0,
    pioneer_lab_level INT DEFAULT 0,
    eu_x_radar_level INT DEFAULT 0,
    wormhole_generator_level INT DEFAULT 0,
    galactonite_research_center_level INT DEFAULT 0,
    const_building VARCHAR(50),
    const_target_level INT,
    construction_end TIMESTAMP WITH TIME ZONE,
    prod_unit_type VARCHAR(50),
    prod_quantity INT,
    production_end TIMESTAMP WITH TIME ZONE
);

CREATE TABLE IF NOT EXISTS research (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT REFERENCES users(id) ON DELETE CASCADE,
    energy_tech INT DEFAULT 0,
    laser_tech INT DEFAULT 0,
    ion_tech INT DEFAULT 0,
    hyperspace_tech INT DEFAULT 0,
    plasma_tech INT DEFAULT 0,
    ultra_temperature_tech INT DEFAULT 0,
    anti_gravity_tech INT DEFAULT 0,
    weapons_tech INT DEFAULT 0,
    shielding_tech INT DEFAULT 0,
    strength_tech INT DEFAULT 0,
    combustion_drive INT DEFAULT 0,
    impulse_drive INT DEFAULT 0,
    hyperspace_drive INT DEFAULT 0,
    astrophysics_tech INT DEFAULT 0,
    computer_tech INT DEFAULT 0,
    espionage_tech INT DEFAULT 0,
    queued_tech VARCHAR(50),
    research_start TIMESTAMP WITH TIME ZONE,
    research_end TIMESTAMP WITH TIME ZONE,
    UNIQUE(user_id)
);

CREATE TABLE IF NOT EXISTS fleet_missions (
    id BIGSERIAL PRIMARY KEY,
    owner_id BIGINT REFERENCES users(id),
    origin_planet_id BIGINT REFERENCES planets(id),
    target_galaxy INT NOT NULL DEFAULT 0,
    target_system INT NOT NULL DEFAULT 0,
    target_position INT NOT NULL DEFAULT 0,
    target_is_moon BOOLEAN DEFAULT FALSE,
    mission_type INT DEFAULT 1,
    departure_time TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    arrival_time TIMESTAMP WITH TIME ZONE,
    return_time TIMESTAMP WITH TIME ZONE,
    light_fighter INT DEFAULT 0,
    heavy_fighter INT DEFAULT 0,
    cruiser INT DEFAULT 0,
    battleship INT DEFAULT 0,
    dreadnought INT DEFAULT 0,
    destroyer INT DEFAULT 0,
    bomber INT DEFAULT 0,
    interceptor INT DEFAULT 0,
    iron_behemoth INT DEFAULT 0,
    devourer_interceptor INT DEFAULT 0,
    titanenergetic_frigate INT DEFAULT 0,
    annihilator INT DEFAULT 0,
    small_cargo INT DEFAULT 0,
    large_cargo INT DEFAULT 0,
    recycler INT DEFAULT 0,
    colony_ship INT DEFAULT 0,
    probe INT DEFAULT 0,
    solar_satellite INT DEFAULT 0,
    metal NUMERIC(24,4) DEFAULT 0,
    crystal NUMERIC(24,4) DEFAULT 0,
    gas NUMERIC(24,4) DEFAULT 0,
    recalled BOOLEAN DEFAULT FALSE,
    arrival_processed BOOLEAN DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS fleet_inventories (
    id BIGSERIAL PRIMARY KEY,
    planet_id BIGINT REFERENCES planets(id) UNIQUE,
    light_fighter INT DEFAULT 0,
    heavy_fighter INT DEFAULT 0,
    cruiser INT DEFAULT 0,
    battleship INT DEFAULT 0,
    dreadnought INT DEFAULT 0,
    destroyer INT DEFAULT 0,
    bomber INT DEFAULT 0,
    interceptor INT DEFAULT 0,
    iron_behemoth INT DEFAULT 0,
    devourer_interceptor INT DEFAULT 0,
    titanenergetic_frigate INT DEFAULT 0,
    annihilator INT DEFAULT 0,
    small_cargo INT DEFAULT 0,
    large_cargo INT DEFAULT 0,
    recycler INT DEFAULT 0,
    colony_ship INT DEFAULT 0,
    probe INT DEFAULT 0,
    solar_satellite INT DEFAULT 0
);

CREATE TABLE IF NOT EXISTS defense_inventories (
    id BIGSERIAL PRIMARY KEY,
    planet_id BIGINT REFERENCES planets(id) UNIQUE,
    rocket_launcher INT DEFAULT 0,
    light_laser INT DEFAULT 0,
    heavy_laser INT DEFAULT 0,
    ion_cannon INT DEFAULT 0,
    mk2_cannon INT DEFAULT 0,
    plasma_cannon INT DEFAULT 0,
    proton_cannon INT DEFAULT 0,
    small_shield_dome INT DEFAULT 0,
    large_shield_dome INT DEFAULT 0
);

CREATE TABLE IF NOT EXISTS commanders (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT REFERENCES users(id) ON DELETE CASCADE,
    commander_type VARCHAR(50) NOT NULL,
    tier INT DEFAULT 1,
    expires_at TIMESTAMP WITH TIME ZONE,
    UNIQUE(user_id, commander_type)
);

CREATE TABLE IF NOT EXISTS battle_reports (
    id BIGSERIAL PRIMARY KEY,
    attacker_id BIGINT REFERENCES users(id),
    defender_id BIGINT REFERENCES users(id),
    coordinate_id BIGINT REFERENCES coordinates(id),
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    outcome VARCHAR(50),
    rounds_json TEXT,
    debris_metal BIGINT DEFAULT 0,
    debris_crystal BIGINT DEFAULT 0,
    moon_chance DOUBLE PRECISION DEFAULT 0,
    moon_created BOOLEAN DEFAULT FALSE,
    loot_metal NUMERIC(24,4) DEFAULT 0,
    loot_crystal NUMERIC(24,4) DEFAULT 0,
    loot_gas NUMERIC(24,4) DEFAULT 0
);

CREATE TABLE IF NOT EXISTS spy_reports (
    id BIGSERIAL PRIMARY KEY,
    attacker_id BIGINT REFERENCES users(id),
    target_id BIGINT REFERENCES users(id),
    coordinate_id BIGINT REFERENCES coordinates(id),
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    metal BIGINT DEFAULT 0,
    crystal BIGINT DEFAULT 0,
    gas BIGINT DEFAULT 0,
    fleet_json TEXT,
    defenses_json TEXT,
    buildings_json TEXT,
    research_json TEXT
);

CREATE TABLE IF NOT EXISTS debris_fields (
    coordinate_id BIGINT PRIMARY KEY REFERENCES coordinates(id),
    metal BIGINT DEFAULT 0,
    crystal BIGINT DEFAULT 0
);

CREATE TABLE IF NOT EXISTS fortress_attacks (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT REFERENCES users(id),
    attack_date DATE NOT NULL DEFAULT CURRENT_DATE,
    count INT NOT NULL DEFAULT 1,
    UNIQUE(user_id, attack_date)
);

CREATE TABLE IF NOT EXISTS alliances (
    id BIGSERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL UNIQUE,
    owner_id BIGINT REFERENCES users(id),
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    bulletin TEXT DEFAULT ''
);

CREATE TABLE IF NOT EXISTS alliance_members (
    id BIGSERIAL PRIMARY KEY,
    alliance_id BIGINT REFERENCES alliances(id) ON DELETE CASCADE,
    user_id BIGINT REFERENCES users(id) ON DELETE CASCADE,
    role VARCHAR(20) DEFAULT 'member',
    UNIQUE(alliance_id, user_id)
);

CREATE TABLE IF NOT EXISTS fortress_control (
    id BIGSERIAL PRIMARY KEY,
    alliance_id BIGINT REFERENCES alliances(id) ON DELETE SET NULL,
    captured_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS wormhole_cooldowns (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT REFERENCES users(id),
    moon_a_id BIGINT REFERENCES planets(id),
    moon_b_id BIGINT REFERENCES planets(id),
    cooldown_until TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(user_id, moon_a_id, moon_b_id)
);

CREATE TABLE IF NOT EXISTS galactonites (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT REFERENCES users(id) ON DELETE CASCADE,
    type_name VARCHAR(50) NOT NULL,
    stars INT DEFAULT 1,
    level INT DEFAULT 1,
    equipped BOOLEAN DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS crystal_energy (
    user_id BIGINT PRIMARY KEY REFERENCES users(id),
    amount BIGINT DEFAULT 0
);

CREATE TABLE IF NOT EXISTS galactonite_slots (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT REFERENCES users(id),
    planet_id BIGINT REFERENCES planets(id),
    galactonite_id BIGINT REFERENCES galactonites(id) UNIQUE,
    slot_index INT NOT NULL,
    UNIQUE(user_id, planet_id, slot_index)
);

CREATE TABLE IF NOT EXISTS messages (
    id BIGSERIAL PRIMARY KEY,
    sender_id BIGINT REFERENCES users(id),
    sender_name VARCHAR(100),
    recipient_id BIGINT REFERENCES users(id),
    alliance_id BIGINT REFERENCES alliances(id),
    subject VARCHAR(255),
    body TEXT,
    sent_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    is_read BOOLEAN DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS join_requests (
    id BIGSERIAL PRIMARY KEY,
    alliance_id BIGINT REFERENCES alliances(id) ON DELETE CASCADE,
    user_id BIGINT REFERENCES users(id) ON DELETE CASCADE,
    status VARCHAR(20) DEFAULT 'pending',
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(alliance_id, user_id)
);

CREATE TABLE IF NOT EXISTS president_votes (
    id BIGSERIAL PRIMARY KEY,
    alliance_id BIGINT REFERENCES alliances(id) ON DELETE CASCADE,
    initiator_id BIGINT REFERENCES users(id),
    expires_at TIMESTAMP WITH TIME ZONE NOT NULL,
    status VARCHAR(20) DEFAULT 'active',
    yea_count INT DEFAULT 0,
    nay_count INT DEFAULT 0,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS president_vote_votes (
    id BIGSERIAL PRIMARY KEY,
    vote_id BIGINT REFERENCES president_votes(id) ON DELETE CASCADE,
    user_id BIGINT REFERENCES users(id) ON DELETE CASCADE,
    vote VARCHAR(10) NOT NULL,
    UNIQUE(vote_id, user_id)
);

CREATE TABLE IF NOT EXISTS quest_definitions (
    id TEXT PRIMARY KEY,
    category TEXT NOT NULL DEFAULT 'intro',
    quest_type TEXT NOT NULL,
    target_value INT NOT NULL DEFAULT 1,
    sort_order INT NOT NULL DEFAULT 0,
    prerequisites TEXT[] DEFAULT '{}',
    rewards JSONB DEFAULT '{}',
    title TEXT DEFAULT '',
    description TEXT DEFAULT ''
);

CREATE TABLE IF NOT EXISTS user_quests (
    user_id BIGINT NOT NULL REFERENCES users(id),
    quest_id TEXT NOT NULL REFERENCES quest_definitions(id),
    progress INT NOT NULL DEFAULT 0,
    completed BOOLEAN DEFAULT FALSE,
    claimed BOOLEAN DEFAULT FALSE,
    started_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    completed_at TIMESTAMP WITH TIME ZONE,
    PRIMARY KEY (user_id, quest_id)
);

CREATE TABLE IF NOT EXISTS construction_queue_items (
    id BIGSERIAL PRIMARY KEY,
    planet_id BIGINT NOT NULL REFERENCES planets(id) ON DELETE CASCADE,
    building_name TEXT NOT NULL,
    from_level INT NOT NULL,
    to_level INT NOT NULL,
    sort_order INT NOT NULL DEFAULT 0,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS production_queue_items (
    id BIGSERIAL PRIMARY KEY,
    planet_id BIGINT NOT NULL REFERENCES planets(id) ON DELETE CASCADE,
    unit_type TEXT NOT NULL,
    quantity INT NOT NULL DEFAULT 1,
    sort_order INT NOT NULL DEFAULT 0,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS research_queue_items (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    tech_name TEXT NOT NULL,
    from_level INT NOT NULL,
    to_level INT NOT NULL,
    sort_order INT NOT NULL DEFAULT 0,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS fleet_presets (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    name TEXT NOT NULL,
    light_fighter INT DEFAULT 0,
    heavy_fighter INT DEFAULT 0,
    cruiser INT DEFAULT 0,
    battleship INT DEFAULT 0,
    dreadnought INT DEFAULT 0,
    destroyer INT DEFAULT 0,
    bomber INT DEFAULT 0,
    interceptor INT DEFAULT 0,
    iron_behemoth INT DEFAULT 0,
    devourer_interceptor INT DEFAULT 0,
    titanenergetic_frigate INT DEFAULT 0,
    annihilator INT DEFAULT 0,
    small_cargo INT DEFAULT 0,
    large_cargo INT DEFAULT 0,
    recycler INT DEFAULT 0,
    colony_ship INT DEFAULT 0,
    probe INT DEFAULT 0,
    solar_satellite INT DEFAULT 0,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS power_rank (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT REFERENCES users(id) UNIQUE,
    score BIGINT DEFAULT 0,
    calculated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);
