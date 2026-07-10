const BASE_URL = import.meta.env.DEV ? '/api' : '/api';

export interface User {
  id: string;
  username: string;
  server_id: number;
  dark_matter: number;
  credits: number;
  created_at: string;
  home_planet_id: string;
}

export interface AuthResponse {
  token: string;
  user_id: string;
  username: string;
}

export interface PlanetSummary {
  id: string;
  coordinate_id: string;
  planet_size: number;
  current_metal: number;
  current_crystal: number;
  current_gas: number;
}

export interface PlanetView {
  id: string;
  owner_id: string;
  coordinate_id: string;
  galaxy: number;
  system: number;
  position: number;
  planet_size: number;
  temperature: number;
  metal_mine_level: number;
  crystal_mine_level: number;
  gas_mine_level: number;
  solar_plant_level: number;
  fusion_reactor_level: number;
  nanite_factory_level: number;
  shipyard_level: number;
  robotics_level: number;
  research_lab_level: number;
  radar_level: number;
  alliance_depot_level: number;
  metal_storage_level: number;
  crystal_storage_level: number;
  gas_storage_level: number;
  current_metal: number;
  current_crystal: number;
  current_gas: number;
  metal_per_hour: number;
  crystal_per_hour: number;
  gas_per_hour: number;
  energy_produced: number;
  energy_consumed: number;
  metal_storage: number;
  crystal_storage: number;
  gas_storage: number;
  used_slots: number;
  construction_end: string | null;
  const_building: string | null;
  is_moon: boolean;
  moon_base_level: number;
  robotics_moon_level: number;
  shipyard_moon_level: number;
  pioneer_lab_level: number;
  eu_x_radar_level: number;
  wormhole_generator_level: number;
  galactonite_research_center_level: number;
  construction_queue: QueueItem[];
  production_end: string | null;
  prod_unit_type: string | null;
  prod_quantity: number;
  production_queue: ProductionQueueItem[];
  fleet_inventory?: FleetInventory;
}

export interface QueueItem {
  building: string;
  target_level: number;
}

export interface BuildingInfo {
  name: string;
  level: number;
  target_level: number;
  cost: { metal: number; crystal: number; gas: number };
  time_seconds: number;
  prereqs: Record<string, number> | null;
  met: boolean;
  current_effect: string;
  upgrade_effect: string;
}

let onUnauthorized: (() => void) | null = null;

export function setOnUnauthorized(cb: () => void) {
  onUnauthorized = cb;
}

class ApiClient {
  private async request<T>(path: string, options?: RequestInit): Promise<T> {
    const controller = new AbortController();
    const timeout = setTimeout(() => controller.abort(), 30000);

    try {
      const token = localStorage.getItem('token');
      const headers: Record<string, string> = { 'Content-Type': 'application/json' };
      if (token) {
        headers['Authorization'] = `Bearer ${token}`;
      }

      const opts = { ...options }
      if (opts.body && typeof opts.body === 'object' && !(opts.body instanceof FormData)) {
        opts.body = JSON.stringify(opts.body)
      }
      const res = await fetch(`${BASE_URL}${path}`, {
        ...opts,
        headers: { ...headers, ...options?.headers },
        signal: controller.signal,
      });

      const body = await res.json();

      if (res.status === 401) {
        localStorage.removeItem('token');
        onUnauthorized?.();
        throw new Error(body.error || 'unauthorized');
      }

      if (!res.ok) {
        throw new Error(body.error || 'Request failed');
      }
      return body;
    } finally {
      clearTimeout(timeout);
    }
  }

  async health(): Promise<{ status: string }> {
    return this.request('/health');
  }

  async register(username: string, password: string): Promise<AuthResponse> {
    return this.request('/auth/register', {
      method: 'POST',
      body: JSON.stringify({ username, password }),
    });
  }

  async login(username: string, password: string): Promise<AuthResponse> {
    return this.request('/auth/login', {
      method: 'POST',
      body: JSON.stringify({ username, password }),
    });
  }

  async me(): Promise<User> {
    return this.request('/me');
  }

  async getPlanets(): Promise<PlanetSummary[]> {
    return this.request('/planets');
  }

  async getPlanet(id: string): Promise<PlanetView> {
    return this.request(`/planets/${id}`);
  }

  async getBuildingInfo(planetId: string, building: string): Promise<BuildingInfo> {
    return this.request(`/planets/${planetId}/build/${building}`);
  }

