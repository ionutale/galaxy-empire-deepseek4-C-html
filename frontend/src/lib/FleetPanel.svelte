<script lang="ts">
  import { api, type FleetMission } from './api'
  import { fmtNum, gotoCoords, fmtCoords } from './format'
  import { countdown } from './timers.svelte'

  let missions = $state<FleetMission[]>([])
  let error = $state('')

  function load() {
    api.getFleets().then(m => {
      missions = m.sort((a, b) => new Date(b.arrival_time).getTime() - new Date(a.arrival_time).getTime())
      error = ''
    }).catch(e => error = e.message)
  }

  $effect(() => {
    load()
    const interval = setInterval(load, 5000)
    return () => clearInterval(interval)
  })

  function missionLabel(t: string): string {
    return t.charAt(0).toUpperCase() + t.slice(1)
  }

  const MISSION_ICONS: Record<string, string> = {
    attack: '⚔', transport: '📦', spy: '👁', recycle: '♻',
    explore: '✦', colonize: '🚀', defense: '🛡',
  }

  function shipCount(mission: FleetMission): number {
    let total = 0
    for (const key of Object.keys(mission.ships)) {
      total += (mission.ships as any)[key] || 0
    }
    return total
  }

  function shipSummary(mission: FleetMission): string {
    const parts: string[] = []
    for (const [key, val] of Object.entries(mission.ships)) {
      if (val > 0) parts.push(`${key.replace(/_/g, ' ')} x${val}`)
    }
    return parts.join(', ')
  }

  async function handleRecall(id: string) {
    try {
      await api.recallFleet(id)
      load()
    } catch (e: any) {
      error = e.message
    }
  }
</script>

<div class="space-y-3">
  <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">Active Fleets</h2>

  {#if error}
    <div class="bg-red-900/50 text-red-400 p-2 rounded text-sm">{error}</div>
  {/if}

  {#if missions.length === 0}
    <div class="text-gray-500 text-xs text-center py-4">No active fleet missions</div>
  {/if}

  {#each missions as mission}
    {@const isInTransit = !mission.recalled && new Date(mission.arrival_time).getTime() > Date.now()}
    <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-2">
      <div class="flex items-center justify-between">
        <div class="flex items-center gap-2">
          <span class="text-lg">{MISSION_ICONS[mission.mission_type] || '🚀'}</span>
          <span class="text-sm font-medium text-white">{missionLabel(mission.mission_type)}</span>
        </div>
        <button onclick={() => gotoCoords(mission.target_galaxy, mission.target_system)}
          class="text-xs font-mono text-blue-400 hover:text-blue-300 cursor-pointer">
          {fmtCoords(mission.target_galaxy, mission.target_system, mission.target_position)}
        </button>
      </div>

      <div class="flex justify-between items-center text-xs">
        <span class="text-gray-400">{fmtNum(shipCount(mission))} ships</span>
        <span class="font-mono {isInTransit ? 'text-blue-400' : 'text-green-400'}">
          {isInTransit ? countdown(mission.arrival_time) : countdown(mission.return_time, 'Returned')}
        </span>
      </div>

      {#if shipCount(mission) > 0}
        <div class="text-[10px] text-gray-600 truncate" title={shipSummary(mission)}>
          {shipSummary(mission).length > 80 ? shipSummary(mission).slice(0, 80) + '…' : shipSummary(mission)}
        </div>
      {/if}

      {#if isInTransit}
        <button
          onclick={() => handleRecall(mission.id)}
          class="text-xs text-red-400 hover:text-red-300 transition-colors"
        >Recall</button>
      {/if}
    </div>
  {/each}
</div>