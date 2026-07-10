<script lang="ts">
  import { api, type CommanderData } from './api'

  let data = $state<CommanderData | null>(null)
  let error = $state('')
  let hiring = $state<string | null>(null)

  const commanderNames: Record<string, string> = {
    geologist: 'Geologist',
    construction: 'Construction Commander',
    admiral: 'Admiral',
    engineer: 'Engineer',
    technocrat: 'Technocrat',
    guider: 'Guider',
  }

  const commanderDesc: Record<string, string> = {
    geologist: '+10% resource production per tier',
    construction: '-10% construction time per tier',
    admiral: '+1 fleet slot per tier',
    engineer: '+10% defense repair rate per tier',
    technocrat: '-10% research time per tier',
    guider: '+10% fleet speed per tier',
  }

  $effect(() => {
    api.getCommanders()
      .then(d => data = d)
      .catch(e => error = e.message)
  })

  async function hire(cmd: string, tier: number) {
    hiring = `${cmd}-${tier}`
    error = ''
    try {
      await api.hireCommander(cmd, tier)
      data = await api.getCommanders()
    } catch (e: any) {
      error = e.message
    } finally {
      hiring = null
    }
  }

  function timeLeft(expires: string): string {
    const ms = new Date(expires).getTime() - Date.now()
    if (ms <= 0) return 'Expired'
    const days = Math.floor(ms / 86400000)
    const hours = Math.floor((ms % 86400000) / 3600000)
    return `${days}d ${hours}h`
  }
</script>

<div>
  <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide mb-4">Commanders</h2>
  {#if error}
    <p class="text-red-400 text-sm mb-3">{error}</p>
  {/if}

  {#if data}
    <!-- Active Commanders -->
    <div class="mb-6">
      <h3 class="text-sm font-semibold text-gray-400 mb-2 uppercase tracking-wider">Active</h3>
      {#if Object.keys(data.active).length === 0}
        <p class="text-gray-500 text-sm">No active commanders.</p>
      {:else}
        <div class="space-y-2">
          {#each Object.entries(data.active) as [type, cmd]}
            <div class="bg-[#0a0e1a] rounded-lg p-3 border border-[#1e293b] flex justify-between items-center">
              <div>
                <span class="text-white font-medium">{commanderNames[type] || type}</span>
                <span class="text-yellow-400 text-xs ml-2">Tier {cmd.tier}</span>
                <div class="text-xs text-gray-500">{commanderDesc[type] || ''}</div>
              </div>
              <span class="text-xs text-gray-400">{timeLeft(cmd.expires_at)}</span>
            </div>
          {/each}
        </div>
      {/if}
    </div>

    <!-- Available Commanders -->
    <div>
      <h3 class="text-sm font-semibold text-gray-400 mb-2 uppercase tracking-wider">Available</h3>
      <div class="space-y-2">
        {#each Object.entries(data.available) as [type, tiers]}
          <div class="bg-[#0a0e1a] rounded-lg p-3 border border-[#1e293b]">
            <div class="text-white font-medium text-sm mb-2">{commanderNames[type] || type}</div>
            <div class="text-xs text-gray-500 mb-2">{commanderDesc[type] || ''}</div>
            <div class="flex gap-2">
              {#each Object.entries(tiers) as [tier, cost]}
                {@const isActive = data.active[type]?.tier === parseInt(tier)}
                <button
                  onclick={() => hire(type, parseInt(tier))}
                  disabled={isActive || hiring === `${type}-${tier}`}
                  class="px-2 py-1 text-xs rounded {isActive ? 'bg-green-800/50 text-green-400 cursor-default' : 'bg-blue-600 hover:bg-blue-700 disabled:bg-blue-800/50 text-white'}"
                >
                  {#if hiring === `${type}-${tier}`}
                    Hiring...
                  {:else if isActive}
                    Active
                  {:else}
                    T{tier} — {cost} DM
                  {/if}
                </button>
              {/each}
            </div>
          </div>
        {/each}
      </div>
    </div>
  {:else}
    <p class="text-gray-500 text-sm">Loading...</p>
  {/if}
</div>
