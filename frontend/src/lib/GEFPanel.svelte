<script lang="ts">
  import { api, type FleetShips } from './api'
  import { SHIP_TYPES } from './shipTypes'
  import { lockBody } from './timers.svelte'
  import { fmtNum } from './format'

  let { planetId }: { planetId: string } = $props()

  let stages = $state<any[]>([])
  let loading = $state(true)
  let error = $state('')
  let selectedStage = $state<number | null>(null)
  let sending = $state(false)
  let battleResult = $state<any>(null)

  let ships = $state<FleetShips>({
    light_fighter: 0, heavy_fighter: 0, cruiser: 0,
    battleship: 0, dreadnought: 0, bomber: 0,
    destroyer: 0, small_cargo: 0, large_cargo: 0,
    colony_ship: 0, recycler: 0, probe: 0,
    solar_satellite: 0, interceptor: 0, iron_behemoth: 0,
    devourer_interceptor: 0, titanenergetic_frigate: 0,
    annihilator: 0,
  })

  const STAGE_DATA = [
    { name: 'Patrol Fleet', reward: 10 },
    { name: 'Recon Squad', reward: 15 },
    { name: 'Pirate Outpost', reward: 20 },
    { name: 'Smuggler Base', reward: 30 },
    { name: 'Warlord Garrison', reward: 40 },
    { name: 'Nebula Fortress', reward: 50 },
    { name: 'Dark Fleet', reward: 60 },
    { name: 'Ancient Vanguard', reward: 80 },
    { name: 'Shadow Armada', reward: 100 },
    { name: 'The Nexus', reward: 150 },
  ]

  $effect(() => { lockBody(!!selectedStage) })

  $effect(() => {
    loading = true
    api.getQuests('gef').then(data => {
      stages = data
      loading = false
    }).catch(e => {
      error = e.message
      loading = false
    })
  })

  function stageStatus(idx: number): 'completed' | 'available' | 'locked' {
    const s = stages[idx]
    if (s?.completed || s?.claimed) return 'completed'
    if (idx === 0 || stages[idx - 1]?.completed || stages[idx - 1]?.claimed) return 'available'
    return 'locked'
  }

  function openStage(idx: number) {
    if (stageStatus(idx) !== 'available') return
    selectedStage = idx
    battleResult = null
    ships = Object.fromEntries(SHIP_TYPES.map(s => [s.key, 0])) as FleetShips
  }

  function clearShips() {
    ships = Object.fromEntries(SHIP_TYPES.map(s => [s.key, 0])) as FleetShips
  }

  async function handleAttack() {
    if (selectedStage === null) return
    sending = true
    error = ''
    battleResult = null
    try {
      const activeShips: FleetShips = {} as FleetShips
      for (const st of SHIP_TYPES) {
        ;(activeShips as any)[st.key] = (ships as any)[st.key] || 0
      }
      const res = await api.sendGEFAttack(planetId, selectedStage + 1, activeShips)
      battleResult = res
      stages = stages.map((s, i) => i === selectedStage ? { ...s, completed: true } : s)
    } catch (e: any) {
      error = e.message
    } finally {
      sending = false
    }
  }
</script>

