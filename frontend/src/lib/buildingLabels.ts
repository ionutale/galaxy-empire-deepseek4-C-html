const labels: Record<string, string> = {
  metal_mine: 'Metal Mine',
  crystal_mine: 'Crystal Mine',
  gas_mine: 'Gas Mine',
  solar_plant: 'Solar Plant',
  fusion_reactor: 'Fusion Reactor',
  robotics: 'Robotics',
  nanite_factory: 'Nanite Factory',
  shipyard: 'Shipyard',
  research_lab: 'Research Lab',
  radar: 'Radar',
  alliance_depot: 'Alliance Depot',
  metal_storage: 'Metal Storage',
  crystal_storage: 'Crystal Storage',
  gas_storage: 'Gas Storage',
  galactonite_research_center: 'Galactonite Res.',
  moon_base: 'Moon Base',
  robotics_moon: 'Robotics (Moon)',
  shipyard_moon: 'Shipyard (Moon)',
  pioneer_lab: 'Pioneer Lab',
  eu_x_radar: 'EU-X Radar',
  wormhole_generator: 'Wormhole Gen',
}

export function buildingLabel(key: string): string {
  return labels[key] || key
}