  async cancelBuilding(planetId: string, building: string): Promise<{ status: string }> {
    return this.fetch(`/api/planets/${planetId}/cancel`, { method: 'POST', body: { building } });
  }

  async buildBuilding(planetId: string, building: string, level: number): Promise<{ status: string }> {
    return this.request(`/planets/${planetId}/build`, {
      method: 'POST',
      body: JSON.stringify({ building, level }),
    });
  }

  async getResearch(): Promise<Research> {
    return this.request('/research');
  }

  async getTechInfo(planetId: string, tech: string): Promise<TechInfo> {
    return this.request(`/research/${tech}?planet_id=${planetId}`);
  }

  async queueResearch(planetId: string, tech: string): Promise<{ status: string }> {
    return this.request(`/research/${tech}?planet_id=${planetId}`, {
      method: 'POST',
    });
  }

  async cancelResearch(tech: string): Promise<{ status: string }> {
    return this.request('/research/cancel', {
      method: 'POST',
      body: JSON.stringify({ tech }),
    });
  }

  async getShipyard(planetId: string): Promise<ShipyardData> {
    return this.request(`/planets/${planetId}/shipyard`);
  }

  async produceUnit(planetId: string, unitType: string, quantity: number): Promise<{ status: string }> {
    return this.request(`/planets/${planetId}/produce`, {
      method: 'POST',
      body: JSON.stringify({ unit_type: unitType, quantity }),
    });
  }

  async cancelProduction(planetId: string, unitType: string): Promise<{ status: string }> {
    return this.request(`/planets/${planetId}/cancel-production`, {
      method: 'POST',
      body: JSON.stringify({ unit_type: unitType }),
    });
  }

  async cancelBuilding(planetId: string, building: string): Promise<{ status: string }> {
    return this.request(`/planets/${planetId}/cancel`, { method: 'POST', body: JSON.stringify({ building }) });
  }

  async getSystem(galaxy: number, system: number): Promise<SystemPosition[]> {
    return this.request(`/galaxy/${galaxy}/${system}`);
  }

  async sendFleet(originId: string, galaxy: number, system: number, position: number, missionType: string, ships: Record<string, number>, cargoMetal?: number, cargoCrystal?: number, cargoGas?: number): Promise<{ status: string }> {
    return this.request('/fleet/send', {
      method: 'POST',
      body: JSON.stringify({
        origin_planet_id: originId,
        galaxy, system, position,
        mission_type: missionType,
        ships,
        cargo_metal: cargoMetal || 0,
        cargo_crystal: cargoCrystal || 0,
        cargo_gas: cargoGas || 0,
      }),
    });
  }

  async recallFleet(missionId: string): Promise<{ status: string }> {
    return this.request(`/fleet/${missionId}/recall`, {
      method: 'POST',
    });
  }

  async getFleetEstimate(originPlanetId: string, galaxy: number, system: number, position: number, ships: Record<string, number>): Promise<{ flight_time_seconds: number; arrival_time: string; return_time: string; distance: number; fuel_cost: number }> {
    const params = new URLSearchParams({ origin_planet_id: originPlanetId, galaxy: String(galaxy), system: String(system), position: String(position) })
    for (const [k, v] of Object.entries(ships)) {
      if (v > 0) params.set(k, String(v))
    }
    return this.request(`/fleet/estimate?${params}`);
  }

  async getFleets(): Promise<FleetMission[]> {
    return this.request('/fleet');
  }

  async getPresets(): Promise<{ id: string; name: string; ships: Record<string, number> }[]> {
    return this.request('/fleet/presets');
  }

  async createPreset(name: string, ships: Record<string, number>): Promise<{ id: string; name: string; ships: Record<string, number> }> {
    return this.request('/fleet/presets', { method: 'POST', body: { name, ships } });
  }

  async deletePreset(id: string): Promise<{ status: string }> {
    return this.request(`/fleet/presets/${id}`, { method: 'DELETE' });
  }

  async getFortressStatus(): Promise<FortressStatus> {
    return this.request('/fortress');
  }

  async attackFortress(planetId: string, ships: Record<string, number>): Promise<FleetMission> {
    return this.request('/fortress/attack', {
      method: 'POST',
      body: JSON.stringify({ planet_id: planetId, ships }),
    });
  }

  async claimFortress(): Promise<any> {
    return this.request('/fortress/claim', { method: 'POST' });
  }