<div class="space-y-3">
  <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">G.E.F. Campaign</h2>

  {#if error}
    <div class="bg-red-900/50 text-red-400 p-2 rounded text-sm">{error}</div>
  {/if}

  {#if loading}
    <div class="text-gray-500 text-xs text-center py-8">Loading campaign...</div>
  {:else}
    <div class="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-5 gap-2">
      {#each STAGE_DATA as stage, idx}
        {@const status = stageStatus(idx)}
        <button
          onclick={() => openStage(idx)}
          disabled={status === 'locked'}
          class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-2.5 text-center transition-colors
            {status === 'completed' ? 'border-green-700 bg-green-900/20' : ''}
            {status === 'available' ? 'hover:border-blue-500 cursor-pointer' : ''}
            {status === 'locked' ? 'opacity-40 cursor-not-allowed' : ''}"
        >
          <div class="text-lg">{status === 'completed' ? '✅' : status === 'available' ? '⚔️' : '🔒'}</div>
          <div class="text-xs text-white font-medium mt-1">Stage {idx + 1}</div>
          <div class="text-[10px] text-gray-400 truncate">{stage.name}</div>
          <div class="text-[10px] text-yellow-400 mt-1">+{stage.reward} DM</div>
        </button>
      {/each}
    </div>

    {#if selectedStage !== null}
      <div class="fixed inset-0 z-50 flex items-center justify-center bg-black/60 backdrop-blur-sm" onclick={() => selectedStage = null}>
        <div class="bg-[#111827] border border-[#1e293b] rounded-xl p-4 w-full max-w-md mx-3 max-h-[90vh] overflow-y-auto" onclick={(e) => e.stopPropagation()}>
          <div class="flex items-center justify-between mb-3">
            <h3 class="text-sm font-bold text-white">Stage {selectedStage + 1} — {STAGE_DATA[selectedStage].name}</h3>
            <button onclick={() => selectedStage = null} class="text-gray-500 hover:text-white text-lg">&times;</button>
          </div>

          {#if battleResult}
            <div class="space-y-2 mb-3">
              <div class="bg-yellow-900/30 border border-yellow-700/50 rounded-lg p-3 text-center">
                <p class="text-sm font-bold text-yellow-400">
                  {battleResult.outcome === 'victory' ? 'VICTORY' : battleResult.outcome === 'defeat' ? 'DEFEAT' : battleResult.status || 'Resolved'}
                </p>
              </div>
              {#if battleResult.losses}
                <div class="bg-red-900/30 border border-red-700/50 rounded-lg p-2 text-xs text-red-400">
                  <p class="font-medium mb-1">Losses:</p>
                  {#each Object.entries(battleResult.losses) as [k, v]}
                    {#if v as number > 0}
                      <span class="mr-2">{k}: {fmtNum(v)}</span>
                    {/if}
                  {/each}
                </div>
              {/if}
              {#if battleResult.loot}
                <div class="bg-green-900/30 border border-green-700/50 rounded-lg p-2 text-xs text-green-400">
                  <p class="font-medium mb-1">Rewards:</p>
                  {#each Object.entries(battleResult.loot) as [k, v]}
                    <span class="mr-2">{k}: {v as string}</span>
                  {/each}
                </div>
              {/if}
              <button
                onclick={() => selectedStage = null}
                class="w-full bg-gray-600 hover:bg-gray-700 text-white py-1.5 rounded text-xs font-medium"
              >
                Close
              </button>
            </div>
          {:else}
            <div class="mb-3 text-xs text-gray-400">
              <p>Reward: <span class="text-yellow-400">{STAGE_DATA[selectedStage].reward} Dark Matter</span></p>
            </div>

            <div>
              <div class="flex items-center justify-between mb-1">
                <label class="text-xs text-gray-400">Ships</label>
                <button onclick={clearShips} class="text-[10px] text-gray-500 hover:text-gray-300">Clear All</button>
              </div>
              <div class="grid grid-cols-2 gap-1.5 max-h-60 overflow-y-auto">
                {#each SHIP_TYPES as ship}
                  <div class="flex items-center gap-1">
                    <span class="text-gray-400 text-xs w-20 truncate">{ship.label}</span>
                    <input type="number" min="0" bind:value={ships[ship.key]}
                      class="flex-1 w-16 bg-[#0a0e1a] border border-[#1e293b] text-white text-center rounded py-1 text-xs" />
                  </div>
                {/each}
              </div>
            </div>

            <button
              onclick={handleAttack}
              disabled={sending}
              class="w-full bg-red-600 hover:bg-red-700 disabled:bg-red-800/50 text-white py-2 rounded text-xs font-bold mt-3 transition-colors"
            >
              {sending ? 'Attacking...' : 'Attack'}
            </button>
          {/if}
        </div>
      </div>
    {/if}
  {/if}
</div>
