<script lang="ts">
  import { api, type BattleReport, type FleetMission } from './api'
  import { fmtNum as fmt, gotoCoords, fmtCoords } from './format'

  let { planetId }: { planetId: string } = $props()

  let fleets = $state<FleetMission[]>([])
  let reports = $state<BattleReport[]>([])
  let error = $state('')

  $effect(() => {
    Promise.all([
      api.getFleets(),
      api.getReports(),
    ]).then(([f, r]) => {
      fleets = f
      reports = r
      error = ''
    }).catch(e => error = e.message)
  })

  let activeMissions = $derived(
    fleets.filter(f =>
      f.mission_type === 'explore' &&
      f.target_position === 16
    )
  )

  let nebulaResults = $derived(
    reports.filter(r =>
      r.outcome === 'exploration'
    ).slice().reverse()
  )

  function timeLeft(arrival: string): string {
    const end = new Date(arrival).getTime()
    const remaining = Math.max(0, Math.floor((end - Date.now()) / 1000))
    if (remaining < 60) return remaining + 's'
    if (remaining < 3600) return Math.floor(remaining / 60) + 'm ' + (remaining % 60) + 's'
    return Math.floor(remaining / 3600) + 'h ' + Math.floor((remaining % 3600) / 60) + 'm'
  }

  const formulaSteps = [
    { label: 'Base Roll', value: '1–100' },
    { label: 'Fleet Bonus', value: '15 + min(Recyclers+Probes−2, 10)' },
    { label: 'Espionage Bonus', value: '10 if Espionage ≥ 6' },
    { label: 'Max Roll', value: '120' },
  ]

  const rewardTiers = [
    { tier: 'Low (roll < 30)', metal: '≤ 2M', crystal: '≤ 1.2M', gas: '≤ 1.6M' },
    { tier: 'Medium (roll 30–60)', metal: '2M–5M', crystal: '1.2M–3M', gas: '1.6M–4M' },
    { tier: 'High (roll 60–90)', metal: '5M–8M', crystal: '3M–4.8M', gas: '4M–6.4M' },
    { tier: 'Max (roll ≥ 90)', metal: '8M–10M', crystal: '4.8M–6M', gas: '6.4M–8M' },
  ]
</script>

<div class="space-y-3">
  <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">Mysterious Nebula</h2>

  {#if error}
    <div class="bg-red-900/50 text-red-400 p-2 rounded text-sm">{error}</div>
  {/if}

  <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-2">
    <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide">About</h3>
    <p class="text-xs text-gray-500 leading-relaxed">
      Position 16 in every system. Send a fleet with an <span class="text-cyan-400">explore</span> mission
      to probe the nebula's mysteries. Bring Recyclers and Probes for better haul — the more you send
      (up to 12 total), the higher your roll bonus. Espionage Tech level 6+ grants an additional +10.
    </p>
    <p class="text-xs text-gray-500">
      Max rewards: <span class="text-yellow-400">{fmt(10000000)} Metal</span> /
      <span class="text-cyan-400">{fmt(6000000)} Crystal</span> /
      <span class="text-green-400">{fmt(8000000)} Gas</span>
    </p>
  </div>

  {#if activeMissions.length > 0}
    <div class="bg-[#0a0e1a] border border-cyan-700/50 rounded-lg p-3 space-y-2">
      <h3 class="text-xs font-semibold text-cyan-400 uppercase tracking-wide">Active Expeditions</h3>
      {#each activeMissions as m}
        <div class="flex justify-between items-center text-xs">
          <button onclick={() => gotoCoords(m.target_galaxy, m.target_system)} class="text-xs font-mono text-blue-400 hover:text-blue-300 cursor-pointer">
            {fmtCoords(m.target_galaxy, m.target_system, m.target_position)}
          </button>
          <span class="text-yellow-400">Arriving {timeLeft(m.arrival_time)}</span>
        </div>
      {/each}
    </div>
  {/if}

  <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-2">
    <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide">Reward Formula</h3>
    <div class="space-y-1">
      {#each formulaSteps as step}
        <div class="flex justify-between text-xs">
          <span class="text-gray-400">{step.label}</span>
          <span class="text-white font-mono">{step.value}</span>
        </div>
      {/each}
    </div>
    <p class="text-xs text-gray-500 mt-1">
      Total Roll = Base Roll + Fleet Bonus + Espionage Bonus. Higher roll = better tier multiplier.
    </p>
    <div class="mt-2 space-y-1">
      {#each rewardTiers as tier}
        <div class="flex justify-between text-xs bg-[#111827] rounded px-2 py-1">
          <span class="text-gray-400">{tier.tier}</span>
          <span class="text-gray-300">{tier.metal}M / {tier.crystal}C / {tier.gas}G</span>
        </div>
      {/each}
    </div>
  </div>

  <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-2">
    <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide">Exploration Reports ({nebulaResults.length})</h3>
    {#if nebulaResults.length === 0}
      <p class="text-gray-500 text-xs text-center py-4">No nebula expeditions yet.</p>
    {:else}
      <div class="space-y-1.5 max-h-80 overflow-y-auto">
        {#each nebulaResults as r}
          <div class="bg-[#111827] rounded-lg px-3 py-2 border border-[#1e293b]">
            <div class="flex items-center justify-between mb-1">
              <span class="text-[10px] text-gray-500">{new Date(r.timestamp).toLocaleDateString()} {new Date(r.timestamp).toLocaleTimeString()}</span>
              <span class="text-[10px] text-cyan-400 font-medium">Explore</span>
            </div>
            <div class="grid grid-cols-3 gap-2 text-center">
              <div>
                <div class="text-xs font-mono text-yellow-400">{fmt(r.loot_metal)}</div>
                <div class="text-[9px] text-gray-600">Metal</div>
              </div>
              <div>
                <div class="text-xs font-mono text-blue-400">{fmt(r.loot_crystal)}</div>
                <div class="text-[9px] text-gray-600">Crystal</div>
              </div>
              <div>
                <div class="text-xs font-mono text-green-400">{fmt(r.loot_gas)}</div>
                <div class="text-[9px] text-gray-600">Gas</div>
              </div>
            </div>
          </div>
        {/each}
      </div>
    {/if}
  </div>
</div>
