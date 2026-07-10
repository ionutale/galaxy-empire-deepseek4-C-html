<script lang="ts">
  import { api, type SystemPosition, type PlanetSummary, type PlanetView } from './api'
  import { SHIP_TYPES } from './shipTypes'
  import { fmtNum } from './format'
  import { lockBody } from './timers.svelte'

  let { galaxy = $bindable(1), system = $bindable(1) }: { galaxy?: number; system?: number } = $props()

  let positions = $state<SystemPosition[]>([])
  let planets = $state<PlanetSummary[]>([])
  let planetViews = $state<Record<string, PlanetView>>({})
  let error = $state('')
  let selectedPos = $state<SystemPosition | null>(null)
  let sendError = $state('')
  let sending = $state(false)

  // Fleet composition state
  let originPlanetId = $state('')
  let missionType = $state('attack')
  let shipQuantities = $state<Record<string, number>>({})
  let cargoMetal = $state(0)
  let cargoCrystal = $state(0)
  let cargoGas = $state(0)
  let estimate = $state<{ flight_time_seconds: number; arrival_time: string; return_time: string; distance: number; fuel_cost: number } | null>(null)
  let estimateLoading = $state(false)
  let shipFilter = $state<'all' | 'combat' | 'civil' | 'selected'>('all')
  let fleetSpeed = $state(100)
  let showTelemetry = $state(false)
  let presets = $state<{ id: string; name: string; ships: Record<string, number> }[]>([])
  let presetName = $state('')
  let showSavePreset = $state(false)

  function loadPresets() {
    api.getPresets().then(p => presets = p).catch(() => {})
  }
  $effect(loadPresets)

  const MISSION_TYPES = [
    { id: 'attack', label: 'Attack', icon: '⚔', desc: 'Combat', requiresTarget: true },
    { id: 'transport', label: 'Transport', icon: '📦', desc: 'Deliver resources', requiresTarget: true },
    { id: 'spy', label: 'Spy', icon: '👁', desc: 'Gather intel', requiresTarget: true },
    { id: 'recycle', label: 'Recycle', icon: '♻', desc: 'Collect debris', requiresTarget: true },
    { id: 'explore', label: 'Explore', icon: '✦', desc: 'Probe the Nebula', requiresTarget: false },
    { id: 'colonize', label: 'Colonize', icon: '🚀', desc: 'Settle new world', requiresTarget: true },
    { id: 'defense', label: 'Defense', icon: '🛡', desc: 'Protect ally', requiresTarget: true },
  ]

  function load() {
    api.getSystem(galaxy, system).then(p => {
      positions = p
      error = ''
    }).catch(e => error = e.message)
    api.getPlanets().then(p => {
      planets = p
    }).catch(() => {})
  }

  $effect(load)

  $effect(() => { lockBody(!!selectedPos) })

  function posClass(p: SystemPosition): string {
    if (p.owner_name) return 'border-emerald-600/60 bg-emerald-900/20'
    if (p.is_npc) return 'border-red-600/60 bg-red-900/20'
    if (p.is_nebula) return 'border-purple-600/60 bg-purple-900/20'
    if (p.is_empty) return 'border-gray-700 bg-gray-800/20'
    return 'border-gray-700 bg-gray-800/20'
  }

  function posLabel(p: SystemPosition): string {
    if (p.owner_name) return p.owner_name.slice(0, 8)
    if (p.is_npc) return p.npc_tier ?? 'NPC'
    if (p.is_nebula) return 'Nebula'
    return '—'
  }

  function canSendFleetTo(p: SystemPosition): boolean {
    if (p.position >= 17 && p.position <= 21) return missionType === 'attack'
    if (p.is_nebula) return missionType === 'explore'
    return true
  }

  function openFleetModal(pos: SystemPosition) {
    selectedPos = pos
    sendError = ''
    estimate = null
    shipQuantities = {}
    cargoMetal = 0
    cargoCrystal = 0
    cargoGas = 0
    missionType = pos.is_empty && pos.position >= 1 && pos.position <= 15 ? 'colonize' : 'attack'
    originPlanetId = planets[0]?.id || ''
    if (originPlanetId && !planetViews[originPlanetId]) {
      loadPlanetView(originPlanetId)
    }
  }

  async function loadPlanetView(planetId: string) {
    try {
      const view = await api.getPlanet(planetId)
      planetViews = { ...planetViews, [planetId]: view }
      // Init ship quantities for this planet
      if (view.fleet_inventory) {
        const init: Record<string, number> = {}
        for (const s of SHIP_TYPES) {
          init[s.key] = 0
        }
        shipQuantities = init
      }
    } catch {}
  }

  function handleOriginChange(id: string) {
    originPlanetId = id
    if (id && !planetViews[id]) {
      loadPlanetView(id)
    } else {
      const init: Record<string, number> = {}
      for (const s of SHIP_TYPES) {
        init[s.key] = 0
      }
      shipQuantities = init
    }
  }

  function availableShips(planetId: string): Record<string, number> {
    const view = planetViews[planetId]
    if (!view?.fleet_inventory) return {}
    const fi = view.fleet_inventory as unknown as Record<string, number>
    const result: Record<string, number> = {}
    for (const s of SHIP_TYPES) {
      result[s.key] = fi[s.key] || 0
    }
    return result
  }

  function totalShipsSelected(): number {
    return Object.values(shipQuantities).reduce((a, b) => a + (b || 0), 0)
  }

  function totalCargo(): number {
    const view = planetViews[originPlanetId]
    if (!view?.fleet_inventory) return 0
    const caps: Record<string, number> = {
      small_cargo: 5000, large_cargo: 25000,
      light_fighter: 50, heavy_fighter: 100,
      cruiser: 800, battleship: 1500, dreadnought: 750,
      bomber: 500, destroyer: 2000,
      colony_ship: 7500, recycler: 20000,
      probe: 5, solar_satellite: 0,
      interceptor: 200, iron_behemoth: 3000,
      devourer_interceptor: 800, titanenergetic_frigate: 1500,
      annihilator: 0,
    }
    let total = 0
    for (const [k, v] of Object.entries(shipQuantities)) {
      if (v > 0) total += (caps[k] || 0) * v
    }
    return total
  }

  async function handleSend() {
    if (!selectedPos || !originPlanetId) return
    const activeShips: Record<string, number> = {}
    for (const [k, v] of Object.entries(shipQuantities)) {
      if (v > 0) activeShips[k] = v
    }
    if (Object.keys(activeShips).length === 0) {
      sendError = 'Select at least one ship'
      return
    }
    sending = true
    sendError = ''
    try {
      await api.sendFleet(
        originPlanetId,
        galaxy, system, selectedPos.position,
        missionType,
        activeShips,
        missionType === 'transport' ? cargoMetal : 0,
        missionType === 'transport' ? cargoCrystal : 0,
        missionType === 'transport' ? cargoGas : 0,
      )
      selectedPos = null
    } catch (e: any) {
      sendError = e.message
    } finally {
      sending = false
    }
  }

  $effect(() => {
    if (!selectedPos || !originPlanetId) { estimate = null; return }
    const activeShips: Record<string, number> = {}
    for (const [k, v] of Object.entries(shipQuantities)) {
      if (v > 0) activeShips[k] = v
    }
    if (Object.keys(activeShips).length === 0) { estimate = null; return }
    estimateLoading = true
    api.getFleetEstimate(originPlanetId, galaxy, system, selectedPos.position, activeShips)
      .then(e => estimate = e)
      .catch(() => estimate = null)
      .finally(() => estimateLoading = false)
  })

  function selectAll() {
    const avail = availableShips(originPlanetId)
    const q: Record<string, number> = {}
    for (const s of SHIP_TYPES) {
      q[s.key] = avail[s.key] || 0
    }
    shipQuantities = q
  }

  function clearShips() {
    const q: Record<string, number> = {}
    for (const s of SHIP_TYPES) {
      q[s.key] = 0
    }
    shipQuantities = q
  }

  function clampShipQty(shipKey: string, value: number): number {
    const avail = availableShips(originPlanetId)[shipKey] || 0
    if (!Number.isFinite(value)) return 0
    return Math.max(0, Math.min(avail, Math.floor(value)))
  }

  function setShipQty(shipKey: string, value: number) {
    shipQuantities = { ...shipQuantities, [shipKey]: clampShipQty(shipKey, value) }
  }

  function adjustShipQty(shipKey: string, delta: number) {
    const current = shipQuantities[shipKey] || 0
    setShipQty(shipKey, current + delta)
  }

  function setShipPercent(shipKey: string, ratio: number) {
    const avail = availableShips(originPlanetId)[shipKey] || 0
    setShipQty(shipKey, Math.floor(avail * ratio))
  }

  function filteredShipTypes() {
    const avail = availableShips(originPlanetId)
    return SHIP_TYPES.filter((ship) => {
      const qty = shipQuantities[ship.key] || 0
      if ((avail[ship.key] || 0) === 0 && qty === 0) return false
      if (shipFilter === 'selected') return qty > 0
      if (shipFilter === 'combat') return ship.cat === 'combat'
      if (shipFilter === 'civil') return ship.cat === 'civil'
      return true
    })
  }

  function isRecommended(key: string): boolean {
    switch (missionType) {
      case 'attack': case 'defense':
        return ['light_fighter','heavy_fighter','cruiser','battleship','dreadnought','bomber','destroyer','interceptor','iron_behemoth','devourer_interceptor','titanenergetic_frigate','annihilator'].includes(key)
      case 'transport': return key === 'small_cargo' || key === 'large_cargo'
      case 'spy': return key === 'probe'
      case 'recycle': return key === 'recycler'
      case 'explore': return key === 'recycler' || key === 'probe'
      case 'colonize': return key === 'colony_ship'
      default: return false
    }
  }

  const TEMPLATES: { id: string; label: string; icon: string; apply: (avail: Record<string, number>) => Record<string, number> }[] = [
    { id: 'cargo', label: 'Cargo', icon: '📦', apply: (a) => ({ ...a, probe: 0, solar_satellite: 0, colony_ship: 0, recycler: 0, light_fighter: 0, heavy_fighter: 0, cruiser: 0, battleship: 0, dreadnought: 0, bomber: 0, destroyer: 0, interceptor: 0, iron_behemoth: 0, devourer_interceptor: 0, titanenergetic_frigate: 0, annihilator: 0 }) },
    { id: 'combat', label: 'Combat', icon: '⚔', apply: (a) => ({ small_cargo: 0, large_cargo: 0, colony_ship: 0, recycler: 0, probe: 0, solar_satellite: 0, ...a }) },
    { id: 'explore', label: 'Explore', icon: '🌫', apply: (a) => {
      const r = { ...a }; for (const k of Object.keys(r)) { if (k !== 'recycler' && k !== 'probe') r[k] = 0 }; return r
    }},
    { id: 'spy', label: 'Spy', icon: '👁', apply: () => {
      const q: Record<string, number> = {}; for (const s of SHIP_TYPES) q[s.key] = 0; q.probe = 1; return q
    }},
    { id: 'colonize', label: 'Colony', icon: '🏠', apply: () => {
      const q: Record<string, number> = {}; for (const s of SHIP_TYPES) q[s.key] = 0; q.colony_ship = 1; return q
    }},
    { id: 'recycle', label: 'Recycle', icon: '♻', apply: (a) => {
      const r = { ...a }; for (const k of Object.keys(r)) { if (k !== 'recycler') r[k] = 0 }; return r
    }},
  ]

  function applyTemplate(tpl: typeof TEMPLATES[0]) {
    const avail = availableShips(originPlanetId)
    const result = tpl.apply(avail)
    const capped: Record<string, number> = {}
    for (const s of SHIP_TYPES) {
      const v = result[s.key] || 0
      const max = avail[s.key] || 0
      capped[s.key] = Math.min(v, max)
    }
    shipQuantities = capped
  }

  function missionColor(id: string): string {
    switch (id) {
      case 'attack': return 'text-red-400 border-red-600'
      case 'transport': return 'text-amber-400 border-amber-600'
      case 'spy': return 'text-cyan-400 border-cyan-600'
      case 'recycle': return 'text-green-400 border-green-600'
      case 'explore': return 'text-purple-400 border-purple-600'
      case 'colonize': return 'text-emerald-400 border-emerald-600'
      case 'defense': return 'text-blue-400 border-blue-600'
      default: return 'text-gray-400 border-gray-600'
    }
  }

  const ROWS = 7
  const COLS = 3
