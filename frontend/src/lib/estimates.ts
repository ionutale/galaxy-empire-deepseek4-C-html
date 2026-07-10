// Frontend estimation of build/production/research times
// Formulas mirror the Go backend for display purposes

const BUILDING_CONFIGS: Record<string, { A: number; B: number }> = {
  metal_mine:             { A: 17.76721463, B: 0.406103736 },
  crystal_mine:           { A: 14.33641153, B: 0.477180667 },
  gas_mine:               { A: 14.33641153, B: 0.477180667 },
  solar_plant:            { A: 22,          B: 0.4 },
  fusion_reactor:         { A: 30,          B: 0.45 },
  robotics:               { A: 60,          B: 0.45 },
  nanite_factory:         { A: 7200,        B: 0.5 },
  shipyard:               { A: 60,          B: 0.45 },
  research_lab:           { A: 178.64176677, B: 0.522122844 },
  radar:                  { A: 30,          B: 0.4 },
  alliance_depot:         { A: 60,          B: 0.4 },
  metal_storage:          { A: 20,          B: 0.4 },
  crystal_storage:        { A: 20,          B: 0.4 },
  gas_storage:            { A: 20,          B: 0.4 },
  moon_base:              { A: 17.76721463, B: 0.406103736 },
  robotics_moon:          { A: 17.76721463, B: 0.406103736 },
  shipyard_moon:          { A: 17.76721463, B: 0.406103736 },
  pioneer_lab:            { A: 17.76721463, B: 0.406103736 },
  eu_x_radar:             { A: 17.76721463, B: 0.406103736 },
  wormhole_generator:      { A: 17.76721463, B: 0.406103736 },
  galactonite_research_center: { A: 17.76721463, B: 0.406103736 },
}

const TECH_MULTIPLIERS: Record<string, number> = {
  energy_tech: 2, laser_tech: 2, ion_tech: 2, hyperspace_tech: 2, plasma_tech: 2,
  ultra_temperature_tech: 1.4, anti_gravity_tech: 1.4,
  weapons_tech: 2, shielding_tech: 2, strength_tech: 2,
  combustion_drive: 2, impulse_drive: 2, hyperspace_drive: 2,
  astrophysics: 2, computer_tech: 2, espionage_tech: 2,
}

const SHIP_COSTS: Record<string, { metal: number; crystal: number }> = {
  light_fighter:         { metal: 2000,  crystal: 0 },
  heavy_fighter:         { metal: 6000,  crystal: 0 },
  cruiser:               { metal: 20000, crystal: 7000 },
  battleship:            { metal: 45000, crystal: 15000 },
  dreadnought:           { metal: 70000, crystal: 40000 },
  destroyer:             { metal: 60000, crystal: 50000 },
  bomber:                { metal: 50000, crystal: 25000 },
  interceptor:           { metal: 12000, crystal: 4000 },
  iron_behemoth:         { metal: 100000, crystal: 80000 },
  devourer_interceptor:  { metal: 30000, crystal: 15000 },
  titanenergetic_frigate: { metal: 80000, crystal: 60000 },
  annihilator:           { metal: 200000, crystal: 150000 },
  small_cargo:           { metal: 2000,  crystal: 2000 },
  large_cargo:           { metal: 6000,  crystal: 6000 },
  colony_ship:           { metal: 10000, crystal: 10000 },
  recycler:              { metal: 10000, crystal: 6000 },
  probe:                 { metal: 0,     crystal: 1000 },
  solar_satellite:       { metal: 0,     crystal: 2000 },
  rocket_launcher:       { metal: 2000,  crystal: 0 },
  light_laser:           { metal: 1500,  crystal: 500 },
  heavy_laser:           { metal: 6000,  crystal: 2000 },
  ion_cannon:            { metal: 5000,  crystal: 3000 },
  mk2_cannon:            { metal: 20000, crystal: 15000 },
  plasma_cannon:         { metal: 50000, crystal: 50000 },
  proton_cannon:         { metal: 100000, crystal: 100000 },
  small_shield_dome:     { metal: 10000, crystal: 10000 },
  large_shield_dome:     { metal: 50000, crystal: 50000 },
}

export function estimateBuildTime(building: string, targetLevel: number, roboticsLevel: number): number {
  const cfg = BUILDING_CONFIGS[building]
  if (!cfg) return 0
  const t = cfg.A * Math.exp(cfg.B * targetLevel) / (roboticsLevel + 1)
  return Math.round(t)
}

export function estimateProductionTime(unitType: string, quantity: number, shipyardLevel: number, naniteLevel: number): number {
  const cost = SHIP_COSTS[unitType]
  if (!cost) return 0
  const totalMC = cost.metal + cost.crystal
  if (totalMC === 0) return 0
  let seconds = totalMC * 3600 / (11132 * (shipyardLevel + 1)) / Math.pow(2, naniteLevel)
  seconds *= 0.2
  return Math.round(seconds * quantity)
}

export function estimateResearchTime(tech: string, targetLevel: number, labLevel: number): number {
  const mult = TECH_MULTIPLIERS[tech] || 2
  const m = Math.pow(mult, targetLevel - 1)
  const seconds = m * 10 / (labLevel + 1)
  return Math.round(seconds)
}
