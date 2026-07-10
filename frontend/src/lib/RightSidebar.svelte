<script lang="ts">
  import { api, type PlanetView, type FleetMission, type Research } from './api'
  import { buildingLabel } from './buildingLabels'
  import { unitLabel } from './shipTypes'
  import { countdown, secondsRemaining } from './timers.svelte'
  import { estimateBuildTime, estimateProductionTime, estimateResearchTime } from './estimates'
  import { fmtNum, fmtTime, gotoCoords, fmtCoords } from './format'

  let { currentPlanet, onNavigate }: { currentPlanet: PlanetView | null; onNavigate?: (v: string) => void } = $props()

  let pv = $state<PlanetView | null>(null)
  let research = $state<Research | null>(null)
  let fleets = $state<FleetMission[]>([])
  let notifications = $state<{ type: string; timestamp: string; label: string }[]>([])
  // Sync pv from prop
  $effect(() => {
    pv = currentPlanet
  })

  async function fetchData() {
    try {
      const r = await api.getResearch()
      research = r
    } catch (e) {
      console.error('research fetch failed', e)
    }
    try {
      const f = await api.getFleets()
      fleets = f
      const now = Date.now()
      const items: { type: string; timestamp: string; label: string }[] = []
      for (const m of f) {
        const arr = new Date(m.arrival_time).getTime()
        const ret = new Date(m.return_time).getTime()
        if (arr > now && arr < now + 3600000) {
          items.push({ type: 'fleet', timestamp: m.arrival_time, label: `${m.mission_type} fleet arriving` })
        }
        if (ret > now && ret < now + 3600000) {
          items.push({ type: 'fleet', timestamp: m.return_time, label: `${m.mission_type} fleet returning` })
        }
      }
      items.sort((a, b) => new Date(a.timestamp).getTime() - new Date(b.timestamp).getTime())
      notifications = items.slice(0, 5)
    } catch {}
  }

  $effect(() => {
    fetchData()
    const interval = setInterval(fetchData, 10000)
    return () => clearInterval(interval)
  })
</script>