  async getWormholeStatus(): Promise<WormholeStatus> {
    return this.request('/wormhole/status');
  }

  async sendWormhole(originMoonId: string, targetMoonId: string, ships: Record<string, number>): Promise<{ status: string }> {
    return this.request('/wormhole/send', {
      method: 'POST',
      body: JSON.stringify({ origin_moon_id: originMoonId, target_moon_id: targetMoonId, ships }),
    });
  }

  async getEUXStatus(): Promise<EUXStatus> {
    return this.request('/eu-x/status');
  }

  async scanSystem(moonId: string, galaxy: number, system: number): Promise<ScanResult> {
    return this.request('/eu-x/scan', {
      method: 'POST',
      body: JSON.stringify({ moon_id: moonId, galaxy, system }),
    });
  }

  async getGalactonites(): Promise<GalactoniteData> {
    return this.request('/galactonites');
  }

  async getCrystalEnergy(): Promise<{ amount: number }> {
    return this.request('/galactonites/crystal-energy');
  }

  async equipGalactonite(galactoniteId: string, planetId: string): Promise<{ status: string }> {
    return this.request('/galactonites/equip', {
      method: 'POST',
      body: JSON.stringify({ galactonite_id: galactoniteId, planet_id: planetId }),
    });
  }

  async unequipGalactonite(galactoniteId: string): Promise<{ status: string }> {
    return this.request('/galactonites/unequip', {
      method: 'POST',
      body: JSON.stringify({ galactonite_id: galactoniteId }),
    });
  }

  async upgradeGalactonite(galactoniteId: string): Promise<{ status: string }> {
    return this.request('/galactonites/upgrade', {
      method: 'POST',
      body: JSON.stringify({ galactonite_id: galactoniteId }),
    });
  }

  async fuseGalactonite(galactoniteIds: string[]): Promise<Galactonite> {
    return this.request('/galactonites/fuse', {
      method: 'POST',
      body: JSON.stringify({ galactonite_ids: galactoniteIds }),
    });
  }

  async convertResources(planetId: string, metal: number, crystal: number, gas: number): Promise<{ status: string }> {
    return this.request('/galactonites/convert', {
      method: 'POST',
      body: JSON.stringify({ planet_id: planetId, metal, crystal, gas }),
    });
  }

  async buyCrystalEnergy(amount: number, currency: string): Promise<{ status: string }> {
    return this.request('/galactonites/buy-energy', {
      method: 'POST',
      body: JSON.stringify({ amount, currency }),
    });
  }

  async setBulletin(allianceId: string, bulletin: string): Promise<{ status: string }> {
    return this.request(`/alliance/${allianceId}/bulletin`, {
      method: 'POST',
      body: JSON.stringify({ bulletin }),
    });
  }

  async sendMail(allianceId: string, subject: string, body: string): Promise<{ status: string }> {
    return this.request(`/alliance/${allianceId}/mail`, {
      method: 'POST',
      body: JSON.stringify({ subject, body }),
    });
  }

  async getMail(allianceId: string): Promise<any[]> {
    return this.request(`/alliance/${allianceId}/mail`);
  }

  async requestJoin(allianceId: string): Promise<{ status: string }> {
    return this.request(`/alliance/${allianceId}/join`, { method: 'POST' });
  }

  async invitePlayer(allianceId: string, username: string): Promise<{ status: string }> {
    return this.request(`/alliance/${allianceId}/invite`, {
      method: 'POST',
      body: JSON.stringify({ username }),
    });
  }

  async approveJoin(allianceId: string, userId: string): Promise<{ status: string }> {
    return this.request(`/alliance/${allianceId}/approve/${userId}`, { method: 'POST' });
  }

  async denyJoin(allianceId: string, userId: string): Promise<{ status: string }> {
    return this.request(`/alliance/${allianceId}/deny/${userId}`, { method: 'POST' });
  }

  async getJoinRequests(allianceId: string): Promise<any[]> {
    return this.request(`/alliance/${allianceId}/join-requests`);
  }

  async initiateVote(allianceId: string): Promise<any> {
    return this.request(`/alliance/${allianceId}/vote-initiate`, { method: 'POST' });
  }

  async castVote(allianceId: string, vote: string): Promise<{ status: string }> {
    return this.request(`/alliance/${allianceId}/vote`, {
      method: 'POST',
      body: JSON.stringify({ vote }),
    });
  }

