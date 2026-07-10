<script lang="ts">
  import { api } from './api'
  import { fmtNum } from './format'

  let { category }: { category: 'intro' | 'daily' | 'gef' } = $props()

  let quests = $state<any[]>([])
  let loading = $state(true)
  let error = $state('')
  let claiming = $state<Record<string, boolean>>({})

  function categoryLabel(cat: string): string {
    if (cat === 'intro') return 'Intro Missions'
    if (cat === 'daily') return 'Daily Tasks'
    return 'G.E.F. Campaign'
  }

  function rewardIcon(type: string): string {
    if (type === 'metal') return '🪨'
    if (type === 'crystal') return '🔮'
    if (type === 'gas') return '💨'
    if (type === 'dark_matter') return '💎'
    return '🎁'
  }

  $effect(() => {
    loading = true
    error = ''
    api.getQuests(category).then(data => {
      quests = data
      loading = false
    }).catch(e => {
      error = e.message
      loading = false
    })
  })

  async function handleClaim(questId: string) {
    claiming[questId] = true
    try {
      await api.claimQuest(questId)
      quests = quests.map(q => q.id === questId ? { ...q, claimed: true } : q)
    } catch (e: any) {
      error = e.message
    } finally {
      claiming[questId] = false
    }
  }

  function isUnlocked(q: any, idx: number): boolean {
    if (category !== 'intro') return true
    if (idx === 0) return true
    return quests.slice(0, idx).every(prev => prev.claimed)
  }
</script>

<div class="space-y-3">
  <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">{categoryLabel(category)}</h2>

  {#if error}
    <div class="bg-red-900/50 text-red-400 p-2 rounded text-sm">{error}</div>
  {/if}

  {#if loading}
    <div class="text-gray-500 text-xs text-center py-8">Loading quests...</div>
  {:else if quests.length === 0}
    <div class="text-gray-500 text-xs text-center py-4">No quests available.</div>
  {:else}
    <div class="space-y-2">
      {#each quests as q, idx}
        {@const unlocked = isUnlocked(q, idx)}
        <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-2 {unlocked ? '' : 'opacity-50'}">
          <div class="flex items-start justify-between gap-2">
            <div class="flex-1 min-w-0">
              <h3 class="text-sm font-medium text-white">{q.title || q.name}</h3>
              {#if q.description}
                <p class="text-xs text-gray-400 mt-0.5">{q.description}</p>
              {/if}
            </div>
            <div class="shrink-0">
              {#if q.claimed}
                <span class="text-green-400 text-lg">✓</span>
              {:else if !unlocked}
                <span class="text-gray-600 text-lg">🔒</span>
              {:else if q.completed}
                <button
                  onclick={() => handleClaim(q.id)}
                  disabled={claiming[q.id]}
                  class="bg-green-600 hover:bg-green-700 disabled:bg-green-800/50 text-white px-3 py-1 rounded text-xs font-medium transition-colors"
                >
                  {claiming[q.id] ? '...' : 'Claim'}
                </button>
              {:else}
                <span class="text-yellow-500 text-[10px] font-medium">In Progress</span>
              {/if}
            </div>
          </div>

          {#if q.progress !== undefined && q.target_value !== undefined}
            <div>
              <div class="flex justify-between text-[10px] text-gray-500 mb-1">
                <span>{q.progress} / {q.target_value}</span>
                <span>{Math.round((q.progress / q.target_value) * 100)}%</span>
              </div>
              <div class="w-full bg-[#1e293b] rounded-full h-1.5">
                <div
                  class="bg-blue-500 h-1.5 rounded-full transition-all duration-300"
                  style="width: {Math.min(100, (q.progress / q.target_value) * 100)}%"
                ></div>
              </div>
            </div>
          {/if}

          {#if q.rewards && q.rewards.length > 0}
            <div class="flex flex-wrap gap-1.5">
              {#each q.rewards as r}
                <span class="text-[10px] bg-[#1e293b] text-gray-300 px-1.5 py-0.5 rounded flex items-center gap-1">
                  <span>{rewardIcon(r.type)}</span>
                  <span>{r.amount ? fmtNum(r.amount) : r.amount}</span>
                </span>
              {/each}
            </div>
          {/if}
        </div>
      {/each}
    </div>
  {/if}
</div>
