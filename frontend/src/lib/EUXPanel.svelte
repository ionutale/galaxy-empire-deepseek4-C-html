<script lang="ts">
  import { api, type EUXStatus, type ScanResult } from './api'
  import { fmtNum } from './format'

  let status = $state<EUXStatus | null>(null)
  let selectedMoon = $state('')
  let galaxy = $state(1)
  let system = $state(1)
  let error = $state('')
  let scanning = $state(false)
  let result = $state<ScanResult | null>(null)

  function load() {
    api.getEUXStatus().then(s => {
      status = s
      if (s.moons.length > 0) {
        selectedMoon = s.moons[0].id
        galaxy = s.moons[0].galaxy
      }
    }).catch(e => error = e.message)
  }

  $effect(load)

  function selectedMoonData() {
    return status?.moons.find(m => m.id === selectedMoon)
  }

  async function handleScan() {
    if (!selectedMoon) return
    scanning = true
    error = ''
    result = null
    try {
      result = await api.scanSystem(selectedMoon, galaxy, system)
    } catch (e: any) {
      error = e.message
    } finally {
      scanning = false
    }
  }
</script>

<div class="space-y-3">
  <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">EU-X Radar Array</h2>

  {#if error}
    <div class="bg-red-900/50 text-red-400 p-2 rounded text-sm">{error}</div>
  {/if}

  {#if status}
    {#if status.moons.length === 0}
      <div class="bg-yellow-900/30 border border-yellow-700/50 rounded-lg p-3 text-yellow-400 text-xs text-center">
        You need a moon with EU-X Radar Array to scan systems.
      </div>
    {:else}
      <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-2">
        <div>
          <label class="text-xs text-gray-400 block mb-1">Radar Moon</label>
          <select bind:value={selectedMoon} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs">
            {#each status.moons as m}
              <option value={m.id}>{m.coordinate} EU-X Lv.{m.eu_x_level}</option>
            {/each}
          </select>
        </div>

        <div class="flex gap-2">
          <div class="flex-1">
            <label class="text-xs text-gray-400 block mb-1">Galaxy</label>
            <input type="number" min="1" max="9" bind:value={galaxy}
              disabled={selectedMoonData()?.eu_x_level === 1}
              class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
          </div>
          <div class="flex-1">
            <label class="text-xs text-gray-400 block mb-1">System</label>
            <input type="number" min="1" max="499" bind:value={system}
              class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
          </div>
        </div>

        <button
          onclick={handleScan}
          disabled={scanning || !selectedMoon}
          class="w-full bg-cyan-600 hover:bg-cyan-700 disabled:bg-cyan-800/50 text-white py-1.5 rounded text-xs font-medium transition-colors"
        >
          {scanning ? 'Scanning...' : 'Scan (50k Gas)'}
        </button>
      </div>
    {/if}

    {#if result}
      <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-1">
        <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide">System {result.system}</h3>
        {#if result.detected_fleets.length === 0}
          <p class="text-gray-500 text-xs text-center py-4">No fleet activity detected.</p>
        {:else}
          {#each result.detected_fleets as f}
            <div class="border border-[#1e293b] rounded p-2 text-xs space-y-1">
              <div class="flex justify-between text-gray-400">
                <span>Pos {f.position} · {f.mission_type}</span>
              </div>
              <div class="text-gray-500">
                Arrival: {new Date(f.arrival_time).toLocaleString()}
              </div>
              <div class="text-gray-500">
                Return: {new Date(f.return_time).toLocaleString()}
              </div>
              <div class="flex flex-wrap gap-1">
                {#each Object.entries(f.ships) as [type, count]}
                  {#if count > 0}
                    <span class="bg-[#1e293b] px-1.5 py-0.5 rounded text-gray-300">{type}: {fmtNum(count)}</span>
                  {/if}
                {/each}
              </div>
            </div>
          {/each}
        {/if}
      </div>
    {/if}
  {:else}
    <div class="text-gray-500 text-xs text-center py-8">Loading radar network...</div>
  {/if}
</div>
