<script lang="ts">
  import { api, type ShipyardData, type ShipUnitInfo, type DefenseUnitInfo } from './api'
  import { fmtNum as formatNum, fmtTime as formatTime } from './format'
  import { estimateProductionTime } from './estimates'
  import { SHIP_KEYS, DEFENSE_KEYS } from './shipTypes'
  import { lockBody } from './timers.svelte'

  let { planetId }: { planetId: string } = $props()

  let data = $state<ShipyardData | null>(null)
  let error = $state('')
  let selected = $state<string | null>(null)
  let quantity = $state(1)
  let producing = $state(false)
  let prodError = $state('')
  let cancellingUnit = $state<string | null>(null)

  let sortedShips = $derived(
    SHIP_KEYS.map(k => (data?.ships ?? []).find(s => s.type === k)).filter(Boolean) as ShipUnitInfo[]
  )
  let sortedDefenses = $derived(
    DEFENSE_KEYS.map(k => (data?.defenses ?? []).find(d => d.type === k)).filter(Boolean) as DefenseUnitInfo[]
  )

  function load() {
    api.getShipyard(planetId).then(d => {
      data = d
      error = ''
    }).catch(e => {
      error = e.message
    })
  }

  $effect(() => { load() })

  $effect(() => { lockBody(!!selected) })

  $effect(() => { lockBody(!!cancellingUnit) })

  function unitLabel(t: string): string {
    const labels: Record<string, string> = {
      light_fighter: 'Light Fighter', heavy_fighter: 'Heavy Fighter',
      cruiser: 'Cruiser', battleship: 'Battleship',
      dreadnought: 'Dreadnought', destroyer: 'Destroyer',
      bomber: 'Bomber',
      small_cargo: 'Small Cargo', large_cargo: 'Large Cargo',
      colony_ship: 'Colony Ship', recycler: 'Recycler',
      probe: 'Probe', solar_satellite: 'Solar Satellite',
      interceptor: 'Interceptor', iron_behemoth: 'Iron Behemoth',
      devourer_interceptor: 'Devourer Interceptor', titanenergetic_frigate: 'Titanenergetic Frigate',
      annihilator: 'Annihilator',
      rocket_launcher: 'Rocket Launcher', light_laser: 'Light Laser',
      heavy_laser: 'Heavy Laser', ion_cannon: 'Ion Cannon',
      mk2_cannon: 'MK2 Cannon', plasma_cannon: 'Plasma Cannon',
      proton_cannon: 'Proton Cannon',
      small_shield_dome: 'Small Shield Dome', large_shield_dome: 'Large Shield Dome',
    }
    return labels[t] || t
  }

  function unitDesc(t: string): string {
    const descs: Record<string, string> = {
      light_fighter: 'Fast and cheap combat ship. Effective in large numbers.',
      heavy_fighter: 'Tougher fighter with improved firepower and armor.',
      cruiser: 'Versatile medium combat ship with rapid-fire against Light Fighters.',
      battleship: 'Heavy combat vessel with powerful armament. The backbone of any serious fleet.',
      dreadnought: 'Massive capital ship with devastating rapid-fire against Battleships, Cruisers, and Heavy Fighters.',
      destroyer: 'Advanced warship with rapid-fire against Dreadnoughts and Probes.',
      bomber: 'Specialized fleet attacker with bonus against defensive structures.',
      interceptor: 'Fast anti-fighter craft designed to counter swarm tactics.',
      iron_behemoth: 'Ultimate siege engine. Produced exclusively at the Pioneer Lab on your moon.',
      devourer_interceptor: 'Specialized interceptor that counters other interceptors and medium hulls.',
      titanenergetic_frigate: 'Advanced frigate harnessing titan-energy for superior combat performance.',
      annihilator: 'Suicide bomber — AoE chain reaction bypassing shields. Maximum 200 per battle.',
      small_cargo: 'Small cargo vessel for transporting resources between your colonies.',
      large_cargo: 'Large freighter with substantially greater cargo capacity.',
      colony_ship: 'Colonizes new planets. Requires Astrophysics Tech level 2. One ship per mission.',
      recycler: 'Harvests debris fields after combat to recover Metal and Crystal.',
      probe: 'Fast reconnaissance drone for espionage. Also effective in Nebula exploration.',
      solar_satellite: 'Orbital power generator that produces energy for your colony.',
      rocket_launcher: 'Cheap and quick defensive turret. First line of defense.',
      light_laser: 'Improved defensive turret with better range and damage.',
      heavy_laser: 'Heavy laser turret with concentrated firepower against larger vessels.',
      ion_cannon: 'Ion-based weapon that disrupts ship systems. Effective vs medium hulls.',
      mk2_cannon: 'Advanced kinetic cannon with balanced damage across ship classes.',
      plasma_cannon: 'High-tech plasma weapon with devastating damage. Requires Plasma Tech.',
      proton_cannon: 'Experimental proton weapon. Extreme damage. Requires advanced facilities.',
      small_shield_dome: 'Planetary shield generator providing basic protection for your colony.',
      large_shield_dome: 'Reinforced shield generator with significantly higher absorption capacity.',
    }
    return descs[t] || ''
  }

  function isShip(t: string): boolean {
    return ['light_fighter','heavy_fighter','cruiser','battleship','dreadnought',
      'destroyer','bomber','small_cargo','large_cargo','colony_ship','recycler',
      'probe','solar_satellite','interceptor','iron_behemoth',
      'devourer_interceptor','titanenergetic_frigate','annihilator'].includes(t)
  }

  function unitCount(t: string): number {
    if (!data) return 0
    if (isShip(t)) {
      return (data.fleet_inventory as any)[t] ?? 0
    }
    return (data.defense_inventory as any)[t] ?? 0
  }

  async function handleProduce() {
    if (!selected) return
    producing = true
    prodError = ''
    try {
      await api.produceUnit(planetId, selected, quantity)
      selected = null
      quantity = 1
      load()
    } catch (e: any) {
      prodError = e.message
    } finally {
      producing = false
    }
  }

  function prereqLabel(k: string): string {
    return unitLabel(k) + (isShip(k) ? '' : '')
  }

  let remainingStr = $derived.by(() => {
    if (!data?.production) return ''
    const r = data.production.remaining_seconds
    return formatTime(r) + ' remaining'
  })

  function tierIcon(t: string): string {
    if (isShip(t)) {
      const combat = ['light_fighter','heavy_fighter','cruiser','battleship','dreadnought','destroyer','bomber']
      if (combat.includes(t)) return '⚔'
      if (t === 'colony_ship') return '🏠'
      if (t === 'recycler') return '♻'
      if (t === 'probe') return '👁'
      if (t === 'solar_satellite') return '☀'
      return '🚀'
    }
    return '🛡'
  }
