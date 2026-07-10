<script lang="ts">
  import { api, type WormholeStatus } from './api'
  import { SHIP_KEYS, SHIP_TYPES } from './shipTypes'
  import { countdown } from './timers.svelte'

  let status = $state<WormholeStatus | null>(null)
  let originMoon = $state('')
  let targetMoon = $state('')
  let error = $state('')
  let sending = $state(false)
  let result = $state<string | null>(null)
  let ships = $state<Record<string, number>>(
    Object.fromEntries(SHIP_KEYS.map(k => [k, 0]))
  )

  function load() {
    api.getWormholeStatus().then(s => {
      status = s
      error = ''
    }).catch(e => error = e.message)
  }

  $effect(load)

  function isOnCooldown(a: string, b: string): boolean {
    if (!status) return false
    return status.cooldowns.some(c =>
      (c.moon_a_id === a && c.moon_b_id === b) ||
      (c.moon_a_id === b && c.moon_b_id === a)
    )
  }

  function moonOptions() {
    return status?.moons ?? []
  }

  function targetOptions() {
    return moonOptions().filter(m => m.id !== originMoon)
  }

  async function handleSend() {
    if (!originMoon || !targetMoon) return
    sending = true
    error = ''
    result = null
    try {
      const activeShips: Record<string, number> = {}
      for (const [k, v] of Object.entries(ships)) {
        if (v > 0) activeShips[k] = v
      }
      await api.sendWormhole(originMoon, targetMoon, activeShips)
      result = 'Ships transferred successfully!'
      await load()
    } catch (e: any) {
      error = e.message
    } finally {
      sending = false
    }
  }
</script>

<div class="space-y-3">
  <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">Wormhole Generator</h2>

  {#if error}
    <div class="bg-red-900/50 text-red-400 p-2 rounded text-sm">{error}</div>
  {/if}

  {#if status}
    {#if status.moons.length < 2}
      <div class="bg-yellow-900/30 border border-yellow-700/50 rounded-lg p-3 text-yellow-400 text-xs text-center">
        You need at least 2 moons with Wormhole Generators to use the wormhole network.
      </div>
    {:else}
      <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-2">
        <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide">Send Fleet</h3>

        <div>
          <label class="text-xs text-gray-400 block mb-1">Origin Moon</label>
          <select bind:value={originMoon} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs">
            <option value="">Select origin...</option>
            {#each moonOptions() as m}
              <option value={m.id}>{m.coordinate} WG Lv.{m.wormhole_level}</option>
            {/each}
          </select>
        </div>

        <div>
          <label class="text-xs text-gray-400 block mb-1">Target Moon</label>
          <select bind:value={targetMoon} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs">
            <option value="">Select target...</option>
            {#each targetOptions() as m}
              <option value={m.id}
                disabled={isOnCooldown(originMoon, m.id)}
              >
                {m.coordinate} WG Lv.{m.wormhole_level}{isOnCooldown(originMoon, m.id) ? ' (cooldown)' : ''}
              </option>
            {/each}
          </select>
        </div>

        <div>
          <label class="text-xs text-gray-400 block mb-1">Ships to Transfer</label>
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
          onclick={handleSend}
          disabled={sending || !originMoon || !targetMoon || isOnCooldown(originMoon, targetMoon)}
          class="w-full bg-purple-600 hover:bg-purple-700 disabled:bg-purple-800/50 text-white py-1.5 rounded text-xs font-medium transition-colors"
        >
          {sending ? 'Sending...' : 'Send Through Wormhole'}
        </button>
      </div>
    {/if}

    {#if status.cooldowns.length > 0}
      <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-1">
        <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide">Active Cooldowns</h3>
        {#each status.cooldowns as cd, i}
          <div class="flex justify-between text-xs text-gray-400">
            <span>Moon pair {i + 1}</span>
            <span class="text-yellow-400">{countdown(cd.cooldown_until)}</span>
          </div>
        {/each}
      </div>
    {/if}

    {#if result}
      <div class="bg-green-900/30 border border-green-700/50 rounded-lg p-3 text-green-400 text-xs text-center">
        {result}
      </div>
    {/if}
  {:else}
    <div class="text-gray-500 text-xs text-center py-8">Loading wormhole network...</div>
  {/if}
</div>
