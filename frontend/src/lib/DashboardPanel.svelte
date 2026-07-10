<script lang="ts">
  import { api, type PlanetView, type FleetInventory } from './api'
  import { fmtNum } from './format'
  import { SHIP_TYPES } from './shipTypes'

  let planets = $state<PlanetView[]>([])
  let loading = $state(true)
  let error = $state('')

  $effect(() => {
    async function load() {
      try {
        const list = await api.getPlanets()
        const views = await Promise.all(list.map(p => api.getPlanet(p.id)))
        planets = views
      } catch (e: any) {
        error = e.message
      } finally {
        loading = false
      }
    }
    load()
  })

  let totalMetal = $derived(planets.reduce((s, p) => s + p.current_metal, 0))
  let totalCrystal = $derived(planets.reduce((s, p) => s + p.current_crystal, 0))
  let totalGas = $derived(planets.reduce((s, p) => s + p.current_gas, 0))
  let totalMetalPH = $derived(planets.reduce((s, p) => s + p.metal_per_hour, 0))
  let totalCrystalPH = $derived(planets.reduce((s, p) => s + p.crystal_per_hour, 0))
  let totalGasPH = $derived(planets.reduce((s, p) => s + p.gas_per_hour, 0))

  let maxMetalPH = $derived(Math.max(...planets.map(p => p.metal_per_hour), 1))
  let maxCrystalPH = $derived(Math.max(...planets.map(p => p.crystal_per_hour), 1))
  let maxGasPH = $derived(Math.max(...planets.map(p => p.gas_per_hour), 1))

  // Fleet aggregation
  function sumFleet(fn: (f: FleetInventory) => number): number {
    return planets.reduce((s, p) => s + (p.fleet_inventory ? fn(p.fleet_inventory) : 0), 0)
  }

  let totalShips = $derived(planets.reduce((s, p) => {
    if (!p.fleet_inventory) return s
    const inv = p.fleet_inventory as any
    for (const st of SHIP_TYPES) s += inv[st.key] || 0
    return s
  }, 0))

  let combatTypes = $derived(SHIP_TYPES.filter(s => s.cat === 'combat'))
  let civilTypes = $derived(SHIP_TYPES.filter(s => s.cat === 'civil'))
  let defenseTypes = $derived(SHIP_TYPES.filter(s => s.cat === 'defense'))

  let totalCombat = $derived(sumFleet(f => {
    let t = 0; for (const s of combatTypes) t += (f as any)[s.key] || 0; return t
  }))
  let totalCivil = $derived(sumFleet(f => {
    let t = 0; for (const s of civilTypes) t += (f as any)[s.key] || 0; return t
  }))

  let maxFleetType = $derived(Math.max(...SHIP_TYPES.map(st => sumFleet(f => (f as any)[st.key] || 0)), 1))
</script>