</script>

<div class="space-y-4">
  <div class="flex items-center justify-between">
    <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">Shipyard</h2>
    <span class="text-xs text-gray-500">Shipyard Lv.{data?.shipyard_level ?? 0} | Nanite Lv.{data?.nanite_level ?? 0}</span>
  </div>

  {#if error}
    <div class="bg-red-900/50 text-red-400 p-2 rounded text-sm">{error}</div>
  {/if}

  {#if data?.production}
    <div class="bg-yellow-900/50 text-yellow-400 p-2 rounded text-sm flex justify-between items-center">
      <span>Producing: {unitLabel(data.production.unit_type)} x{data.production.quantity}</span>
      <div class="flex items-center gap-2">
        <span>{remainingStr}</span>
        <button
          onclick={async () => { cancellingUnit = data?.production?.unit_type ?? null }}
          class="text-xs text-red-400 hover:text-red-300 underline"
        >Cancel</button>
      </div>
    </div>
  {/if}

  <!-- Production Queue -->
  {#if data?.production_queue && data.production_queue.length > 0}
    {@const maxSlots = 1 + Math.floor((data?.shipyard_level ?? 0) / 2)}
    {@const queueTotal = data.production_queue.reduce((s, i) => s + estimateProductionTime(i.unit_type, i.quantity, data?.shipyard_level ?? 0, data?.nanite_level ?? 0), 0)}
    <div class="bg-[#0a0e1a] border border-[#1e293b] rounded p-2">
      <div class="flex items-center justify-between mb-1">
        <span class="text-xs text-gray-500 uppercase tracking-wide">Production Queue</span>
        <span class="text-xs text-gray-500">{data.production_queue.length} / {maxSlots} | {formatTime(queueTotal)}</span>
      </div>
      <div class="space-y-1">
        {#each data.production_queue as item, i}
          {@const estSec = estimateProductionTime(item.unit_type, item.quantity, data?.shipyard_level ?? 0, data?.nanite_level ?? 0)}
          <div class="text-xs text-gray-400">
            <div class="flex items-center justify-between">
              <span>#{i+1} {unitLabel(item.unit_type)} x{item.quantity}</span>
              <button
                onclick={async () => { cancellingUnit = item.unit_type }}
                class="text-red-400 hover:text-red-300 underline"
              >Cancel</button>
            </div>
            <div class="w-full bg-black/50 rounded-full h-0.5 mt-1 overflow-hidden">
              <div class="bg-green-500/50 h-full rounded-full" style="width: 0%"></div>
            </div>
            <span class="text-[9px] text-gray-600">≈{formatTime(estSec)}</span>
          </div>
        {/each}
      </div>
    </div>
  {/if}

  <!-- Ships -->
  <div>
    <p class="text-xs text-gray-500 uppercase tracking-wide mb-2">Ships</p>
    <div class="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-2">
      {#each sortedShips as ship}
        <button
          onclick={() => { selected = ship.type; quantity = 1 }}
          class="bg-[#0a0e1a] border {selected === ship.type ? 'border-blue-500' : ship.met ? 'border-[#1e293b]' : 'border-[#1e293b] opacity-50'} rounded p-3 hover:border-blue-500/50 transition-colors text-left relative group"
        >
          <div class="text-lg mb-1">{tierIcon(ship.type)}</div>
          <div class="text-xs text-gray-400 truncate">{unitLabel(ship.type)}</div>
          <div class="text-sm font-mono text-white">{formatNum(unitCount(ship.type))}</div>
          {#if !ship.met}
            <div class="text-xs text-red-400">🔒</div>
          {/if}
          <div class="absolute bottom-1 right-1 text-[9px] text-gray-600 opacity-0 group-hover:opacity-100 transition-opacity">ℹ</div>
        </button>
      {/each}
    </div>
  </div>

  <!-- Defenses -->
  <div>
    <p class="text-xs text-gray-500 uppercase tracking-wide mb-2">Defenses</p>
    <div class="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-2">
      {#each sortedDefenses as def}
        <button
          onclick={() => { selected = def.type; quantity = 1 }}
          class="bg-[#0a0e1a] border {selected === def.type ? 'border-blue-500' : def.met ? 'border-[#1e293b]' : 'border-[#1e293b] opacity-50'} rounded p-3 hover:border-blue-500/50 transition-colors text-left relative group"
        >
          <div class="text-lg mb-1">{tierIcon(def.type)}</div>
          <div class="text-xs text-gray-400 truncate">{unitLabel(def.type)}</div>
          <div class="text-sm font-mono text-white">{formatNum(unitCount(def.type))}</div>
          {#if !def.met}
            <div class="text-xs text-red-400">🔒</div>
          {/if}
          <div class="absolute bottom-1 right-1 text-[9px] text-gray-600 opacity-0 group-hover:opacity-100 transition-opacity">ℹ</div>
        </button>
      {/each}
    </div>
  </div>

  <!-- Production Dialog -->
  {#if selected}
    {@const unit = [...(data?.ships ?? []), ...(data?.defenses ?? [])].find(u => u.type === selected)}
    <div class="fixed inset-0 bg-black/60 flex items-center justify-center z-50" onclick={() => selected = null}>
      <div class="bg-[#111827] border border-[#1e293b] rounded-lg p-6 w-96 max-w-full" onclick={(e) => e.stopPropagation()}>
        <div class="flex justify-between items-start mb-4">
          <div>
            <h3 class="text-lg font-bold text-white">{unitLabel(selected)}</h3>
            <p class="text-sm text-gray-500">Owned: {unitCount(selected)}</p>
          </div>
          <button onclick={() => selected = null} class="text-gray-500 hover:text-white text-lg">&times;</button>
        </div>

        {#if unit}
          <div class="space-y-3">
            <!-- Description -->
            {#if unitDesc(selected)}
              <p class="text-xs text-gray-500 leading-relaxed">{unitDesc(selected)}</p>
            {/if}

            <!-- Cost -->
            <div>
              <p class="text-xs text-gray-400 uppercase tracking-wide mb-1">Cost per unit</p>
              <div class="flex gap-3 text-sm">
                <span class="text-red-400">{formatNum(unit.metal)} Metal</span>
                <span class="text-blue-400">{formatNum(unit.crystal)} Crystal</span>
                <span class="text-green-400">{formatNum(unit.gas)} Gas</span>
              </div>
            </div>

            <!-- Time -->
            <div>
              <p class="text-xs text-gray-400 uppercase tracking-wide mb-1">Time per unit</p>
              <p class="text-sm text-white">{formatTime(unit.time_seconds)}</p>
            </div>

            <!-- Quantity -->
            <div>
              <p class="text-xs text-gray-400 uppercase tracking-wide mb-1">Quantity</p>
              <div class="flex gap-2 items-center">
                <button onclick={() => quantity = Math.max(1, quantity - 1)} class="bg-[#0a0e1a] border border-[#1e293b] text-white px-3 py-1 rounded text-sm">-</button>
                <input type="number" bind:value={quantity} min="1" max="99999"
                  class="w-20 bg-[#0a0e1a] border border-[#1e293b] text-white text-center rounded py-1 text-sm" />
                <button onclick={() => quantity = Math.min(99999, quantity + 1)} class="bg-[#0a0e1a] border border-[#1e293b] text-white px-3 py-1 rounded text-sm">+</button>
              </div>
            </div>

            <!-- Total -->
            <div class="border-t border-[#1e293b] pt-2">
              <p class="text-xs text-gray-400 uppercase tracking-wide mb-1">Total for {quantity}</p>
              <div class="flex gap-3 text-sm">
                <span class="text-red-400">{formatNum(unit.metal * quantity)} Metal</span>
                <span class="text-blue-400">{formatNum(unit.crystal * quantity)} Crystal</span>
                <span class="text-green-400">{formatNum(unit.gas * quantity)} Gas</span>
              </div>
              <p class="text-sm text-white mt-1">Time: {formatTime(unit.time_seconds * quantity)}</p>
            </div>

            <!-- Prerequisites -->
            {#if unit.prereqs}
              <div>
                <p class="text-xs text-gray-400 uppercase tracking-wide mb-1">Required</p>
                {#each Object.entries(unit.prereqs) as [prereq, reqLevel]}
                  <p class="text-xs text-green-400">{unitLabel(prereq)} Lv.{reqLevel}</p>
                {/each}
              </div>
            {/if}

            {#if prodError}
              <div class="bg-red-900/50 text-red-400 p-2 rounded text-sm">{prodError}</div>
            {/if}

            <button
              onclick={handleProduce}
              disabled={producing}
              class="w-full bg-blue-600 hover:bg-blue-700 disabled:bg-blue-800/50 text-white py-2 rounded font-medium transition-colors"
            >
              {producing ? 'Building...' : 'Produce x' + quantity}
            </button>
          </div>
        {:else}
          <div class="text-gray-500 text-center py-4">Loading...</div>
        {/if}
      </div>
    </div>
  {/if}

  <!-- Cancel Confirmation Dialog -->
  {#if cancellingUnit}
    <div class="fixed inset-0 bg-black/60 flex items-center justify-center z-50" onclick={() => cancellingUnit = null}>
      <div class="bg-[#111827] border border-[#1e293b] rounded-lg p-6 w-80 max-w-full" onclick={(e) => e.stopPropagation()}>
        <h3 class="text-lg font-bold text-white mb-2">Cancel Production</h3>
        <p class="text-sm text-gray-400 mb-4">Cancel {unitLabel(cancellingUnit)}? You'll be refunded 75% of the cost.</p>
        <div class="flex gap-2">
          <button
            onclick={async () => {
              const unitType = cancellingUnit
              cancellingUnit = null
              try {
                await api.cancelProduction(planetId, unitType)
                load()
              } catch (e: any) {
                prodError = e.message
              }
            }}
            class="flex-1 bg-red-600 hover:bg-red-700 text-white py-2 rounded font-medium transition-colors text-sm"
          >Confirm Cancel</button>
          <button onclick={() => cancellingUnit = null} class="flex-1 bg-[#1e293b] hover:bg-[#293548] text-white py-2 rounded font-medium transition-colors text-sm">Keep</button>
        </div>
      </div>
    </div>
  {/if}
</div>
