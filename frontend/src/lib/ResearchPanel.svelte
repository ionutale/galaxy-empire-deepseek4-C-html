<script lang="ts">
  import { api, type PlanetView, type Research, type TechInfo } from './api'
  import { fmtNum as formatNum, fmtTime as formatTime } from './format'
  import { estimateResearchTime } from './estimates'
  import { countdown, isExpired, lockBody } from './timers.svelte'

  let { planet }: { planet: PlanetView } = $props()

  let research = $state<Research | null>(null)
  let selectedTech = $state<string | null>(null)
  let techInfo = $state<TechInfo | null>(null)
  let researchError = $state('')
  let researchLoading = $state(false)
  let activeCategory = $state('all')

  const TECHS = [
    { name: 'energy_tech', label: 'Energy Tech', icon: '⚡', category: 'Basic' },
    { name: 'laser_tech', label: 'Laser Tech', icon: '🔫', category: 'Basic' },
    { name: 'ion_tech', label: 'Ion Tech', icon: '⚛', category: 'Basic' },
    { name: 'hyperspace_tech', label: 'Hyperspace Tech', icon: '🌀', category: 'Basic' },
    { name: 'plasma_tech', label: 'Plasma Tech', icon: '🔥', category: 'Basic' },
    { name: 'ultra_temperature_tech', label: 'Ultra Temp Tech', icon: '🌡', category: 'Advanced' },
    { name: 'anti_gravity_tech', label: 'Anti-Gravity Tech', icon: '🔄', category: 'Advanced' },
    { name: 'weapons_tech', label: 'Weapons Tech', icon: '⚔', category: 'Combat' },
    { name: 'shielding_tech', label: 'Shielding Tech', icon: '🛡', category: 'Combat' },
    { name: 'strength_tech', label: 'Strength Tech', icon: '💪', category: 'Combat' },
    { name: 'combustion_drive', label: 'Combustion Drive', icon: '🔥', category: 'Drive' },
    { name: 'impulse_drive', label: 'Impulse Drive', icon: '💫', category: 'Drive' },
    { name: 'hyperspace_drive', label: 'Hyperspace Drive', icon: '🚀', category: 'Drive' },
    { name: 'astrophysics', label: 'Astrophysics', icon: '🌌', category: 'Other' },
    { name: 'computer_tech', label: 'Computer Tech', icon: '💻', category: 'Other' },
    { name: 'espionage_tech', label: 'Espionage Tech', icon: '🕵', category: 'Other' },
  ]

  const CATEGORIES = ['all', ...new Set(TECHS.map(t => t.category))] as const

  const PREREQ_MAP: Record<string, Record<string, number> | null> = {
    energy_tech: null,
    laser_tech: { energy_tech: 1 },
    ion_tech: { laser_tech: 5, energy_tech: 4 },
    hyperspace_tech: { energy_tech: 5 },
    plasma_tech: { laser_tech: 10 },
    ultra_temperature_tech: { laser_tech: 15, hyperspace_tech: 8 },
    anti_gravity_tech: { laser_tech: 10, hyperspace_tech: 7, impulse_drive: 6 },
    weapons_tech: null,
    shielding_tech: null,
    strength_tech: null,
    combustion_drive: null,
    impulse_drive: { combustion_drive: 5 },
    hyperspace_drive: { impulse_drive: 5, hyperspace_tech: 3 },
    astrophysics: null,
    computer_tech: null,
    espionage_tech: null,
  }

  let filteredTechs = $derived(
    activeCategory === 'all' ? TECHS : TECHS.filter(t => t.category === activeCategory)
  )

  let researching = $derived(
    research?.queued_tech != null && research?.research_end != null && new Date(research.research_end) > new Date()
  )

  $effect(() => {
    let cancelled = false
    async function load() {
      try { research = await api.getResearch() } catch {}
    }
    load()
    const interval = setInterval(() => {
      if (!cancelled) load()
    }, 10000)
    return () => { cancelled = true; clearInterval(interval) }
  })

  $effect(() => {
    if (selectedTech) {
      researchError = ''
      techInfo = null
      api.getTechInfo(planet.id, selectedTech).then(i => techInfo = i).catch(() => techInfo = null)
    }
  })

  $effect(() => {
    if (selectedTech) {
      lockBody(true)
    } else {
      lockBody(false)
    }
  })

  function levelOf(tech: string): number {
    return (research as any)?.[tech] ?? 0
  }

  function prereqsMet(tech: string): { ok: boolean; missing: string[] } {
    const prereqs = PREREQ_MAP[tech]
    if (!prereqs) return { ok: true, missing: [] }
    const missing: string[] = []
    for (const [p, lv] of Object.entries(prereqs)) {
      const cur = levelOf(p)
      if (cur < lv) {
        const label = TECHS.find(t => t.name === p)?.label ?? p
        missing.push(`${label} Lv.${lv}`)
      }
    }
    return { ok: missing.length === 0, missing }
  }

  function canAfford(cost: { metal: number; crystal: number; gas: number }): boolean {
    return planet.current_metal >= cost.metal && planet.current_crystal >= cost.crystal && planet.current_gas >= cost.gas
  }

  function techStatus(tech: string): 'locked' | 'available' | 'in_progress' | 'done' {
    if (research?.queued_tech === tech && researching) return 'in_progress'
    const p = PREREQ_MAP[tech]
    if (p) {
      for (const [k, v] of Object.entries(p)) {
        if (levelOf(k) < v) return 'locked'
      }
    }
    return 'available'
  }

  function statusBorder(status: string, sel: boolean): string {
    if (sel) return 'border-blue-500'
    switch (status) {
      case 'locked': return 'border-[#1e293b] opacity-50'
      case 'in_progress': return 'border-yellow-500/50'
      default: return 'border-[#1e293b] hover:border-blue-500/50'
    }
  }

  async function handleResearch() {
    if (!selectedTech) return
    researchLoading = true
    researchError = ''
    try {
      await api.queueResearch(planet.id, selectedTech)
      research = await api.getResearch()
      // Reload tech info so user can queue the next level
      api.getTechInfo(planet.id, selectedTech).then(i => {
        techInfo = i
      }).catch(() => {
        selectedTech = null
        techInfo = null
      })
    } catch (e: any) {
      researchError = e.message
    } finally {
      researchLoading = false
    }
  }

  function techDesc(name: string): string {
    const descs: Record<string, string> = {
      energy_tech: 'Improves energy efficiency. Required for Fusion Reactor and advanced weapons.',
      laser_tech: 'Advances laser weapon technology. Unlocks Light Lasers and Ion Tech.',
      ion_tech: 'Ion-based weapon systems. Unlocks Ion Cannons and Cruisers.',
      hyperspace_tech: 'Hyperspace physics. Required for Battleships, Dreadnoughts, and Hyperspace Drive.',
      plasma_tech: 'Plasma weapon systems. Unlocks Plasma Cannons and Bombers.',
      ultra_temperature_tech: 'Extreme temperature research. Required for Pioneer Lab and Iron Behemoth.',
      anti_gravity_tech: 'Anti-gravity field research. Required for Pioneer Lab and advanced structures.',
      weapons_tech: 'Increases all weapon attack power by 10% per level.',
      shielding_tech: 'Increases all shield strength by 10% per level.',
      strength_tech: 'Increases all ship armor integrity by 10% per level.',
      combustion_drive: 'Increases speed of combustion-engine ships. Required for early ship classes.',
      impulse_drive: 'Advanced propulsion. Required for Cruisers and faster cargo ships.',
      hyperspace_drive: 'Top-tier FTL drive. Required for Battleships, Dreadnoughts, and Destroyers.',
      astrophysics: 'Expands your empire. Each level increases colony slots. Level 2 required for Colony Ships.',
      computer_tech: 'Improves fleet command systems. Increases simultaneous fleet slots.',
      espionage_tech: 'Enhances sensor range. Higher levels reveal more intel and increase Nebula loot.',
    }
    return descs[name] || ''
  }
