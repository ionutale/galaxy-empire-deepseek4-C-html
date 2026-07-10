<script lang="ts">
  import { api, type BattleReport, type SpyReport } from './api'
  import { fmtNum } from './format'

  let { planetId }: { planetId: string } = $props()

  let battleReports = $state<BattleReport[]>([])
  let spyReports = $state<SpyReport[]>([])
  let error = $state('')
  let tab = $state<'all' | 'combat' | 'espionage'>('all')
  let expandedId = $state<string | null>(null)

  type InboxItem =
    | { type: 'battle'; data: BattleReport }
    | { type: 'spy'; data: SpyReport }

  $effect(() => {
    Promise.all([
      api.getReports().catch((e: Error) => { error = e.message; return [] }),
      api.getSpyReports().catch((e: Error) => { error = e.message; return [] }),
    ]).then(([b, s]) => {
      battleReports = b
      spyReports = s
    })
  })

  let items = $derived.by(() => {
    const all: InboxItem[] = [
      ...battleReports.map(r => ({ type: 'battle' as const, data: r })),
      ...spyReports.map(r => ({ type: 'spy' as const, data: r })),
    ]
    const filtered = tab === 'all' ? all : all.filter(i => i.type === (tab === 'combat' ? 'battle' : 'spy'))
    return filtered.sort((a, b) => {
      const ta = a.type === 'battle' ? a.data.timestamp : a.data.created_at
      const tb = b.type === 'battle' ? b.data.timestamp : b.data.created_at
      return new Date(tb).getTime() - new Date(ta).getTime()
    })
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

  function intelLevel(r: SpyReport): string {
    let bits = ['Resources']
    if (r.fleet) bits.push('Fleet')
    if (r.defenses) bits.push('Defenses')
    if (r.buildings) bits.push('Buildings')
    if (r.research) bits.push('Research')
    return bits.join(', ')
  }

  function itemTimestamp(item: InboxItem): string {
    return new Date(item.type === 'battle' ? item.data.timestamp : item.data.created_at).toLocaleString()
  }

  function itemId(item: InboxItem): string {
    return item.type === 'battle' ? `b-${item.data.id}` : `s-${item.data.id}`
  }
</script>

<div>
  <div class="flex items-center justify-between mb-4">
    <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">Inbox</h2>
    <div class="flex gap-1 bg-[#0a0e1a] rounded-lg p-0.5 border border-[#1e293b]">
      <button
        onclick={() => tab = 'all'}
        class="px-3 py-1.5 text-xs font-medium rounded-md transition-colors {tab === 'all' ? 'bg-blue-600 text-white' : 'text-gray-400 hover:text-white'}"
      >All</button>
      <button
        onclick={() => tab = 'combat'}
        class="px-3 py-1.5 text-xs font-medium rounded-md transition-colors {tab === 'combat' ? 'bg-blue-600 text-white' : 'text-gray-400 hover:text-white'}"
      >Combat</button>
      <button
        onclick={() => tab = 'espionage'}
        class="px-3 py-1.5 text-xs font-medium rounded-md transition-colors {tab === 'espionage' ? 'bg-blue-600 text-white' : 'text-gray-400 hover:text-white'}"
      >Espionage</button>
    </div>
  </div>

  {#if error}
    <p class="text-red-400 text-sm mb-3">{error}</p>
  {/if}

  {#if items.length === 0}
    <p class="text-gray-500 text-xs text-center py-4">No reports yet.</p>
  {:else}
    <div class="space-y-2">
      {#each items as item (itemId(item))}
        {@const isExpanded = expandedId === itemId(item)}
        <div class="bg-[#0a0e1a] rounded-lg border border-[#1e293b] overflow-hidden transition-all">
          <button
            onclick={() => expandedId = isExpanded ? null : itemId(item)}
            class="w-full flex items-center gap-3 p-3 text-left hover:bg-[#111827] transition-colors"
          >
            {#if item.type === 'battle'}
              <span class="shrink-0 w-7 h-7 rounded-full flex items-center justify-center text-xs {outcomeClass(item.data.outcome)} bg-current/10">
                <svg class="w-3.5 h-3.5" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2l2.4 7.2H22l-6 4.8 2.4 7.2L12 16l-6.4 4.8L8 14l-6-4.8h7.6z"/></svg>
              </span>
            {:else}
              <span class="shrink-0 w-7 h-7 rounded-full flex items-center justify-center text-xs text-cyan-400 bg-cyan-400/10">
                <svg class="w-3.5 h-3.5" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="3"/><path d="M22 12c0 5.5-4.5 10-10 10S2 17.5 2 12 6.5 2 12 2s10 4.5 10 10z"/></svg>
              </span>
            {/if}
            <div class="flex-1 min-w-0">
              <div class="flex items-center gap-2">
                <span class="text-sm font-medium text-white truncate">
                  {#if item.type === 'battle'}
                    {outcomeLabel(item.data.outcome)}
                  {:else}
                    Spy Report
                  {/if}
                </span>
                {#if item.type === 'battle'}
                  <span class="text-[10px] font-semibold px-1.5 py-0.5 rounded {outcomeClass(item.data.outcome)} bg-current/10">{outcomeLabel(item.data.outcome).toUpperCase()}</span>
                {/if}
              </div>
              <div class="text-xs text-gray-500 mt-0.5">
                {#if item.type === 'battle'}
                  Loot: {fmtNum(item.data.loot_metal)}M / {fmtNum(item.data.loot_crystal)}C / {fmtNum(item.data.loot_gas)}G
                {:else}
                  {intelLevel(item.data)}
                {/if}
              </div>
            </div>
            <div class="shrink-0 text-right">
              <div class="text-[10px] text-gray-500 leading-tight">{itemTimestamp(item)}</div>
              <svg class="w-3 h-3 text-gray-600 mt-0.5 ml-auto transition-transform {isExpanded ? 'rotate-180' : ''}" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="6 9 12 15 18 9"/></svg>
            </div>
          </button>

          {#if isExpanded}
            <div class="border-t border-[#1e293b] p-3 space-y-2 text-sm">
              {#if item.type === 'battle'}
                {@const r = item.data}
                <div class="grid grid-cols-2 gap-2">
                  <div>
                    <span class="text-gray-500 text-xs">Outcome</span>
                    <p class="text-white font-medium {outcomeClass(r.outcome)}">{outcomeLabel(r.outcome)}</p>
                  </div>
                  <div>
                    <span class="text-gray-500 text-xs">Time</span>
                    <p class="text-white">{new Date(r.timestamp).toLocaleString()}</p>
                  </div>
                  <div>
                    <span class="text-gray-500 text-xs">Loot</span>
                    <p class="text-white">{fmtNum(r.loot_metal)}M / {fmtNum(r.loot_crystal)}C / {fmtNum(r.loot_gas)}G</p>
                  </div>
                  <div>
                    <span class="text-gray-500 text-xs">Debris</span>
                    <p class="text-white">{fmtNum(r.debris_metal)}M / {fmtNum(r.debris_crystal)}C</p>
                  </div>
                  <div>
                    <span class="text-gray-500 text-xs">Moon Chance</span>
                    <p class="text-white">{(r.moon_chance * 100).toFixed(1)}% {r.moon_created ? '(Created!)' : ''}</p>
                  </div>
                </div>
              {:else}
                {@const r = item.data}
                <div class="grid grid-cols-2 gap-2">
                  <div>
                    <span class="text-gray-500 text-xs">Target</span>
                    <p class="text-white font-mono text-xs">{r.target_id.slice(0, 12)}</p>
                  </div>
                  <div>
                    <span class="text-gray-500 text-xs">Resources</span>
                    <p class="text-white">{fmtNum(r.metal)}M / {fmtNum(r.crystal)}C / {fmtNum(r.gas)}G</p>
                  </div>
                </div>
                {#if r.fleet}
                  <div>
                    <span class="text-gray-500 text-xs block mb-1">Fleet</span>
                    <div class="grid grid-cols-3 gap-1 text-xs">
                      {#each Object.entries(r.fleet) as [k, v]}
                        {#if v > 0}
                          <span class="text-gray-400">{k}: <span class="text-white">{fmtNum(v)}</span></span>
                        {/if}
                      {/each}
                    </div>
                  </div>
                {/if}
                {#if r.defenses}
                  <div>
                    <span class="text-gray-500 text-xs block mb-1">Defenses</span>
                    <div class="grid grid-cols-3 gap-1 text-xs">
                      {#each Object.entries(r.defenses) as [k, v]}
                        {#if v > 0}
                          <span class="text-gray-400">{k}: <span class="text-white">{fmtNum(v)}</span></span>
                        {/if}
                      {/each}
                    </div>
                  </div>
                {/if}
                {#if r.buildings}
                  <div>
                    <span class="text-gray-500 text-xs block mb-1">Buildings</span>
                    <div class="grid grid-cols-3 gap-1 text-xs">
                      {#each Object.entries(r.buildings) as [k, v]}
                        <span class="text-gray-400">{k}: <span class="text-white">Lv.{v}</span></span>
                      {/each}
                    </div>
                  </div>
                {/if}
                {#if r.research}
                  <div>
                    <span class="text-gray-500 text-xs block mb-1">Research</span>
                    <div class="grid grid-cols-3 gap-1 text-xs">
                      {#each Object.entries(r.research) as [k, v]}
                        {#if typeof v === 'number' && v > 0}
                          <span class="text-gray-400">{k}: <span class="text-white">Lv.{v}</span></span>
                        {/if}
                      {/each}
                    </div>
                  </div>
                {/if}
              {/if}
            </div>
          {/if}
        </div>
      {/each}
    </div>
  {/if}
</div>
