<script lang="ts">
  import { api, type BattleReport } from './api'
  import { fmtNum as fmt } from './format'

  let reports = $state<BattleReport[]>([])
  let error = $state('')
  let selected = $state<BattleReport | null>(null)

  $effect(() => {
    async function load() {
      try { reports = await api.getReports() } catch (e: any) { error = e.message }
    }
    load()
    const interval = setInterval(load, 15000)
    return () => clearInterval(interval)
  })

  function outcomeClass(o: string): string {
    if (o === 'attacker_victory') return 'text-green-400'
    if (o === 'defender_victory') return 'text-red-400'
    return 'text-yellow-400'
  }

  function outcomeLabel(o: string): string {
    if (o === 'attacker_victory') return 'Victory'
    if (o === 'defender_victory') return 'Defeat'
    return 'Draw'
  }

  function galactoniteTypeLabel(t: string): string {
    const labels: Record<string, string> = {
      flaming_galactonite: 'Flaming Crystals', shield_galactonite: 'Galactonite Shield',
      concentrated_galactonite: 'Concentrated', metal_galactonite: 'Metal Galactonite',
      blue_galactonite: 'Blue Galactonite', cloud_galactonite: 'Cloud Galactonite',
    }
    return labels[t] || t
  }

  function starsDisplay(s: number): string {
    return '★'.repeat(s)
  }

</script>

<div>
  <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide mb-4">Battle Reports</h2>
  {#if error}
    <p class="text-red-400 text-sm">{error}</p>
  {/if}

  {#if selected}
    <button onclick={() => selected = null} class="text-xs text-gray-400 hover:text-white mb-3">&larr; Back</button>
    <div class="bg-[#0a0e1a] rounded-lg border border-[#1e293b] p-4 space-y-2">
      <div class="flex justify-between">
        <span class="text-gray-400 text-xs">Outcome</span>
        <span class="text-sm font-bold {outcomeClass(selected.outcome)}">{outcomeLabel(selected.outcome)}</span>
      </div>
      <div class="flex justify-between">
        <span class="text-gray-400 text-xs">Loot</span>
        <span class="text-sm text-white">{fmt(selected.loot_metal)}M / {fmt(selected.loot_crystal)}C / {fmt(selected.loot_gas)}G</span>
      </div>
      <div class="flex justify-between">
        <span class="text-gray-400 text-xs">Debris</span>
        <span class="text-sm text-white">{fmt(selected.debris_metal)}M / {fmt(selected.debris_crystal)}C</span>
      </div>
      <div class="flex justify-between">
        <span class="text-gray-400 text-xs">Moon Chance</span>
        <span class="text-sm text-white">{(selected.moon_chance * 100).toFixed(1)}% {selected.moon_created ? '(Created!)' : ''}</span>
      </div>
      <div class="flex justify-between">
        <span class="text-gray-400 text-xs">Time</span>
        <span class="text-sm text-white">{new Date(selected.timestamp).toLocaleString()}</span>
      </div>
      {#if selected.galactonite_drops && selected.galactonite_drops.length > 0}
        <div class="border-t border-[#1e293b] pt-2 mt-2">
          <span class="text-xs text-gray-400 block mb-1">Galactonite Drops</span>
          <div class="space-y-1">
            {#each selected.galactonite_drops as drop}
              <div class="flex items-center gap-2 text-xs">
                <span class="text-purple-400">{galactoniteTypeLabel(drop.type)}</span>
                <span class="text-yellow-400">{starsDisplay(drop.stars)}</span>
              </div>
            {/each}
          </div>
        </div>
      {/if}
    </div>
  {:else if reports.length === 0}
    <p class="text-gray-500 text-sm">No battle reports yet.</p>
  {:else}
    <div class="space-y-2">
      {#each reports as r}
        <button
          onclick={() => selected = r}
          class="w-full bg-[#0a0e1a] rounded-lg p-3 border border-[#1e293b] text-left hover:border-gray-600 transition-colors"
        >
          <div class="flex justify-between items-center">
            <span class="{outcomeClass(r.outcome)} text-sm font-medium">{outcomeLabel(r.outcome)}</span>
            <span class="text-xs text-gray-500">{new Date(r.timestamp).toLocaleString()}</span>
          </div>
          <div class="text-xs text-gray-500 mt-1">
            Loot: {fmt(r.loot_metal)}M / {fmt(r.loot_crystal)}C / {fmt(r.loot_gas)}G &middot; Debris: {fmt(r.debris_metal)}M / {fmt(r.debris_crystal)}C
          </div>
        </button>
      {/each}
    </div>
  {/if}
</div>