</script>

<div class="space-y-3">
  <!-- Galaxy/System Navigation -->
  <div class="flex items-center justify-between">
    <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">Galaxy View</h2>
    <div class="flex items-center gap-2">
      <div class="flex items-center gap-1 text-xs">
        <span class="text-gray-500">G:</span>
        <button onclick={() => { galaxy = Math.max(1, galaxy - 1); load() }} class="px-1.5 py-0.5 rounded bg-[#1e293b] hover:bg-[#334155] text-gray-400 hover:text-white transition-colors" disabled={galaxy <= 1}>◀</button>
        <input type="number" min="1" max="9" bind:value={galaxy}
          onchange={() => load()}
          class="w-10 bg-[#0a0e1a] border border-[#1e293b] text-white text-center rounded py-0.5 text-xs [appearance:textfield] [&::-webkit-inner-spin-button]:appearance-none [&::-webkit-outer-spin-button]:appearance-none" />
        <button onclick={() => { galaxy = Math.min(9, galaxy + 1); load() }} class="px-1.5 py-0.5 rounded bg-[#1e293b] hover:bg-[#334155] text-gray-400 hover:text-white transition-colors" disabled={galaxy >= 9}>▶</button>
      </div>
      <div class="flex items-center gap-1 text-xs">
        <span class="text-gray-500">S:</span>
        <button onclick={() => { system = Math.max(1, system - 1); load() }} class="px-1.5 py-0.5 rounded bg-[#1e293b] hover:bg-[#334155] text-gray-400 hover:text-white transition-colors" disabled={system <= 1}>◀</button>
        <input type="number" min="1" max="499" bind:value={system}
          onchange={() => load()}
          class="w-14 bg-[#0a0e1a] border border-[#1e293b] text-white text-center rounded py-0.5 text-xs [appearance:textfield] [&::-webkit-inner-spin-button]:appearance-none [&::-webkit-outer-spin-button]:appearance-none" />
        <button onclick={() => { system = Math.min(499, system + 1); load() }} class="px-1.5 py-0.5 rounded bg-[#1e293b] hover:bg-[#334155] text-gray-400 hover:text-white transition-colors" disabled={system >= 499}>▶</button>
      </div>
    </div>
  </div>

  {#if error}
    <div class="bg-red-900/50 text-red-400 p-2 rounded text-sm">{error}</div>
  {/if}

  <div class="grid grid-cols-3 gap-1.5">
    {#each positions as pos}
      <button
        onclick={() => openFleetModal(pos)}
        class="border rounded p-2 text-left transition-all duration-150 hover:border-blue-500/60 hover:bg-blue-900/10 {posClass(pos)}"
      >
        <div class="text-[10px] text-gray-500 font-mono">#{pos.position}</div>
        <div class="text-xs font-medium text-white truncate">{posLabel(pos)}</div>
      </button>
    {/each}
  </div>

  {#if selectedPos}
    <!-- Fleet composition modal -->
    <div class="fixed inset-0 bg-black/70 flex items-end sm:items-center justify-center z-50 backdrop-blur-sm" onclick={() => selectedPos = null}>
      <div class="bg-[#111827] border border-[#1e293b] rounded-t-xl sm:rounded-xl w-full sm:w-[520px] sm:max-w-full max-h-[85vh] sm:max-h-[90vh] flex flex-col" onclick={(e) => e.stopPropagation()}>

        <!-- Header -->
        <div class="flex items-center justify-between p-4 sm:p-5 pb-0 shrink-0">
          <div>
            <h3 class="text-lg font-bold text-white">Position #{selectedPos.position}</h3>
            <p class="text-xs text-gray-500 font-mono mt-0.5">[{galaxy}:{system}:{selectedPos.position}]</p>
          </div>
          <div class="flex items-center gap-2">
            <button
              onclick={handleSend}
              disabled={sending || totalShipsSelected() === 0}
              class="text-[11px] px-2.5 py-1 rounded font-medium transition-all
                {missionType === 'attack' ? 'bg-red-700 hover:bg-red-600 disabled:bg-red-900/50'
                : missionType === 'defense' ? 'bg-blue-600 hover:bg-blue-700 disabled:bg-blue-900/50'
                : missionType === 'transport' ? 'bg-amber-700 hover:bg-amber-600 disabled:bg-amber-900/50'
                : missionType === 'spy' ? 'bg-cyan-700 hover:bg-cyan-600 disabled:bg-cyan-900/50'
                : missionType === 'colonize' ? 'bg-emerald-700 hover:bg-emerald-600 disabled:bg-emerald-900/50'
                : missionType === 'recycle' ? 'bg-green-700 hover:bg-green-600 disabled:bg-green-900/50'
                : 'bg-purple-700 hover:bg-purple-600 disabled:bg-purple-900/50'}
                text-white disabled:text-white/40 disabled:cursor-not-allowed"
            >Send</button>
            <button onclick={() => selectedPos = null} class="text-gray-600 hover:text-gray-300 text-xl leading-none">&times;</button>
          </div>
        </div>

        <!-- Scrollable content -->
        <div class="overflow-y-auto flex-1 px-4 sm:px-5 pt-4">
        <!-- Target info -->
        <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-2.5 mb-4 text-xs">
          <div class="flex items-center gap-3">
            <span class="text-gray-500">Target:</span>
            <span class="text-white font-mono">[{selectedPos.position}]</span>
            <span class="text-gray-400">{posLabel(selectedPos)}</span>
            {#if selectedPos.owner_name}
              <span class="text-emerald-400">
                {selectedPos.npc_tier || 'Player'}
              </span>
            {/if}
          </div>
        </div>

        {#if sendError}
          <div class="bg-red-900/50 text-red-400 p-2.5 rounded-lg text-sm mb-4 border border-red-800/50">{sendError}</div>
        {/if}

        <!-- Origin planet selector -->
        <div class="mb-4">
          <label class="text-xs text-gray-500 uppercase tracking-wider block mb-1.5">Origin Planet</label>
          <select
            bind:value={originPlanetId}
            onchange={() => handleOriginChange(originPlanetId)}
            class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs focus:border-blue-500/50 focus:outline-none"
          >
            <option value="">Select origin...</option>
            {#each planets as p, i}
              <option value={p.id}>Planet {i + 1}</option>
            {/each}
          </select>
        </div>

        {#if originPlanetId}
          {#key originPlanetId}
            <!-- Mission type selector -->
            <div class="mb-4">
              <label class="text-xs text-gray-500 uppercase tracking-wider block mb-1.5">Mission Type</label>
              <div class="grid grid-cols-3 gap-1.5">
                {#each MISSION_TYPES as mt}
                    <button
                      onclick={() => missionType = mt.id}
                      class="border rounded-lg px-2.5 py-1.5 text-xs font-medium transition-all
                        {missionType === mt.id ? 'bg-opacity-20 border-2 ' + missionColor(mt.id) : 'border-gray-700 text-gray-500 hover:border-gray-500 bg-[#0a0e1a]'}
                        {missionType === mt.id ? missionColor(mt.id).replace('text-', 'bg-').replace('border-', 'border-') + '/20 ' + missionColor(mt.id) : ''}"
                    >
                      <div class="text-sm">{mt.icon}</div>
                      <div class="mt-0.5">{mt.label}</div>
                    </button>
                {/each}
              </div>
            </div>

            <!-- Ship quantities -->
            <div class="mb-3">
              <div class="flex items-center justify-between mb-1.5">
                <label class="text-xs text-gray-500 uppercase tracking-wider">Ships</label>
                <div class="flex items-center gap-1.5">
                  {#each TEMPLATES as tpl}
                    <button
                      onclick={() => applyTemplate(tpl)}
                      class="text-[10px] px-1.5 py-0.5 rounded bg-[#1e293b] hover:bg-[#334155] text-gray-400 hover:text-white transition-colors"
                      title={tpl.label}
                    >{tpl.icon}</button>
                  {/each}
                  <span class="text-gray-700 text-[10px]">|</span>
                  <button onclick={selectAll} class="text-[10px] text-blue-400 hover:text-blue-300 uppercase tracking-wider">Max</button>
                  <button onclick={clearShips} class="text-[10px] text-gray-500 hover:text-gray-400 uppercase tracking-wider">Clear</button>
                </div>
              </div>
              <div class="mb-2 flex flex-wrap gap-1.5">
                <button
                  onclick={() => shipFilter = 'all'}
                  class="px-2 py-1 rounded-md border text-[11px] transition-colors {shipFilter === 'all' ? 'border-blue-500/50 text-blue-300 bg-blue-900/20' : 'border-[#243047] text-gray-400 hover:text-gray-200'}"
                >All</button>
                <button
                  onclick={() => shipFilter = 'combat'}
                  class="px-2 py-1 rounded-md border text-[11px] transition-colors {shipFilter === 'combat' ? 'border-red-500/50 text-red-300 bg-red-900/20' : 'border-[#243047] text-gray-400 hover:text-gray-200'}"
                >Combat</button>
                <button
                  onclick={() => shipFilter = 'civil'}
                  class="px-2 py-1 rounded-md border text-[11px] transition-colors {shipFilter === 'civil' ? 'border-cyan-500/50 text-cyan-300 bg-cyan-900/20' : 'border-[#243047] text-gray-400 hover:text-gray-200'}"
                >Utility</button>
                <button
                  onclick={() => shipFilter = 'selected'}
                  class="px-2 py-1 rounded-md border text-[11px] transition-colors {shipFilter === 'selected' ? 'border-emerald-500/50 text-emerald-300 bg-emerald-900/20' : 'border-[#243047] text-gray-400 hover:text-gray-200'}"
                >Selected</button>
              </div>
                  <div class="space-y-1.5">
                {#each filteredShipTypes() as ship}
                  {@const avail = availableShips(originPlanetId)[ship.key] || 0}
                  {@const selected = shipQuantities[ship.key] || 0}
                  <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg px-2.5 py-2 {selected > 0 ? 'border-[#334155]' : ''}">
                    <div class="flex items-center justify-between mb-1.5">
                      <div class="flex items-center gap-1.5 min-w-0">
                        <span class="text-xs font-medium text-gray-200 truncate">{ship.label}</span>
                        {#if isRecommended(ship.key)}
                          <span class="text-[8px] px-1 py-0.5 rounded bg-cyan-900/30 text-cyan-400 font-bold">REC</span>
                        {/if}
                      </div>
                      <div class="flex items-center gap-1.5">
                        <span class="text-[10px] font-mono text-gray-400">{fmtNum(selected)}</span>
                        <span class="text-[9px] text-gray-600">/</span>
                        <span class="text-[10px] font-mono text-gray-500">{fmtNum(avail)}</span>
                      </div>
                    </div>

                    <div class="w-full h-1 rounded-full bg-[#0f1729] overflow-hidden mb-2">
                      <div
                        class="h-full transition-all duration-200 {ship.cat === 'combat' ? 'bg-red-500/70' : 'bg-cyan-500/70'}"
                        style="width: {avail > 0 ? (selected / avail) * 100 : 0}%"
                      ></div>
                    </div>

                    <div class="flex items-center gap-1">
                      <div class="flex items-center bg-[#0d1322] border border-[#243047] rounded-lg overflow-hidden shrink-0">
                        <button onclick={() => adjustShipQty(ship.key, -100000)} class="px-1 py-1 text-gray-500 hover:text-white text-[9px] border-r border-[#243047]" disabled={selected === 0}>-100k</button>
                        <button onclick={() => adjustShipQty(ship.key, -10000)} class="px-1 py-1 text-gray-500 hover:text-white text-[9px] border-r border-[#243047]" disabled={selected === 0}>-10k</button>
                        <input
                          type="number" min="0" max={avail} value={selected}
                          oninput={(e) => setShipQty(ship.key, Number((e.currentTarget as HTMLInputElement).value || 0))}
                          class="w-12 bg-transparent text-white text-center text-xs [appearance:textfield] [&::-webkit-inner-spin-button]:appearance-none [&::-webkit-outer-spin-button]:appearance-none"
                        />
                        <button onclick={() => adjustShipQty(ship.key, 10000)} class="px-1 py-1 text-gray-500 hover:text-white text-[9px] border-l border-[#243047]" disabled={selected >= avail}>+10k</button>
                        <button onclick={() => adjustShipQty(ship.key, 100000)} class="px-1 py-1 text-gray-500 hover:text-white text-[9px] border-l border-[#243047]" disabled={selected >= avail}>+100k</button>
                      </div>
                      <div class="flex gap-0.5 ml-auto">
                        <button onclick={() => setShipPercent(ship.key, 0.5)} class="px-1.5 py-1 rounded text-[9px] text-gray-500 hover:text-gray-200 bg-[#111b2f]">½</button>
                        <button onclick={() => setShipQty(ship.key, avail)} class="px-1.5 py-1 rounded text-[9px] text-blue-400 hover:text-blue-300 bg-[#1b2c46]">MAX</button>
                      </div>
                    </div>
                  </div>
                {/each}

                {#if filteredShipTypes().length === 0}
                  <div class="text-xs text-gray-500 bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 text-center">
                    No ships match this filter.
                  </div>
                {/if}
              </div>
            </div>

            <!-- Summary row -->
            <div class="flex items-center gap-3 mb-2 text-xs text-gray-500">
              <span>{fmtNum(totalShipsSelected())} ships selected</span>
              <span>{fmtNum(totalCargo())} cargo</span>
            </div>

            <!-- Telemetry + Speed Drawer -->
            <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg mb-3 overflow-hidden">
              <!-- Drawer header -->
              <button onclick={() => showTelemetry = !showTelemetry} class="w-full flex items-center justify-between px-3 py-2 text-xs text-gray-400 hover:text-gray-200 transition-colors">
                <span class="uppercase tracking-wider font-medium">
                  {showTelemetry ? '▼' : '▲'} Flight Telemetry
                </span>
                <span class="font-mono text-[10px]">
                  {#if estimate}
                    {Math.floor(estimate.flight_time_seconds / 3600)}h {Math.floor((estimate.flight_time_seconds % 3600) / 60)}m
                  {:else}
                    --
                  {/if}
                </span>
              </button>

              {#if showTelemetry}
                <div class="px-3 pb-3 space-y-2.5 border-t border-[#1e293b] pt-2.5">
                  <!-- Flight speed -->
                  <div class="space-y-1">
                    <div class="flex justify-between text-[10px]">
                      <span class="text-gray-500">Speed</span>
                      <span class="text-cyan-400 font-mono">{fleetSpeed}%</span>
                    </div>
                    <input type="range" min="10" max="100" step="10" bind:value={fleetSpeed}
                      class="w-full h-1.5 bg-[#1e293b] rounded-lg appearance-none cursor-pointer accent-[#00F0FF]" />
                    <div class="flex justify-between text-[8px] text-gray-600 px-1">
                      <span>10% ECO</span>
                      <span>50%</span>
                      <span>100% MAX</span>
                    </div>
                  </div>

                  {#if estimateLoading}
                    <div class="text-[10px] text-gray-600 italic">Calculating...</div>
                  {:else if estimate}
                    <div class="grid grid-cols-2 gap-x-4 gap-y-1.5 text-[10px]">
                      <span class="text-gray-500">Distance</span>
                      <span class="text-white font-mono text-right">{fmtNum(estimate.distance)} units</span>
                      <span class="text-gray-500">Duration</span>
                      <span class="text-blue-300 font-mono text-right">{Math.floor(estimate.flight_time_seconds / 3600)}h {Math.floor((estimate.flight_time_seconds % 3600) / 60)}m {estimate.flight_time_seconds % 60}s</span>
                      <span class="text-gray-500">Arrival</span>
                      <span class="text-gray-300 font-mono text-right">{new Date(estimate.arrival_time).toLocaleTimeString()}</span>
                      <span class="text-gray-500">Return</span>
                      <span class="text-gray-300 font-mono text-right">{new Date(estimate.return_time).toLocaleTimeString()}</span>
                      <span class="text-gray-500">Fuel</span>
                      <span class="text-amber-400 font-mono text-right">{fmtNum(estimate.fuel_cost)}</span>
                    </div>
                  {:else if totalShipsSelected() > 0}
                    <div class="text-[10px] text-gray-600 italic">Select origin planet to calculate...</div>
                  {/if}
                </div>
              {/if}
            </div>

            <!-- Cargo (transport only) -->
            {#if missionType === 'transport'}
              <div class="mb-4 p-3 bg-[#0a0e1a] border border-amber-800/30 rounded-lg">
                <label class="text-xs text-gray-500 uppercase tracking-wider block mb-2">Cargo</label>
                <div class="grid grid-cols-3 gap-2">
                  <div>
                    <label class="text-[10px] text-gray-600 block mb-0.5">Metal</label>
                    <input type="number" min="0" bind:value={cargoMetal}
                      class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-sm text-center" />
                  </div>
                  <div>
                    <label class="text-[10px] text-gray-600 block mb-0.5">Crystal</label>
                    <input type="number" min="0" bind:value={cargoCrystal}
                      class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-sm text-center" />
                  </div>
                  <div>
                    <label class="text-[10px] text-gray-600 block mb-0.5">Gas</label>
                    <input type="number" min="0" bind:value={cargoGas}
                      class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-sm text-center" />
                  </div>
                </div>
              </div>
            {/if}

          {/key}
        {/if}
        </div> <!-- end scrollable content -->

        <!-- Sticky bottom bar -->
        <div class="shrink-0 border-t border-[#1e293b] bg-[#111827] p-4 sm:p-5 space-y-2">
          <!-- Presets row -->
          <div class="flex items-center gap-1.5 flex-wrap">
            {#each presets as preset}
              <button
                onclick={() => { shipQuantities = { ...preset.ships }; estimate = null }}
                class="text-[10px] px-2 py-1 rounded bg-[#1e293b] hover:bg-[#334155] text-gray-400 hover:text-white transition-colors"
              >📋 {preset.name}</button>
            {/each}
            <button
              onclick={() => showSavePreset = !showSavePreset}
              class="text-[10px] px-2 py-1 rounded bg-blue-900/30 hover:bg-blue-900/50 text-blue-400 transition-colors"
            >+ Save</button>
          </div>

          {#if showSavePreset}
            <div class="flex items-center gap-2">
              <input type="text" bind:value={presetName} placeholder="Preset name..."
                class="flex-1 bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
              <button
                onclick={async () => {
                  if (!presetName.trim()) return
                  const active: Record<string, number> = {}
                  for (const s of SHIP_TYPES) if (shipQuantities[s.key]) active[s.key] = shipQuantities[s.key]
                  await api.createPreset(presetName.trim(), active)
                  presetName = ''
                  showSavePreset = false
                  loadPresets()
                }}
                class="text-[10px] px-2.5 py-1 rounded bg-green-700 hover:bg-green-600 text-white transition-colors"
              >Save</button>
            </div>
          {/if}

          <!-- Send button -->
          <button
            onclick={handleSend}
            disabled={sending || totalShipsSelected() === 0}
            class="w-full py-2.5 rounded-lg text-sm font-semibold transition-all duration-150
              {missionType === 'attack' ? 'bg-red-700 hover:bg-red-600 disabled:bg-red-900/50'
              : missionType === 'defense' ? 'bg-blue-600 hover:bg-blue-700 disabled:bg-blue-900/50'
              : missionType === 'transport' ? 'bg-amber-700 hover:bg-amber-600 disabled:bg-amber-900/50'
              : missionType === 'spy' ? 'bg-cyan-700 hover:bg-cyan-600 disabled:bg-cyan-900/50'
              : missionType === 'colonize' ? 'bg-emerald-700 hover:bg-emerald-600 disabled:bg-emerald-900/50'
              : missionType === 'recycle' ? 'bg-green-700 hover:bg-green-600 disabled:bg-green-900/50'
              : 'bg-purple-700 hover:bg-purple-600 disabled:bg-purple-900/50'}
              text-white disabled:text-white/40 disabled:cursor-not-allowed"
          >
            {sending ? 'Launching...' : `Send ${missionType.charAt(0).toUpperCase() + missionType.slice(1)} Fleet`}
          </button>
        </div>
      </div>
    </div>
  {/if}
</div>