<script lang="ts">
  import { api, type PlanetView, type BuildingInfo } from './api'
  import ResearchPanel from './ResearchPanel.svelte'
  import ShipyardPanel from './ShipyardPanel.svelte'
  import { buildingLabel as buildLabel } from './buildingLabels'
  import { fmtNum as formatNum, fmtTime as formatTime, fmtRate as formatRate } from './format'
  import { estimateBuildTime } from './estimates'
  import { lockBody } from './timers.svelte'

  let { planet }: { planet: PlanetView } = $props()

  let currentMetal = $state(planet.current_metal)
  let currentCrystal = $state(planet.current_crystal)
  let currentGas = $state(planet.current_gas)
  let pv = $state(planet)
  let tab = $state<'buildings' | 'research' | 'shipyard'>('buildings')
  let selectedBuilding = $state<string | null>(null)
  let buildingInfo = $state<BuildingInfo | null>(null)
  let buildError = $state('')
  let buildLoading = $state(false)
  let constructionRemaining = $state(0)

  function resourceBarPct(current: number, max: number): number {
    if (max <= 0) return 0
    return Math.min(100, (current / max) * 100)
  }

  function energyPct(): number {
    if (pv.energy_produced <= 0) return 0
    return Math.min(100, (pv.energy_consumed / pv.energy_produced) * 100)
  }

  function canAfford(cost: { metal: number; crystal: number; gas: number }): boolean {
    return currentMetal >= cost.metal && currentCrystal >= cost.crystal && currentGas >= cost.gas
  }

  function hasPrereqs(): boolean {
    if (!buildingInfo?.prereqs) return true
    for (const prereq of Object.keys(buildingInfo.prereqs)) {
      const level = (pv as any)[prereq] ?? (pv as any)[prereq + '_level'] ?? 0
      if (level < (buildingInfo.prereqs[prereq] || 0)) return false
    }
    return true
  }

  // Sync local state from prop (updated by App-level polling)
  $effect(() => {
    pv = planet
    currentMetal = planet.current_metal
    currentCrystal = planet.current_crystal
    currentGas = planet.current_gas
  })

  // Live construction countdown (pure client-side)
  $effect(() => {
    const end = pv.construction_end ? new Date(pv.construction_end).getTime() : null
    if (!end || end <= Date.now()) { constructionRemaining = 0; return }
    const tick = setInterval(() => {
      const rem = Math.max(0, Math.floor((end - Date.now()) / 1000))
      constructionRemaining = rem
      if (rem <= 0) clearInterval(tick)
    }, 1000)
    return () => clearInterval(tick)
  })

  $effect(() => {
    if (selectedBuilding) {
      buildError = ''
      buildingInfo = null
      api.getBuildingInfo(pv.id, selectedBuilding).then((info) => {
        buildingInfo = info
      }).catch(() => {
        buildingInfo = null
      })
    }
  })

  async function handleBuild() {
    if (!buildingInfo) return
    buildLoading = true
    buildError = ''
    try {
      await api.buildBuilding(pv.id, buildingInfo.name, buildingInfo.target_level)
      pv = await api.getPlanet(pv.id)
      currentMetal = pv.current_metal
      currentCrystal = pv.current_crystal
      currentGas = pv.current_gas
      // Reload building info so user can queue next level
      api.getBuildingInfo(pv.id, buildingInfo.name).then(info => {
        buildingInfo = info
      }).catch(() => {})
    } catch (e: any) {
      buildError = e.message
    } finally {
      buildLoading = false
    }
  }

  async function cancelQueueItem(building: string) {
    try {
      await api.cancelBuilding(pv.id, building)
      pv = await api.getPlanet(pv.id)
      currentMetal = pv.current_metal
      currentCrystal = pv.current_crystal
      currentGas = pv.current_gas
      if (buildingInfo && building === buildingInfo.name) {
        selectedBuilding = null
        buildingInfo = null
      }
    } catch (e: any) {
      buildError = e.message
    }
  }

  function buildingDesc(b: string): string {
    const descs: Record<string, string> = {
      metal_mine: 'Extracts Metal — the primary construction resource required in the highest volume. Production consumes energy.',
      crystal_mine: 'Mines Crystal — a key component for electronics, ship systems, and advanced technologies.',
      gas_mine: 'Extracts Gas — used as both construction material and consumable fuel for fleet deployment.',
      solar_plant: 'Converts solar energy to power your colony. Energy is consumed by mines and other facilities.',
      fusion_reactor: 'Advanced power generation that consumes Gas. Efficiency scales with Energy Tech level.',
      robotics: 'Increases construction speed of all buildings. Higher levels exponentially reduce build times.',
      nanite_factory: 'Halves build, ship, and defense production times. Prohibitively expensive but transformative.',
      shipyard: 'Required to construct ships and defenses. Higher levels unlock advanced ship classes.',
      research_lab: 'Enables scientific research. Required to unlock and upgrade all technologies.',
      galactonite_research_center: 'Manages Galactonite equipment for endgame vertical progression. Every 2 levels adds 1 equipment slot.',
      radar: 'Provides intel on incoming fleets. Higher levels reveal ship types and give up to 15 min advance warning.',
      alliance_depot: 'Required to join or create an alliance. Level determines fuel reserve capacity.',
      metal_storage: 'Increases the maximum amount of Metal your colony can hold.',
      crystal_storage: 'Increases the maximum amount of Crystal your colony can hold.',
      gas_storage: 'Increases the maximum amount of Gas your colony can hold.',
      moon_base: 'Foundation of your lunar colony. Required for all other moon buildings. Each level adds 2 building slots.',
      robotics_moon: 'Increases construction speed of moon buildings. Planetary Robotics level affects moon build times.',
      shipyard_moon: 'Enables ship and defense construction on your moon.',
      pioneer_lab: 'Produces the Iron Behemoth — a powerful siege unit for late-game combat.',
      eu_x_radar: 'Scans any system for fleet activity. Invisible to the target. Costs 50,000 Gas per use.',
      wormhole_generator: 'Instantly teleports fleets between synchronized moons. Zero gas cost, 1-hour cooldown per pair.',
    }
    return descs[b] || ''
  }

  const buildings = [
    { key: 'metal_mine', icon: '⛏' },
    { key: 'crystal_mine', icon: '💎' },
    { key: 'gas_mine', icon: '🔥' },
    { key: 'solar_plant', icon: '☀' },
    { key: 'fusion_reactor', icon: '⚡' },
    { key: 'robotics', icon: '⚙' },
    { key: 'nanite_factory', icon: '🔬' },
    { key: 'shipyard', icon: '🚀' },
    { key: 'research_lab', icon: '🧪' },
    { key: 'galactonite_research_center', icon: '💠' },
    { key: 'radar', icon: '📡' },
    { key: 'alliance_depot', icon: '🏛' },
    { key: 'metal_storage', icon: '📦' },
    { key: 'crystal_storage', icon: '📦' },
    { key: 'gas_storage', icon: '📦' },
  ]

  const moonBuildings = [
    { key: 'moon_base', icon: '🌙' },
    { key: 'robotics_moon', icon: '⚙' },
    { key: 'shipyard_moon', icon: '🚀' },
    { key: 'pioneer_lab', icon: '🧪' },
    { key: 'eu_x_radar', icon: '📡' },
    { key: 'wormhole_generator', icon: '🌀' },
  ]

  function buildingLevel(b: string): number {
    return (pv as any)[b + '_level'] ?? 0
  }

  const buildingInProgress = $derived(pv.const_building && pv.construction_end ? pv.const_building : null)