<aside class="w-60 bg-[#111827] border-l border-[#1e293b] flex flex-col h-full">
  <div class="px-4 py-4 border-b border-[#1e293b]">
    <h2 class="text-xs font-semibold text-gray-500 uppercase tracking-wider">Activity</h2>
  </div>
  <div class="flex-1 overflow-y-auto px-3 py-3 space-y-4">

    <!-- Construction + Queue -->
    <div>
      <h3 class="text-[10px] font-semibold text-gray-500 uppercase tracking-wider mb-2">Construction {#if (pv?.construction_queue ?? []).length > 0}<span class="text-gray-600 font-normal">| {fmtTime((pv?.construction_queue ?? []).reduce((s, i) => s + estimateBuildTime(i.building, i.target_level, pv?.robotics_level ?? 0), 0))}</span>{/if}</h3>
      {#if pv?.const_building && pv?.construction_end}
        <!-- svelte-ignore a11y_click_events_have_key_events a11y_interactive_supports_focus -->
        <div role="button" tabindex="0" onclick={() => onNavigate?.('planet')} class="bg-[#0a0e1a] border border-yellow-800/30 rounded-lg p-2.5 mb-1.5 cursor-pointer hover:bg-[#1e293b] transition-colors">
          <div class="flex items-center gap-2 mb-1.5">
            <span class="w-1.5 h-1.5 bg-yellow-400 rounded-full animate-pulse"></span>
            <span class="text-xs text-yellow-400 font-medium">{buildingLabel(pv.const_building)}</span>
          </div>
          <div class="flex justify-between text-[10px] text-gray-500">
            <span>Lv.{(pv as any)[pv.const_building + '_level'] ?? 0}</span>
            <span class="text-yellow-400 font-mono">{countdown(pv?.construction_end, 'Complete!')}</span>
          </div>
          <div class="w-full bg-black/50 rounded-full h-1 mt-1.5 overflow-hidden">
            <div class="bg-yellow-500 h-full rounded-full animate-pulse"></div>
          </div>
        </div>
      {:else}
        <p class="text-xs text-gray-500">Idle</p>
      {/if}
      {#if (pv?.construction_queue ?? []).length > 0}
        <div class="space-y-1">
          {#each pv.construction_queue as item}
            <!-- svelte-ignore a11y_click_events_have_key_events a11y_interactive_supports_focus -->
            <div role="button" tabindex="0" onclick={() => onNavigate?.('planet')} class="bg-[#0a0e1a] rounded px-2 py-1 border border-[#1e293b] cursor-pointer hover:bg-[#1e293b] transition-colors">
              <p class="text-[10px] text-gray-400 truncate">{buildingLabel(item.building)}</p>
              <p class="text-[9px] text-gray-600">Lv.{item.target_level - 1} → Lv.{item.target_level}</p>
              <div class="w-full bg-black/50 rounded-full h-0.5 mt-1 overflow-hidden">
                <div class="bg-yellow-500/50 h-full rounded-full" style="width: 0%"></div>
              </div>
              <p class="text-[8px] text-gray-600 mt-0.5">≈{fmtTime(estimateBuildTime(item.building, item.target_level, pv?.robotics_level ?? 0))}</p>
            </div>
          {/each}
        </div>
      {/if}
    </div>

    <!-- Production + Queue -->
    <div>
      <h3 class="text-[10px] font-semibold text-gray-500 uppercase tracking-wider mb-2">Production {#if (pv?.production_queue ?? []).length > 0}<span class="text-gray-600 font-normal">| {fmtTime((pv?.production_queue ?? []).reduce((s, i) => s + estimateProductionTime(i.unit_type, i.quantity, pv?.shipyard_level ?? 0, pv?.nanite_factory_level ?? 0), 0))}</span>{/if}</h3>
      {#if pv?.prod_unit_type}
        <!-- svelte-ignore a11y_click_events_have_key_events a11y_interactive_supports_focus -->
        <div role="button" tabindex="0" onclick={() => onNavigate?.('planet')} class="bg-[#0a0e1a] border border-green-800/30 rounded-lg p-2.5 mb-1.5 cursor-pointer hover:bg-[#1e293b] transition-colors">
          <div class="flex items-center gap-2 mb-1">
            <span class="w-1.5 h-1.5 bg-green-400 rounded-full animate-pulse"></span>
            <span class="text-xs text-green-400 font-medium">{unitLabel(pv.prod_unit_type)}</span>
          </div>
          <div class="flex justify-between text-[10px] text-gray-500">
            <span>x{fmtNum(pv.prod_quantity)}</span>
            {#if pv?.production_end}
              <span class="text-green-400 font-mono">{countdown(pv.production_end)}</span>
            {/if}
          </div>
          {#if pv?.production_end}
            <div class="w-full bg-black/50 rounded-full h-1 mt-1.5 overflow-hidden">
              <div class="bg-green-500 h-full rounded-full animate-pulse"></div>
            </div>
          {/if}
        </div>
      {:else}
        <p class="text-xs text-gray-500">Idle</p>
      {/if}
      {#if (pv?.production_queue ?? []).length > 0}
        <div class="space-y-1">
          {#each pv.production_queue as item}
            <!-- svelte-ignore a11y_click_events_have_key_events a11y_interactive_supports_focus -->
            <div role="button" tabindex="0" onclick={() => onNavigate?.('planet')} class="bg-[#0a0e1a] rounded px-2 py-1 border border-[#1e293b] cursor-pointer hover:bg-[#1e293b] transition-colors">
              <p class="text-[10px] text-gray-400 truncate">{unitLabel(item.unit_type)}</p>
              <p class="text-[9px] text-gray-600">x{fmtNum(item.quantity)}</p>
              <div class="w-full bg-black/50 rounded-full h-0.5 mt-1 overflow-hidden">
                <div class="bg-green-500/50 h-full rounded-full" style="width: 0%"></div>
              </div>
              <p class="text-[8px] text-gray-600 mt-0.5">≈{fmtTime(estimateProductionTime(item.unit_type, item.quantity, pv?.shipyard_level ?? 0, pv?.nanite_factory_level ?? 0))}</p>
            </div>
          {/each}
        </div>
      {/if}
    </div>

    <!-- Research -->
    <div>
      <h3 class="text-[10px] font-semibold text-gray-500 uppercase tracking-wider mb-2">Research {#if research?.research_queue && research.research_queue.length > 0}<span class="text-gray-600 font-normal">| {research.research_queue.length} items queued</span>{/if}</h3>
      {#if research?.research_end}
        {@const rTech = research.queued_tech}
        {@const rLevel = rTech ? ((research as any)?.[rTech] ?? 0) : 0}
        {@const remaining = secondsRemaining(research?.research_end)}
        {@const totalEst = rTech ? estimateResearchTime(rTech, rLevel + 1, pv?.research_lab_level ?? 0) : 0}
        {@const pct = totalEst > 0 ? Math.max(0, Math.min(100, (1 - remaining / totalEst) * 100)) : 0}
        <!-- svelte-ignore a11y_click_events_have_key_events a11y_interactive_supports_focus -->
        <div role="button" tabindex="0" onclick={() => onNavigate?.('planet')} class="bg-[#0a0e1a] border border-purple-800/30 rounded-lg p-2.5 cursor-pointer hover:bg-[#1e293b] transition-colors">
          <div class="flex items-center gap-2 mb-1.5">
            <span class="w-1.5 h-1.5 bg-purple-400 rounded-full animate-pulse"></span>
            <span class="text-xs text-purple-400 font-medium">{rTech || 'Research'}</span>
          </div>
          <div class="flex justify-between text-[10px] text-gray-500">
            <span>{rTech}</span>
            <span class="text-purple-400 font-mono">{countdown(research?.research_end, 'Complete!')}</span>
          </div>
          <div class="w-full bg-black/50 rounded-full h-1 mt-1.5 overflow-hidden">
            <div class="bg-purple-500 h-full rounded-full transition-all duration-1000" style="width: {pct}%"></div>
          </div>
        </div>
        {#if research?.research_queue && research.research_queue.length > 0}
          <div class="space-y-1 mt-1.5">
            {#each research.research_queue as item}
              <div class="bg-[#0a0e1a] rounded px-2 py-1 border border-[#1e293b]">
                <div class="flex items-center justify-between mb-1">
                  <span class="text-[10px] text-gray-400 truncate">{item.tech}</span>
                  <button
                    onclick={async () => { await api.cancelResearch(item.tech); research = await api.getResearch() }}
                    class="text-[9px] text-red-400 hover:text-red-300 shrink-0 ml-1"
                  >Cancel</button>
                </div>
                <div class="w-full bg-black/50 rounded-full h-0.5 overflow-hidden">
                  <div class="bg-purple-500/50 h-full rounded-full" style="width: 0%"></div>
                </div>
                <p class="text-[8px] text-gray-600 mt-0.5">Pending</p>
              </div>
            {/each}
          </div>
        {/if}
      {:else}
        <p class="text-xs text-gray-500">Idle</p>
      {/if}
    </div>

    <!-- Notifications -->
    <div>
      <h3 class="text-[10px] font-semibold text-gray-500 uppercase tracking-wider mb-2">Notifications</h3>
      {#if notifications.length === 0}
        <p class="text-xs text-gray-500">No recent activity</p>
      {:else}
        <div class="space-y-1">
          {#each notifications as r}
            <!-- svelte-ignore a11y_click_events_have_key_events a11y_interactive_supports_focus -->
            <div role="button" tabindex="0" onclick={() => onNavigate?.('fleet')} class="bg-[#0a0e1a] rounded px-2 py-1.5 border border-[#1e293b] cursor-pointer hover:bg-[#1e293b] transition-colors">
              <p class="text-xs text-gray-400 truncate">{r.label}</p>
              <p class="text-[9px] text-gray-600 font-mono">{countdown(r.timestamp)}</p>
            </div>
          {/each}
        </div>
      {/if}
    </div>

    <!-- Active Fleets -->
    <div>
      <h3 class="text-[10px] font-semibold text-gray-500 uppercase tracking-wider mb-2">Active Fleets</h3>
      {#if fleets.length === 0}
        <p class="text-xs text-gray-500">No active fleets</p>
      {:else}
        <div class="space-y-1">
          {#each fleets.slice(0, 5) as m}
            <!-- svelte-ignore a11y_click_events_have_key_events a11y_interactive_supports_focus -->
            <div role="button" tabindex="0" onclick={() => onNavigate?.('fleet')} class="bg-[#0a0e1a] rounded px-2 py-1.5 border border-[#1e293b] cursor-pointer hover:bg-[#1e293b] transition-colors">
              <p class="text-xs text-gray-400 truncate">{m.mission_type} → <button onclick={(e) => { e.stopPropagation(); gotoCoords(m.target_galaxy, m.target_system); }} class="text-blue-400 hover:text-blue-300 cursor-pointer">{fmtCoords(m.target_galaxy, m.target_system, m.target_position)}</button></p>
              <p class="text-[9px] text-gray-600 font-mono">{countdown(m.arrival_time, 'Returning')}</p>
            </div>
          {/each}
        </div>
      {/if}
    </div>
  </div>
</aside>