<div class="space-y-4">
  <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">Dashboard</h2>

  {#if error}
    <div class="bg-red-900/50 text-red-400 p-2 rounded text-sm">{error}</div>
  {/if}

  {#if loading}
    <div class="text-gray-500 text-xs text-center py-8">Loading empire data...</div>
  {:else}
    <!-- Total Resources -->
    <div class="bg-[#111827] rounded-lg border border-[#1e293b] p-4">
      <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide mb-3">Empire Totals</h3>
      <div class="grid grid-cols-3 gap-3">
        <div class="bg-[#0a0e1a] rounded-lg p-3 text-center">
          <div class="text-lg font-bold text-red-400">{fmtNum(totalMetal)}</div>
          <div class="text-[10px] text-gray-500">Metal</div>
          <div class="text-xs text-green-400">+{fmtNum(totalMetalPH)}/h</div>
        </div>
        <div class="bg-[#0a0e1a] rounded-lg p-3 text-center">
          <div class="text-lg font-bold text-blue-400">{fmtNum(totalCrystal)}</div>
          <div class="text-[10px] text-gray-500">Crystal</div>
          <div class="text-xs text-green-400">+{fmtNum(totalCrystalPH)}/h</div>
        </div>
        <div class="bg-[#0a0e1a] rounded-lg p-3 text-center">
          <div class="text-lg font-bold text-green-400">{fmtNum(totalGas)}</div>
          <div class="text-[10px] text-gray-500">Gas</div>
          <div class="text-xs text-green-400">+{fmtNum(totalGasPH)}/h</div>
        </div>
      </div>
    </div>

    <!-- Fleet Summary -->
    <div class="bg-[#111827] rounded-lg border border-[#1e293b] p-4">
      <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide mb-3">Fleet Summary</h3>
      <div class="grid grid-cols-3 gap-3 mb-3">
        <div class="bg-[#0a0e1a] rounded-lg p-3 text-center">
          <div class="text-lg font-bold text-white">{fmtNum(totalShips)}</div>
          <div class="text-[10px] text-gray-500">Total Ships</div>
        </div>
        <div class="bg-[#0a0e1a] rounded-lg p-3 text-center">
          <div class="text-lg font-bold text-red-400">{fmtNum(totalCombat)}</div>
          <div class="text-[10px] text-gray-500">Combat</div>
        </div>
        <div class="bg-[#0a0e1a] rounded-lg p-3 text-center">
          <div class="text-lg font-bold text-cyan-400">{fmtNum(totalCivil)}</div>
          <div class="text-[10px] text-gray-500">Civil</div>
        </div>
      </div>
      <!-- Fleet composition bar chart -->
      <div class="space-y-1.5">
        {#each SHIP_TYPES as st}
          {@const count = sumFleet(f => (f as any)[st.key] || 0)}
          {#if count > 0}
            <div class="flex items-center gap-2 text-[10px]">
              <span class="text-gray-400 w-16 shrink-0 truncate">{st.label}</span>
              <div class="flex-1 bg-[#0a0e1a] rounded-full h-3 overflow-hidden">
                <div class="h-full rounded-full {st.cat === 'combat' ? 'bg-red-600' : st.cat === 'civil' ? 'bg-cyan-600' : 'bg-amber-600'}" style="width: {(count / maxFleetType) * 100}%"></div>
              </div>
              <span class="text-gray-500 w-16 text-right font-mono">{fmtNum(count)}</span>
            </div>
          {/if}
        {/each}
      </div>
    </div>

    <!-- Planet Cards with Charts -->
    <div class="space-y-3">
      {#each planets as p, i}
        <div class="bg-[#111827] rounded-lg border border-[#1e293b] p-4">
          <div class="flex items-center justify-between mb-3">
            <div>
              <h3 class="text-sm font-medium text-white">Planet {i + 1}</h3>
              <p class="text-[10px] text-gray-500">Size: {p.planet_size} | Temp: {p.temperature}°C</p>
            </div>
            <div class="text-xs text-gray-500 font-mono">[{p.galaxy}:{p.system}:{p.position}]</div>
          </div>

          <!-- Resource bars -->
          <div class="space-y-2 mb-3">
            <div>
              <div class="flex justify-between text-xs mb-0.5">
                <span class="text-red-400">Metal</span>
                <span class="text-gray-400">{fmtNum(p.current_metal)} / {fmtNum(p.metal_storage)}</span>
                <span class="text-green-400">+{fmtNum(p.metal_per_hour)}/h</span>
              </div>
              <div class="w-full bg-[#0a0e1a] rounded-full h-2 overflow-hidden">
                <div class="bg-red-600 h-full rounded-full transition-all" style="width: {Math.min(100, (p.current_metal / p.metal_storage) * 100)}%"></div>
              </div>
            </div>
            <div>
              <div class="flex justify-between text-xs mb-0.5">
                <span class="text-blue-400">Crystal</span>
                <span class="text-gray-400">{fmtNum(p.current_crystal)} / {fmtNum(p.crystal_storage)}</span>
                <span class="text-green-400">+{fmtNum(p.crystal_per_hour)}/h</span>
              </div>
              <div class="w-full bg-[#0a0e1a] rounded-full h-2 overflow-hidden">
                <div class="bg-blue-600 h-full rounded-full transition-all" style="width: {Math.min(100, (p.current_crystal / p.crystal_storage) * 100)}%"></div>
              </div>
            </div>
            <div>
              <div class="flex justify-between text-xs mb-0.5">
                <span class="text-green-400">Gas</span>
                <span class="text-gray-400">{fmtNum(p.current_gas)} / {fmtNum(p.gas_storage)}</span>
                <span class="text-green-400">+{fmtNum(p.gas_per_hour)}/h</span>
              </div>
              <div class="w-full bg-[#0a0e1a] rounded-full h-2 overflow-hidden">
                <div class="bg-green-600 h-full rounded-full transition-all" style="width: {Math.min(100, (p.current_gas / p.gas_storage) * 100)}%"></div>
              </div>
            </div>
          </div>

          <!-- Fleet on this planet -->
          {#if p.fleet_inventory}
            {@const inv = p.fleet_inventory as any}
            {@const planetCombat = combatTypes.reduce((s: number, st: any) => s + (inv[st.key] || 0), 0)}
            {@const planetCivil = civilTypes.reduce((s: number, st: any) => s + (inv[st.key] || 0), 0)}
            <div class="mb-3">
              <div class="flex items-center gap-3 text-[10px] text-gray-500 mb-1.5">
                <span class="text-red-400">⚔ {fmtNum(planetCombat)}</span>
                <span class="text-cyan-400">🚀 {fmtNum(planetCivil)}</span>
                <span class="text-gray-600">{fmtNum(planetCombat + planetCivil)} total</span>
              </div>
              <div class="flex flex-wrap gap-1">
                {#each SHIP_TYPES as st}
                  {@const c = inv[st.key] || 0}
                  {#if c > 0}
                    <span class="text-[9px] bg-[#0a0e1a] border border-[#1e293b] rounded px-1.5 py-0.5 text-gray-400">{st.label} {fmtNum(c)}</span>
                  {/if}
                {/each}
              </div>
            </div>
          {/if}

          <!-- Production bar chart (horizontal bars comparing all planets) -->
          <div class="space-y-1.5">
            <div class="flex items-center gap-2 text-[10px]">
              <span class="text-red-400 w-10 shrink-0">Metal</span>
              <div class="flex-1 bg-[#0a0e1a] rounded-full h-3 overflow-hidden">
                <div class="bg-red-600 h-full rounded-full" style="width: {(p.metal_per_hour / maxMetalPH) * 100}%"></div>
              </div>
              <span class="text-gray-500 w-16 text-right font-mono">{fmtNum(p.metal_per_hour)}/h</span>
            </div>
            <div class="flex items-center gap-2 text-[10px]">
              <span class="text-blue-400 w-10 shrink-0">Crystal</span>
              <div class="flex-1 bg-[#0a0e1a] rounded-full h-3 overflow-hidden">
                <div class="bg-blue-600 h-full rounded-full" style="width: {(p.crystal_per_hour / maxCrystalPH) * 100}%"></div>
              </div>
              <span class="text-gray-500 w-16 text-right font-mono">{fmtNum(p.crystal_per_hour)}/h</span>
            </div>
            <div class="flex items-center gap-2 text-[10px]">
              <span class="text-green-400 w-10 shrink-0">Gas</span>
              <div class="flex-1 bg-[#0a0e1a] rounded-full h-3 overflow-hidden">
                <div class="bg-green-600 h-full rounded-full" style="width: {(p.gas_per_hour / maxGasPH) * 100}%"></div>
              </div>
              <span class="text-gray-500 w-16 text-right font-mono">{fmtNum(p.gas_per_hour)}/h</span>
            </div>

          </div>

          <!-- Summary line -->
          <div class="flex gap-3 mt-3 text-[10px] text-gray-600 border-t border-[#1e293b] pt-2">
            <span>Energy: {fmtNum(p.energy_produced)} / {fmtNum(p.energy_consumed)}</span>
            <span>Storage: {fmtNum(p.metal_storage)} / {fmtNum(p.crystal_storage)} / {fmtNum(p.gas_storage)}</span>
          </div>
        </div>
      {/each}
    </div>
  {/if}
</div>