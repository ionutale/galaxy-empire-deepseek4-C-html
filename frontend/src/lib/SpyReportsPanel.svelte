<script lang="ts">
  import { api, type SpyReport } from './api'
  import { fmtNum } from './format'

  let reports = $state<SpyReport[]>([])
  let error = $state('')
  let selected = $state<SpyReport | null>(null)

  $effect(() => {
    async function load() {
      try { reports = await api.getSpyReports() } catch (e: any) { error = e.message }
    }
    load()
    const interval = setInterval(load, 15000)
    return () => clearInterval(interval)
  })

  function intelLevel(r: SpyReport): string {
    let bits = ['Resources']
    if (r.fleet) bits.push('Fleet')
    if (r.defenses) bits.push('Defenses')
    if (r.buildings) bits.push('Buildings')
    if (r.research) bits.push('Research')
    return bits.join(', ')
  }
</script>

<div>
  <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide mb-4">Spy Reports</h2>
  {#if error}
    <p class="text-red-400 text-sm">{error}</p>
  {/if}

  {#if selected}
    <button onclick={() => selected = null} class="text-xs text-gray-400 hover:text-white mb-3">&larr; Back</button>
    <div class="bg-[#0a0e1a] rounded-lg border border-[#1e293b] p-4 space-y-2">
      <div class="flex justify-between">
        <span class="text-gray-400 text-xs">Resources</span>
        <span class="text-sm text-white">{fmtNum(selected.metal)}M / {fmtNum(selected.crystal)}C / {fmtNum(selected.gas)}G</span>
      </div>
      {#if selected.fleet}
        <div class="border-t border-[#1e293b] pt-2 mt-2">
          <span class="text-xs text-gray-400 block mb-1">Fleet</span>
          <div class="grid grid-cols-3 gap-1 text-xs text-gray-300">
            {#each Object.entries(selected.fleet) as [k, v]}
              {#if v > 0}
                <span>{k}: <span class="text-white">{fmtNum(v)}</span></span>
              {/if}
            {/each}
          </div>
        </div>
      {/if}
      {#if selected.defenses}
        <div class="border-t border-[#1e293b] pt-2 mt-2">
          <span class="text-xs text-gray-400 block mb-1">Defenses</span>
          <div class="grid grid-cols-3 gap-1 text-xs text-gray-300">
            {#each Object.entries(selected.defenses) as [k, v]}
              {#if v > 0}
                <span>{k}: <span class="text-white">{fmtNum(v)}</span></span>
              {/if}
            {/each}
          </div>
        </div>
      {/if}
      {#if selected.buildings}
        <div class="border-t border-[#1e293b] pt-2 mt-2">
          <span class="text-xs text-gray-400 block mb-1">Buildings</span>
          <div class="grid grid-cols-3 gap-1 text-xs text-gray-300">
            {#each Object.entries(selected.buildings) as [k, v]}
              <span>{k}: <span class="text-white">Lv.{v}</span></span>
            {/each}
          </div>
        </div>
      {/if}
      {#if selected.research}
        <div class="border-t border-[#1e293b] pt-2 mt-2">
          <span class="text-xs text-gray-400 block mb-1">Research</span>
          <div class="grid grid-cols-3 gap-1 text-xs text-gray-300">
            {#each Object.entries(selected.research) as [k, v]}
              {#if typeof v === 'number' && v > 0}
                <span>{k}: <span class="text-white">Lv.{v}</span></span>
              {/if}
            {/each}
          </div>
        </div>
      {/if}
    </div>
  {:else if reports.length === 0}
    <p class="text-gray-500 text-sm">No spy reports yet.</p>
  {:else}
    <div class="space-y-2">
      {#each reports as r}
        <button
          onclick={() => selected = r}
          class="w-full bg-[#0a0e1a] rounded-lg p-3 border border-[#1e293b] text-left hover:border-gray-600 transition-colors"
        >
          <div class="flex justify-between items-center">
            <span class="text-cyan-400 text-sm font-medium">Target: {r.target_id.slice(0, 8)}</span>
            <span class="text-xs text-gray-500">{new Date(r.created_at).toLocaleString()}</span>
          </div>
          <div class="text-xs text-gray-500 mt-1">
            {intelLevel(r)} &middot; {fmtNum(r.metal)}M / {fmtNum(r.crystal)}C / {fmtNum(r.gas)}G
          </div>
        </button>
      {/each}
    </div>
  {/if}
</div>
