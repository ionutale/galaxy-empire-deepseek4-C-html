<script lang="ts">
  import { api, type FortressStatus, type FleetMission } from './api'
  import { SHIP_KEYS, SHIP_TYPES } from './shipTypes'
  import { gotoCoords, fmtCoords } from './format'

  let { planets: externalPlanets }: { planets: { id: string }[] } = $props()

  let status = $state<FortressStatus | null>(null)
  let selectedPlanet = $state('')
  let selectedFortress = $state('dwarf-star')
  let error = $state('')
  let sending = $state(false)
  let claiming = $state(false)
  let result = $state<FleetMission | null>(null)
  let claimResult = $state('')
  let planets = $state<{ id: string }[]>(externalPlanets)
  let ships = $state<Record<string, number>>(
    Object.fromEntries(SHIP_KEYS.map(k => [k, 0]))
  )

  $effect(() => {
    planets = externalPlanets
  })

  // Fallback: fetch planets if none provided
  $effect(() => {
    if (planets.length === 0) {
      api.getPlanets().then(p => { planets = p }).catch(() => {})
    }
  })

  const FORTRESS_OPTIONS = [
    { id: 'dwarf-star', label: 'Dwarf Star Fortress', reward: '50 Dark Matter', garrison: '2,000 Cruiser + 1,000 Heavy Fighter' },
  ]

  const fortressLabel = $derived(
    FORTRESS_OPTIONS.find(f => f.id === selectedFortress)?.label || selectedFortress
  )
  const fortressReward = $derived(
    FORTRESS_OPTIONS.find(f => f.id === selectedFortress)?.reward || ''
  )
  const fortressGarrison = $derived(
    FORTRESS_OPTIONS.find(f => f.id === selectedFortress)?.garrison || ''
  )

  function load() {
    api.getFortressStatus().then(s => {
      status = s
      error = ''
    }).catch(e => error = e.message)
  }

  $effect(load)

  async function handleAttack() {
    if (!selectedPlanet) return
    sending = true
    error = ''
    result = null
    try {
      const activeShips: Record<string, number> = {}
      for (const [k, v] of Object.entries(ships)) {
        if (v > 0) activeShips[k] = v
      }
      result = await api.attackFortress(selectedPlanet, activeShips)
      await load()
    } catch (e: any) {
      error = e.message
    } finally {
      sending = false
    }
  }

  async function handleClaim() {
    claiming = true
    claimResult = ''
    error = ''
    try {
      await api.claimFortress()
      claimResult = 'Fortress claimed!'
      await load()
    } catch (e: any) {
      claimResult = ''
      error = e.message
    } finally {
      claiming = false
    }
  }
</script>

<div class="space-y-3">
  <div class="flex items-center justify-between">
    <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">Fortress</h2>
    <select bind:value={selectedFortress} class="bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs">
      {#each FORTRESS_OPTIONS as opt}
        <option value={opt.id}>{opt.label}</option>
      {/each}
    </select>
  </div>

  {#if error}
    <div class="bg-red-900/50 text-red-400 p-2 rounded text-sm">{error}</div>
  {/if}

  {#if status}
    <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-2 text-xs">
      <div class="flex justify-between">
        <span class="text-gray-400">Fortress</span>
        <span class="text-white">{fortressLabel}</span>
      </div>
      <div class="flex justify-between">
        <span class="text-gray-400">Coordinates</span>
        <button onclick={() => gotoCoords(status.coordinates.galaxy, status.coordinates.system)} class="text-xs font-mono text-blue-400 hover:text-blue-300 cursor-pointer">
          {fmtCoords(status.coordinates.galaxy, status.coordinates.system, status.coordinates.position)}
        </button>
      </div>
      <div class="flex justify-between">
        <span class="text-gray-400">Garrison</span>
        <span class="text-white">{fortressGarrison}</span>
      </div>
      <div class="flex justify-between">
        <span class="text-gray-400">Controlled By</span>
        <span class="text-white">{status.controlling_alliance || 'None'}</span>
      </div>
      <div class="flex justify-between">
        <span class="text-gray-400">Attacks Today</span>
        <span class="text-white">
          {status.attacks_today} / {status.max_attacks}
          {#if status.attacks_today > 0}
            <span class="text-gray-500 text-[10px] ml-1">({status.max_attacks - status.attacks_today} remaining)</span>
          {/if}
        </span>
      </div>
      <div class="flex justify-between">
        <span class="text-gray-400">Reward</span>
        <span class="text-yellow-400">{fortressReward}</span>
      </div>
    </div>

    {#if status.controlling_alliance_id === null}
      <button
        onclick={handleClaim}
        disabled={claiming}
        class="w-full bg-purple-600 hover:bg-purple-700 disabled:bg-purple-800/50 text-white py-1.5 rounded text-xs font-medium transition-colors"
      >
        {claiming ? 'Claiming...' : 'Claim Fortress'}
      </button>
    {:else}
      <div class="bg-purple-900/30 border border-purple-700/50 rounded-lg p-3 text-purple-400 text-xs text-center">
        Controlled by {status.controlling_alliance}
      </div>
    {/if}

    {#if status.attacks_today < status.max_attacks}
      <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-2">
        <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide">Send Fleet</h3>

        <div>
          <label class="text-xs text-gray-400 block mb-1">Origin Planet</label>
          <select bind:value={selectedPlanet} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs">
            <option value="">Select planet...</option>
            {#each planets as p, i}
              <option value={p.id}>Planet {i + 1}</option>
            {/each}
          </select>
        </div>

        <div>
          <label class="text-xs text-gray-400 block mb-1">Ships</label>
          <div class="grid grid-cols-2 gap-1">
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
          disabled={sending || !selectedPlanet}
          class="w-full bg-orange-600 hover:bg-orange-700 disabled:bg-orange-800/50 text-white py-1.5 rounded text-xs font-medium transition-colors"
        >
          {sending ? 'Sending...' : 'Attack Fortress'}
        </button>
      </div>
    {:else}
      <div class="bg-yellow-900/30 border border-yellow-700/50 rounded-lg p-3 text-yellow-400 text-xs text-center">
        Daily attack limit reached ({status.max_attacks}/{status.max_attacks}). Come back tomorrow.
      </div>
    {/if}

    {#if result}
      <div class="bg-green-900/30 border border-green-700/50 rounded-lg p-3 text-green-400 text-xs">
        Fleet sent! Arrival in {Math.max(0, Math.floor((new Date(result.arrival_time).getTime() - Date.now()) / 1000))}s.
      </div>
    {/if}

    {#if claimResult}
      <div class="bg-green-900/30 border border-green-700/50 rounded-lg p-3 text-green-400 text-xs">
        {claimResult}
      </div>
    {/if}
  {:else}
    <div class="text-gray-500 text-xs text-center py-8">Loading fortress data...</div>
  {/if}
</div>