  async getActiveVote(allianceId: string): Promise<any> {
    return this.request(`/alliance/${allianceId}/votes`);
  }

  async createAlliance(name: string): Promise<AllianceDetail> {
    return this.request('/alliance/create', {
      method: 'POST',
      body: JSON.stringify({ name }),
    });
  }

  async getMyAlliance(): Promise<AllianceDetail | null> {
    return this.request('/alliance/mine');
  }

  async getAlliance(id: string): Promise<AllianceDetail> {
    return this.request(`/alliance/${id}`);
  }

  async leaveAlliance(allianceId: string): Promise<{ status: string }> {
    return this.request(`/alliance/${allianceId}/leave`, { method: 'POST' });
  }

  async disbandAlliance(id: string): Promise<{ status: string }> {
    return this.request(`/alliance/${id}/disband`, { method: 'POST' });
  }

  async promoteMember(allianceId: string, userId: string): Promise<{ status: string }> {
    return this.request(`/alliance/${allianceId}/promote/${userId}`, { method: 'POST' });
  }

  async demoteMember(allianceId: string, userId: string): Promise<{ status: string }> {
    return this.request(`/alliance/${allianceId}/demote/${userId}`, { method: 'POST' });
  }

  async getPowerRank(): Promise<{ score: number }> {
    return this.request('/power-rank');
  }

  async getCommanders(): Promise<CommanderData> {
    return this.request('/commanders');
  }

  async hireCommander(commanderType: string, tier: number): Promise<{ status: string }> {
    return this.request('/commanders/hire', { method: 'POST', body: JSON.stringify({ commander_type: commanderType, tier }) });
  }

  async getReports(): Promise<BattleReport[]> {
    return this.request('/reports');
  }

  async getSpyReports(): Promise<SpyReport[]> {
    return this.request('/reports/spy');
  }

  async adminSetResources(planetId: string, metal: number, crystal: number, gas: number): Promise<{ status: string }> {
    return this.request(`/admin/planet/${planetId}/resources`, {
      method: 'POST',
      body: JSON.stringify({ metal, crystal, gas }),
    });
  }

  async adminSetBuilding(planetId: string, building: string, level: number): Promise<{ status: string }> {
    return this.request(`/admin/planet/${planetId}/building`, {
      method: 'POST',
      body: JSON.stringify({ building, level }),
    });
  }

  async adminSetDarkMatter(userId: string, amount: number): Promise<{ status: string }> {
    return this.request(`/admin/user/${userId}/dark-matter`, {
      method: 'POST',
      body: JSON.stringify({ amount }),
    });
  }

  async adminSetCredits(userId: string, amount: number): Promise<{ status: string }> {
    return this.request(`/admin/user/${userId}/credits`, {
      method: 'POST',
      body: JSON.stringify({ amount }),
    });
  }

  async adminSetResearch(userId: string, tech: string, level: number): Promise<{ status: string }> {
    return this.request(`/admin/user/${userId}/research`, {
      method: 'POST',
      body: JSON.stringify({ tech, level }),
    });
  }

  async adminSetFleet(planetId: string, ships: Record<string, number>): Promise<{ status: string }> {
    return this.request(`/admin/planet/${planetId}/fleet`, {
      method: 'POST',
      body: JSON.stringify(ships),
    });
  }

  async adminSetDefense(planetId: string, defense: Record<string, number>): Promise<{ status: string }> {
    return this.request(`/admin/planet/${planetId}/defense`, {
      method: 'POST',
      body: JSON.stringify(defense),
    });
  }

  async getQuests(category: string): Promise<any[]> {
    return this.request(`/quests?category=${category}`);
  }

  async claimQuest(questId: string): Promise<{ status: string }> {
    return this.request('/quests/claim', { method: 'POST', body: { quest_id: questId } });
  }

  async claimDailyGift(): Promise<{ reward: any }> {
    return this.request('/daily-gift', { method: 'POST' });
  }

  async sendGEFAttack(planetId: string, stageId: number, ships: FleetShips): Promise<{ status: string }> {
    return this.request('/gef/attack', {
      method: 'POST',
      body: JSON.stringify({ planet_id: planetId, stage_id: stageId, ships }),
    });
  }

  async getProfile(): Promise<{ username: string; dark_matter: number; credits: number; created_at: string }> {
    return this.request('/profile');
  }

  async changePassword(currentPassword: string, newPassword: string): Promise<{ status: string }> {
    return this.request('/profile/change-password', {
      method: 'POST',
      body: JSON.stringify({ current_password: currentPassword, new_password: newPassword }),
    });
  }