</script>

<div class="space-y-3">
  <!-- Category tabs -->
  <div class="category-scroll flex gap-1 overflow-x-auto pb-1 -mx-1 px-1 max-w-full" style="scrollbar-width: thin; scrollbar-color: #1e293b transparent; -webkit-overflow-scrolling: touch;">
    {#each CATEGORIES as cat}
      <button
        onclick={() => activeCategory = cat}
        class="px-2.5 py-1 text-[11px] uppercase tracking-wider rounded transition-all shrink-0
          {activeCategory === cat
            ? 'bg-blue-600/20 text-blue-400 border border-blue-600/40'
            : 'text-gray-600 border border-transparent hover:text-gray-400'}"
      >
        {cat}
      </button>
    {/each}
  </div>

  <!-- Tech grid -->
  <div class="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-4 gap-2">
    {#each filteredTechs as tech}
      {@const status = techStatus(tech.name)}
      <button
        onclick={() => selectedTech = tech.name}
        disabled={status === 'locked'}
        class="bg-[#0a0e1a] border {statusBorder(status, selectedTech === tech.name)} rounded p-3 hover:border-blue-500/50 transition-colors text-left relative group"
      >
        {#if status === 'in_progress'}
          <div class="absolute top-1 right-1 w-2 h-2 bg-yellow-400 rounded-full animate-pulse"></div>
        {/if}
        <div class="text-lg mb-1">{tech.icon}</div>
        <div class="text-xs text-gray-400 truncate">{tech.label}</div>
        <div class="text-sm font-mono text-white">Lv.{levelOf(tech.name)}</div>
        {#if status === 'locked'}
          <div class="text-xs text-red-400/60 mt-0.5">🔒 Locked</div>
        {/if}
        <div class="absolute bottom-1 right-1 text-[9px] text-gray-600 opacity-0 group-hover:opacity-100 transition-opacity">ℹ</div>
      </button>
    {/each}
  </div>

  <!-- Research queue -->
  {#if research?.research_queue && research.research_queue.length > 0}
    {@const resLabLv = planet.research_lab_level ?? 0}
    {@const queueTotal = research.research_queue.reduce((s, item) => s + estimateResearchTime(item.tech, levelOf(item.tech) + 1, resLabLv), 0)}
    <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-2.5">
      <div class="flex items-center justify-between text-[10px] text-gray-600 uppercase tracking-wider mb-2">
        <span>Research Queue</span>
        <span class="font-normal text-gray-500">{formatTime(queueTotal)}</span>
      </div>
      <div class="space-y-1">
        {#each research.research_queue as item}
          {@const estSec = estimateResearchTime(item.tech, levelOf(item.tech) + 1, resLabLv)}
          <div class="bg-[#111827] rounded px-2 py-1.5 border border-[#1e293b]">
            <div class="flex items-center justify-between mb-1">
              <span class="text-xs text-gray-400">{item.tech}</span>
              <button
                onclick={async () => { await api.cancelResearch(item.tech); research = await api.getResearch() }}
                class="text-[10px] text-red-400 hover:text-red-300 transition-colors"
              >Cancel</button>
            </div>
            <div class="w-full bg-black/50 rounded-full h-0.5 overflow-hidden">
              <div class="bg-purple-500/50 h-full rounded-full" style="width: 0%"></div>
            </div>
            <p class="text-[9px] text-gray-600 mt-0.5">≈{formatTime(estSec)}</p>
          </div>
        {/each}
      </div>
    </div>
  {/if}

  <!-- Tech detail modal -->
  {#if selectedTech}
    {@const curLevel = levelOf(selectedTech)}
    {@const status = techStatus(selectedTech)}
    <button class="fixed inset-0 bg-black/70 z-50" onclick={() => selectedTech = null}></button>
    <div class="fixed inset-0 flex items-center justify-center z-50 pointer-events-none">
      <div class="bg-[#111827] border border-[#1e293b] rounded-xl p-5 w-[400px] max-w-[95vw] max-h-[90vh] overflow-y-auto pointer-events-auto shadow-2xl shadow-blue-900/20">
        <div class="flex items-center justify-between mb-4">
          <div>
            <h3 class="text-base font-bold text-white">{TECHS.find(t => t.name === selectedTech)?.label ?? selectedTech}</h3>
            <p class="text-xs text-gray-500 font-mono mt-0.5">Level {curLevel} &rarr; {curLevel + 1}</p>
          </div>
          <button onclick={() => { selectedTech = null; techInfo = null }} class="text-gray-600 hover:text-gray-300 text-lg leading-none">&times;</button>
        </div>

        {#if techInfo}
          <div class="space-y-3 text-xs">
            <!-- Description -->
            {#if techDesc(selectedTech)}
              <p class="text-xs text-gray-500 leading-relaxed">{techDesc(selectedTech)}</p>
            {/if}

            <!-- Resource cost -->
            <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-2.5">
              <div class="text-gray-600 uppercase tracking-wider mb-1.5 text-[10px]">Cost</div>
              <div class="grid grid-cols-3 gap-2">
                <div class="text-center {canAfford(techInfo.cost) ? 'text-red-400' : 'text-red-700'}">
                  <div class="text-sm font-mono font-semibold">{formatNum(techInfo.cost.metal)}</div>
                  <div class="text-[10px] opacity-60">Metal</div>
                </div>
                <div class="text-center {canAfford(techInfo.cost) ? 'text-blue-400' : 'text-blue-700'}">
                  <div class="text-sm font-mono font-semibold">{formatNum(techInfo.cost.crystal)}</div>
                  <div class="text-[10px] opacity-60">Crystal</div>
                </div>
                <div class="text-center {canAfford(techInfo.cost) ? 'text-green-400' : 'text-green-700'}">
                  <div class="text-sm font-mono font-semibold">{formatNum(techInfo.cost.gas)}</div>
                  <div class="text-[10px] opacity-60">Gas</div>
                </div>
              </div>
            </div>

            <!-- Time & Lab -->
            <div class="grid grid-cols-2 gap-2">
              <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-2.5">
                <div class="text-gray-600 uppercase tracking-wider mb-1 text-[10px]">Duration</div>
                <div class="text-white font-mono text-sm">{formatTime(techInfo.time_seconds)}</div>
              </div>
              <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-2.5">
                <div class="text-gray-600 uppercase tracking-wider mb-1 text-[10px]">Lab Required</div>
                <div class="text-sm font-mono {planet.research_lab_level >= (techInfo.lab_level || 0) ? 'text-cyan-400' : 'text-red-400'}">
                  Lv.{planet.research_lab_level} / Lv.{techInfo.lab_level}
                </div>
              </div>
            </div>

            <!-- Prerequisites -->
            {#if techInfo.prereqs}
              <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-2.5">
                <div class="text-gray-600 uppercase tracking-wider mb-1.5 text-[10px]">Prerequisites</div>
                <div class="space-y-1">
                  {#each Object.entries(techInfo.prereqs) as [prereq, reqLevel]}
                    {@const cur = levelOf(prereq)}
                    {@const label = TECHS.find(t => t.name === prereq)?.label ?? prereq}
                    <div class="flex items-center justify-between text-xs">
                      <span class="{cur >= reqLevel ? 'text-green-400' : 'text-red-400'}">{label}</span>
                      <span class="font-mono {cur >= reqLevel ? 'text-green-400' : 'text-red-400'}">
                        {cur} / {reqLevel}
                      </span>
                    </div>
                  {/each}
                </div>
              </div>
            {/if}

            {#if researchError}
              <div class="bg-red-900/40 border border-red-800/30 text-red-400 p-2 rounded-lg text-center">{researchError}</div>
            {/if}

            <!-- Action -->
            {#if status === 'in_progress'}
              <div class="flex items-center gap-2">
                <div class="flex-1 bg-yellow-900/30 border border-yellow-600/30 text-yellow-400 p-2.5 rounded-lg text-center text-sm font-medium">
                  Researching... {countdown(research?.research_end)}
                </div>
                <button
                  onclick={async () => { await api.cancelResearch(selectedTech); research = await api.getResearch() }}
                  class="px-3 py-2.5 bg-red-800/40 hover:bg-red-700/50 text-red-400 rounded-lg text-xs font-medium transition-colors"
                >Cancel</button>
              </div>
            {:else}
              <button
                onclick={handleResearch}
                disabled={!canAfford(techInfo.cost) || !prereqsMet(selectedTech).ok || researchLoading}
                class="w-full py-2.5 rounded-lg text-sm font-semibold transition-all duration-150
                  {canAfford(techInfo.cost) && prereqsMet(selectedTech).ok && !researchLoading
                    ? 'bg-cyan-700 hover:bg-cyan-600 text-white'
                    : 'bg-gray-800 text-gray-500 cursor-not-allowed'}
                  disabled:opacity-50"
              >
                {researchLoading ? 'Researching...' : 'Research Level ' + (curLevel + 1)}
              </button>
            {/if}
          </div>
        {:else}
          <div class="text-gray-500 text-center py-8 text-sm">Loading tech data...</div>
        {/if}
      </div>
    </div>
  {/if}
</div>

<style>
  .category-scroll::-webkit-scrollbar { height: 4px; }
  .category-scroll::-webkit-scrollbar-thumb { background: #1e293b; border-radius: 2px; }
  .category-scroll::-webkit-scrollbar-track { background: transparent; }
</style>