</script>

<div class="min-h-screen bg-[#0a0e1a]">
  <header class="bg-[#111827] border-b border-[#1e293b] px-6 py-3 flex items-center justify-between">
    <div class="flex items-center gap-3">
      <h1 class="text-xl font-bold text-white">Galaxy Empire</h1>
      <span class="text-gray-500 text-sm">[1:1:3]</span>
    </div>
    <div class="flex items-center gap-4">
      <span class="text-gray-400 text-sm">{pv.planet_size} slots</span>
    </div>
  </header>

  <div class="max-w-4xl mx-auto p-4 space-y-4">
    <!-- Resource Bars -->
    <div class="bg-[#111827] rounded-lg border border-[#1e293b] p-4 space-y-3">
      <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">Resources</h2>

      <div>
        <div class="flex justify-between text-sm mb-1">
          <span class="text-red-400 font-medium">Metal</span>
          <span class="text-gray-300">{formatNum(currentMetal)} / {formatNum(pv.metal_storage)}</span>
          <span class="text-green-400 text-xs">{formatRate(pv.metal_per_hour)}</span>
        </div>
        <div class="w-full bg-[#0a0e1a] rounded-full h-3 overflow-hidden">
          <div class="bg-red-600 h-full rounded-full transition-all duration-1000" style="width: {resourceBarPct(currentMetal, pv.metal_storage)}%"></div>
        </div>
      </div>

      <div>
        <div class="flex justify-between text-sm mb-1">
          <span class="text-blue-400 font-medium">Crystal</span>
          <span class="text-gray-300">{formatNum(currentCrystal)} / {formatNum(pv.crystal_storage)}</span>
          <span class="text-green-400 text-xs">{formatRate(pv.crystal_per_hour)}</span>
        </div>
        <div class="w-full bg-[#0a0e1a] rounded-full h-3 overflow-hidden">
          <div class="bg-blue-600 h-full rounded-full transition-all duration-1000" style="width: {resourceBarPct(currentCrystal, pv.crystal_storage)}%"></div>
        </div>
      </div>

      <div>
        <div class="flex justify-between text-sm mb-1">
          <span class="text-green-400 font-medium">Gas</span>
          <span class="text-gray-300">{formatNum(currentGas)} / {formatNum(pv.gas_storage)}</span>
          <span class="text-green-400 text-xs">{formatRate(pv.gas_per_hour)}</span>
        </div>
        <div class="w-full bg-[#0a0e1a] rounded-full h-3 overflow-hidden">
          <div class="bg-green-600 h-full rounded-full transition-all duration-1000" style="width: {resourceBarPct(currentGas, pv.gas_storage)}%"></div>
        </div>
      </div>

      <div>
        <div class="flex justify-between text-sm mb-1">
          <span class="text-yellow-400 font-medium">Energy</span>
          <span class="text-gray-300">{formatNum(pv.energy_produced)} / {formatNum(pv.energy_consumed)}</span>
          <span class="text-yellow-400 text-xs">{formatRate(pv.energy_produced - pv.energy_consumed)}</span>
        </div>
        <div class="w-full bg-[#0a0e1a] rounded-full h-3 overflow-hidden">
          <div class="bg-yellow-500 h-full rounded-full transition-all duration-1000" style="width: {energyPct()}%"></div>
        </div>
      </div>
    </div>

    <!-- Tab Bar -->
    <div class="flex gap-1 bg-[#111827] rounded-lg border border-[#1e293b] p-1">
      <button
        onclick={() => tab = 'buildings'}
        class="flex-1 py-2 text-sm font-medium rounded transition-colors
          {tab === 'buildings' ? 'bg-blue-600 text-white' : 'text-gray-400 hover:text-white'}"
      >Buildings</button>
      <button
        onclick={() => tab = 'research'}
        class="flex-1 py-2 text-sm font-medium rounded transition-colors
          {tab === 'research' ? 'bg-blue-600 text-white' : 'text-gray-400 hover:text-white'}"
      >Research</button>
      <button
        onclick={() => tab = 'shipyard'}
        class="flex-1 py-2 text-sm font-medium rounded transition-colors
          {tab === 'shipyard' ? 'bg-blue-600 text-white' : 'text-gray-400 hover:text-white'}"
      >Shipyard</button>
    </div>

    {#if tab === 'buildings'}
      <!-- Buildings Grid -->
      <div class="bg-[#111827] rounded-lg border border-[#1e293b] p-4">
        <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide mb-3">Buildings</h2>
        <div class="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-2">
          {#each pv.is_moon ? moonBuildings : buildings as b}
            {@const level = buildingLevel(b.key)}
            {@const isBuilding = buildingInProgress === b.key}
            <button
              onclick={() => { selectedBuilding = b.key; lockBody(true) }}
              class="bg-[#0a0e1a] border {selectedBuilding === b.key ? 'border-blue-500' : isBuilding ? 'border-yellow-500/50' : 'border-[#1e293b]'} rounded p-3 hover:border-blue-500/50 transition-colors text-left relative group"
            >
              {#if isBuilding}
                <div class="absolute top-1 right-1 w-2 h-2 bg-yellow-400 rounded-full animate-pulse"></div>
              {/if}
              <div class="text-lg mb-1">{b.icon}</div>
              <div class="text-xs text-gray-400">{buildLabel(b.key)}</div>
              <div class="text-sm font-mono text-white">Lv. {level}</div>
              <div class="absolute bottom-1 right-1 text-[9px] text-gray-600 opacity-0 group-hover:opacity-100 transition-opacity">ℹ</div>
            </button>
          {/each}
        </div>
      </div>
    {:else if tab === 'research'}
      <!-- Research -->
      <div class="bg-[#111827] rounded-lg border border-[#1e293b] p-4">
        <ResearchPanel {planet} />
      </div>
    {:else if tab === 'shipyard'}
      <div class="bg-[#111827] rounded-lg border border-[#1e293b] p-4">
        <ShipyardPanel planetId={pv.id} />
      </div>
    {/if}

    <!-- Build Dialog -->
    {#if selectedBuilding}
      {@const level = buildingLevel(selectedBuilding)}
      <div class="fixed inset-0 bg-black/60 flex items-center justify-center z-50" onclick={() => { selectedBuilding = null; lockBody(false) }}>
        <div class="bg-[#111827] border border-[#1e293b] rounded-lg p-6 w-96 max-w-full" onclick={(e) => e.stopPropagation()}>
          <div class="flex justify-between items-start mb-4">
            <div>
              <h3 class="text-lg font-bold text-white">{buildLabel(selectedBuilding)}</h3>
              <p class="text-sm text-gray-500">Lv. {level} → Lv. {level + 1}</p>
            </div>
            <button onclick={() => { selectedBuilding = null; lockBody(false) }} class="text-gray-500 hover:text-white text-lg">&times;</button>
          </div>

          {#if buildingInfo}
            <div class="space-y-3">
              <!-- Description -->
              {#if buildingDesc(selectedBuilding)}
                <p class="text-xs text-gray-500 leading-relaxed">{buildingDesc(selectedBuilding)}</p>
              {/if}

              <!-- Effect comparison -->
              {#if buildingInfo.current_effect || buildingInfo.upgrade_effect}
                <div class="bg-[#0a0e1a] rounded border border-[#1e293b] p-3">
                  <p class="text-xs text-gray-400 uppercase tracking-wide mb-2">Effect</p>
                  <div class="flex items-center gap-3 text-sm">
                    <div class="flex-1 text-center">
                      <p class="text-gray-500 text-xs mb-1">Current</p>
                      <p class="text-white font-medium">{buildingInfo.current_effect || '—'}</p>
                    </div>
                    <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="#60a5fa" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="9 18 15 12 9 6"/></svg>
                    <div class="flex-1 text-center">
                      <p class="text-green-400 text-xs mb-1">After Upgrade</p>
                      <p class="text-green-300 font-medium">{buildingInfo.upgrade_effect || '—'}</p>
                    </div>
                  </div>
                </div>
              {/if}

              <!-- Cost -->
              <div>
                <p class="text-xs text-gray-400 uppercase tracking-wide mb-1">Cost</p>
                <div class="flex gap-3 text-sm">
                  <span class="text-red-400" class:opacity-50={!canAfford(buildingInfo.cost)}>{formatNum(buildingInfo.cost.metal)} Metal</span>
                  <span class="text-blue-400" class:opacity-50={!canAfford(buildingInfo.cost)}>{formatNum(buildingInfo.cost.crystal)} Crystal</span>
                  <span class="text-green-400" class:opacity-50={!canAfford(buildingInfo.cost)}>{formatNum(buildingInfo.cost.gas)} Gas</span>
                </div>
              </div>

              <!-- Time -->
              <div>
                <p class="text-xs text-gray-400 uppercase tracking-wide mb-1">Time</p>
                <p class="text-sm text-white">{formatTime(buildingInfo.time_seconds)}</p>
              </div>

              <!-- Prerequisites -->
              {#if buildingInfo.prereqs && !hasPrereqs()}
                <div>
                  <p class="text-xs text-gray-400 uppercase tracking-wide mb-1">Required</p>
                  {#each Object.entries(buildingInfo.prereqs) as [prereq, reqLevel]}
                    {@const curLevel = (pv as any)[prereq] ?? (pv as any)[prereq + '_level'] ?? 0}
                    <p class="text-xs {curLevel >= reqLevel ? 'text-green-400' : 'text-red-400'}">
                      {buildLabel(prereq)} Lv.{reqLevel} {curLevel >= reqLevel ? '✓' : `(${curLevel})`}
                    </p>
                  {/each}
                </div>
              {/if}

              {#if buildError}
                <div class="bg-red-900/50 text-red-400 p-2 rounded text-sm">{buildError}</div>
              {/if}

              <button
                onclick={handleBuild}
                disabled={!canAfford(buildingInfo.cost) || !hasPrereqs() || buildLoading}
                class="w-full bg-blue-600 hover:bg-blue-700 disabled:bg-blue-800/50 text-white py-2 rounded font-medium transition-colors mb-2"
              >
                {buildLoading ? 'Building...' : canAfford(buildingInfo.cost) && hasPrereqs() ? 'Upgrade to Lv. ' + (level + 1) : 'Cannot Upgrade'}
              </button>

              <!-- Queue display -->
              <div class="border-t border-[#1e293b] pt-3 max-h-48 overflow-y-auto">
                <div class="flex justify-between items-center mb-2">
                  <p class="text-xs text-gray-400 uppercase tracking-wide">Queue</p>
                  <p class="text-xs text-gray-500">{(pv.construction_queue ?? []).length} / {1 + Math.floor(pv.robotics_level / 2)} slots | {formatTime((pv.construction_queue ?? []).reduce((sum, i) => sum + estimateBuildTime(i.building, i.target_level, pv.robotics_level), 0))}</p>
                </div>

                <!-- Active construction -->
                {#if buildingInProgress}
                  <div class="flex items-center justify-between bg-[#0a0e1a] rounded px-3 py-1.5 mb-1.5">
                    <div class="flex-1">
                      <p class="text-sm text-yellow-400">{buildLabel(buildingInProgress)}</p>
                      <p class="text-xs text-gray-500">Lv. {pv[buildingInProgress + '_level']} → Lv. {pv[buildingInProgress + '_level'] + 1}</p>
                    </div>
                    <button
                      onclick={() => cancelQueueItem(buildingInProgress)}
                      class="text-red-500 hover:text-red-400 text-xs px-2 py-1 rounded hover:bg-red-900/30"
                    >Cancel</button>
                  </div>
                {:else}
                  <p class="text-gray-500 text-xs text-center py-4">No active construction</p>
                {/if}

                <!-- Queued items -->
                {#each (pv.construction_queue ?? []) as item}
                  {@const estSec = estimateBuildTime(item.building, item.target_level, pv.robotics_level)}
                  <div class="flex items-center justify-between bg-[#0a0e1a] rounded px-3 py-1.5 mb-1">
                    <div class="flex-1 min-w-0">
                      <p class="text-sm text-gray-300">{buildLabel(item.building)}</p>
                      <p class="text-xs text-gray-500">Lv. {item.target_level - 1} → Lv. {item.target_level}</p>
                      <div class="w-full bg-black/50 rounded-full h-1 mt-1 overflow-hidden">
                        <div class="bg-yellow-500 h-full rounded-full" style="width: 0%"></div>
                      </div>
                      <p class="text-[10px] text-gray-600 mt-0.5">≈{formatTime(estSec)}</p>
                    </div>
                    <button
                      onclick={() => cancelQueueItem(item.building)}
                      class="text-red-500 hover:text-red-400 text-xs px-2 py-1 rounded hover:bg-red-900/30 shrink-0"
                    >Cancel</button>
                  </div>
                {/each}
              </div>
            </div>
          {:else}
            <div class="text-gray-500 text-center py-4">Loading...</div>
          {/if}
        </div>
      </div>
    {/if}

    <!-- Planet Info -->
    <div class="bg-[#111827] rounded-lg border border-[#1e293b] p-4 flex justify-between text-sm text-gray-500">
      <span>Size: {pv.planet_size}</span>
      <span>Temperature: {pv.temperature}°C</span>
{#if pv.is_moon}
        {@const moonSlotsMax = 2 * pv.moon_base_level + 1}
        {@const moonSlotsUsed = pv.robotics_moon_level + pv.shipyard_moon_level + pv.pioneer_lab_level + pv.eu_x_radar_level + pv.wormhole_generator_level}
        <span>Slots: {moonSlotsUsed} / {moonSlotsMax}</span>
        <span>Moon Base Lv.{pv.moon_base_level}</span>
      {:else}
        <span title="Unique building types placed / Maximum buildings">Buildings: {pv.used_slots} / 178 max</span>
      {/if}
    </div>
  </div>
</div>