  async adminSetResourcesByCoord(galaxy: number, system: number, position: number, metal: number, crystal: number, gas: number): Promise<{ status: string }> {
    return this.request('/admin/planet/coord/resources', {
      method: 'POST',
      body: JSON.stringify({ galaxy, system, position, metal, crystal, gas }),
    });
  }

  async adminSetBuildingByCoord(galaxy: number, system: number, position: number, building: string, level: number): Promise<{ status: string }> {
    return this.request('/admin/planet/coord/building', {
      method: 'POST',
      body: JSON.stringify({ galaxy, system, position, building, level }),
    });
  }

  async adminSetFleetByCoord(galaxy: number, system: number, position: number, ships: Record<string, number>): Promise<{ status: string }> {
    return this.request('/admin/planet/coord/fleet', {
      method: 'POST',
      body: JSON.stringify({ galaxy, system, position, ships }),
    });
  }

  async adminSetDefenseByCoord(galaxy: number, system: number, position: number, defenses: Record<string, number>): Promise<{ status: string }> {
    return this.request('/admin/planet/coord/defense', {
      method: 'POST',
      body: JSON.stringify({ galaxy, system, position, defenses }),
    });
  }

  async adminSetDarkMatterByUsername(username: string, amount: number): Promise<{ status: string }> {
    return this.request(`/admin/user/by-username/dark-matter`, {
      method: 'POST',
      body: JSON.stringify({ amount }),
    });
  }

  async adminSetCreditsByUsername(username: string, amount: number): Promise<{ status: string }> {
    return this.request(`/admin/user/by-username/credits`, {
      method: 'POST',
      body: JSON.stringify({ amount }),
    });
  }

  async adminSetResearchByUsername(username: string, tech: string, level: number): Promise<{ status: string }> {
    return this.request(`/admin/user/by-username/research`, {
      method: 'POST',
      body: JSON.stringify({ tech, level }),
    });
  }
}

export interface Alliance {
  id: string;
  name: string;
  owner_id: string;
  created_at: string;
  bulletin: string;
}

export interface AllianceMember {
  id: string;
  alliance_id: string;
  user_id: string;
  username: string;
  role: string;
}

export interface AllianceDetail {
  alliance: Alliance;
  members: AllianceMember[];
  my_role: string;
}

export interface Galactonite {
  id: string;
  owner_id: string;
  type_name: string;
  stars: number;
  level: number;
  equipped: boolean;
}

export interface EquippedSlot {
  id: string;
  user_id: string;
  planet_id: string;
  galactonite_id: string;
  slot_index: number;
}

export interface Message {
  id: string;
  sender_id: string;
  sender_name?: string;
  alliance_id?: string;
  subject: string;
  body: string;
  sent_at: string;
}

export interface GalactoniteData {
  inventory: Galactonite[];
  equipped: EquippedSlot[];
  slots_per_planet: number;
  crystal_energy: number;
}

export interface Research {
  user_id: string;
  energy_tech: number;
  laser_tech: number;
  ion_tech: number;
  hyperspace_tech: number;
  plasma_tech: number;
  astrophysics: number;
  computer_tech: number;
  espionage_tech: number;
  ultra_temperature_tech: number;
  anti_gravity_tech: number;
  combustion_drive: number;
  impulse_drive: number;
  hyperspace_drive: number;
  weapons_tech: number;
  shielding_tech: number;
  strength_tech: number;
  queued_tech: string | null;
  research_queue: { tech: string }[];
  research_start?: string;
  research_end?: string;
}

export interface TechInfo {
  name: string;
  level: number;
  cost: { metal: number; crystal: number; gas: number };
  time_seconds: number;
  prereqs: Record<string, number> | null;
  met: boolean;
  lab_level: number;
}

export interface ShipUnitInfo {
  type: string;
  metal: number;
  crystal: number;
  gas: number;
  time_seconds: number;
  prereqs: Record<string, number> | null;
  met: boolean;
}

export interface DefenseUnitInfo {
  type: string;
  metal: number;
  crystal: number;
  gas: number;
  time_seconds: number;
  prereqs: Record<string, number> | null;
  met: boolean;
}

