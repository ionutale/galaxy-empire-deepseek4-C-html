export const SHIP_TYPES = [
  { key: 'light_fighter', label: 'Lt Fighter', cat: 'combat' },
  { key: 'heavy_fighter', label: 'Hv Fighter', cat: 'combat' },
  { key: 'cruiser', label: 'Cruiser', cat: 'combat' },
  { key: 'battleship', label: 'Battleship', cat: 'combat' },
  { key: 'dreadnought', label: 'Dreadnought', cat: 'combat' },
  { key: 'bomber', label: 'Bomber', cat: 'combat' },
  { key: 'destroyer', label: 'Destroyer', cat: 'combat' },
  { key: 'small_cargo', label: 'S Cargo', cat: 'civil' },
  { key: 'large_cargo', label: 'L Cargo', cat: 'civil' },
  { key: 'colony_ship', label: 'Colony', cat: 'civil' },
  { key: 'recycler', label: 'Recycler', cat: 'civil' },
  { key: 'probe', label: 'Probe', cat: 'civil' },
  { key: 'solar_satellite', label: 'Solar Sat', cat: 'civil' },
  { key: 'interceptor', label: 'Interceptor', cat: 'combat' },
  { key: 'iron_behemoth', label: 'Iron Behemoth', cat: 'combat' },
  { key: 'devourer_interceptor', label: 'Devourer Int', cat: 'combat' },
  { key: 'titanenergetic_frigate', label: 'Titan Frigate', cat: 'combat' },
  { key: 'annihilator', label: 'Annihilator', cat: 'combat' },
]

export const SHIP_KEYS = SHIP_TYPES.map(s => s.key)

export const DEFENSE_TYPES = [
  { key: 'rocket_launcher', label: 'Rocket Launcher', cat: 'defense' },
  { key: 'light_laser', label: 'Light Laser', cat: 'defense' },
  { key: 'heavy_laser', label: 'Heavy Laser', cat: 'defense' },
  { key: 'ion_cannon', label: 'Ion Cannon', cat: 'defense' },
  { key: 'mk2_cannon', label: 'MK2 Cannon', cat: 'defense' },
  { key: 'plasma_cannon', label: 'Plasma Cannon', cat: 'defense' },
  { key: 'proton_cannon', label: 'Proton Cannon', cat: 'defense' },
  { key: 'small_shield_dome', label: 'S Shield', cat: 'defense' },
  { key: 'large_shield_dome', label: 'L Shield', cat: 'defense' },
]

export const DEFENSE_KEYS = DEFENSE_TYPES.map(d => d.key)

const UNIT_LABELS: Record<string, string> = {
  light_fighter: 'Lt Fighter', heavy_fighter: 'Hv Fighter',
  cruiser: 'Cruiser', battleship: 'Battleship',
  dreadnought: 'Dreadnought', destroyer: 'Destroyer',
  bomber: 'Bomber',
  small_cargo: 'S Cargo', large_cargo: 'L Cargo',
  colony_ship: 'Colony', recycler: 'Recycler',
  probe: 'Probe', solar_satellite: 'Solar Sat',
  interceptor: 'Interceptor', iron_behemoth: 'Iron Behemoth',
  devourer_interceptor: 'Devourer Int', titanenergetic_frigate: 'Titan Frigate',
  annihilator: 'Annihilator',
  rocket_launcher: 'Rocket Launcher', light_laser: 'Light Laser',
  heavy_laser: 'Heavy Laser', ion_cannon: 'Ion Cannon',
  mk2_cannon: 'MK2 Cannon', plasma_cannon: 'Plasma Cannon',
  proton_cannon: 'Proton Cannon',
  small_shield_dome: 'S Shield', large_shield_dome: 'L Shield',
}

export function unitLabel(key: string): string {
  return UNIT_LABELS[key] || key
}