export interface FleetInventory {
  light_fighter: number;
  heavy_fighter: number;
  cruiser: number;
  battleship: number;
  dreadnought: number;
  bomber: number;
  destroyer: number;
  small_cargo: number;
  large_cargo: number;
  colony_ship: number;
  recycler: number;
  probe: number;
  solar_satellite: number;
  interceptor: number;
  iron_behemoth: number;
  devourer_interceptor: number;
  titanenergetic_frigate: number;
  annihilator: number;
}

export interface DefenseInventory {
  rocket_launcher: number;
  light_laser: number;
  heavy_laser: number;
  ion_cannon: number;
  mk2_cannon: number;
  plasma_cannon: number;
  proton_cannon: number;
  small_shield_dome: number;
  large_shield_dome: number;
}

export interface ProductionQueueItem {
  unit_type: string;
  quantity: number;
}

export interface ProductionStatus {
  unit_type: string;
  quantity: number;
  end_time: string;
  remaining_seconds: number;
}

export interface ShipyardData {
  shipyard_level: number;
  nanite_level: number;
  ships: ShipUnitInfo[];
  defenses: DefenseUnitInfo[];
  fleet_inventory: FleetInventory;
  defense_inventory: DefenseInventory;
  production: ProductionStatus | null;
  production_queue: ProductionQueueItem[];
}

export interface SystemPosition {
  position: number;
  owner_name?: string;
  planet_id?: string;
  is_npc?: boolean;
  npc_tier?: string;
  is_nebula?: boolean;
  is_empty?: boolean;
  is_moon?: boolean;
}

export interface FleetMission {
  id: string;
  owner_id: string;
  origin_planet_id: string;
  target_galaxy: number;
  target_system: number;
  target_position: number;
  mission_type: string;
  departure_time: string;
  arrival_time: string;
  return_time: string;
  ships: FleetInventory;
  recalled: boolean;
  cargo_metal?: number;
  cargo_crystal?: number;
  cargo_gas?: number;
  harvested_metal?: number;
  harvested_crystal?: number;
  explored_metal?: number;
  explored_crystal?: number;
  explored_gas?: number;
}

export interface FortressStatus {
  attacks_today: number;
  max_attacks: number;
  garrison: Record<string, number>;
  coordinates: { galaxy: number; system: number; position: number };
  controlling_alliance: string;
  controlling_alliance_id: string | null;
}

export interface WormholeCooldown {
  moon_a_id: string;
  moon_b_id: string;
  cooldown_until: string;
}

export interface WormholeMoon {
  id: string;
  coordinate: string;
  wormhole_level: number;
}

export interface WormholeStatus {
  moons: WormholeMoon[];
  cooldowns: WormholeCooldown[];
}

export interface EUXMoon {
  id: string;
  coordinate: string;
  eu_x_level: number;
  galaxy: number;
}

export interface EUXStatus {
  moons: EUXMoon[];
}

export interface DetectedFleet {
  position: number;
  arrival_time: string;
  return_time: string;
  mission_type: string;
  ships: Record<string, number>;
}

export interface ScanResult {
  system: string;
  detected_fleets: DetectedFleet[];
}

export interface Commander {
  id: string;
  user_id: string;
  commander_type: string;
  tier: number;
  expires_at: string;
}

export interface CommanderData {
  available: Record<string, Record<number, number>>;
  active: Record<string, Commander>;
}

export interface BattleReport {
  id: string;
  attacker_id: string;
  defender_id: string | null;
  coordinate_id: string | null;
  timestamp: string;
  outcome: string;
  debris_metal: number;
  debris_crystal: number;
  loot_metal: number;
  loot_crystal: number;
  loot_gas: number;
  moon_chance: number;
  moon_created: boolean;
  galactonite_drops: { type: string; stars: number }[];
}

export interface SpyReport {
  id: string;
  attacker_id: string;
  target_id: string;
  created_at: string;
  metal: number;
  crystal: number;
  gas: number;
  fleet?: FleetInventory | null;
  defenses?: DefenseInventory | null;
  buildings?: Record<string, number> | null;
  research?: Research | null;
}

export interface FleetShips {
  light_fighter: number; heavy_fighter: number; cruiser: number;
  battleship: number; dreadnought: number; bomber: number;
  destroyer: number; small_cargo: number; large_cargo: number;
  colony_ship: number; recycler: number; probe: number;
  solar_satellite: number; interceptor: number; iron_behemoth: number;
  devourer_interceptor: number; titanenergetic_frigate: number;
  annihilator: number;
}

export const api = new ApiClient();
