<script lang="ts">
  import { api } from './api'

  let cooldownUntil = $state<Date | null>(null)
  let remaining = $state(0)
  let lastReward = $state<any>(null)
  let error = $state('')
  let claiming = $state(false)
  let loading = $state(true)

  const TIERS = ['Common', 'Rare', 'Epic', 'Legendary']
  let currentTier = $state(0)

  $effect(() => {
    loading = true
    checkStatus()
  })

  async function checkStatus() {
    try {
      const res = await api.claimDailyGift()
      if (res.reward) {
        lastReward = res.reward
        cooldownUntil = null
        remaining = 0
      }
    } catch (e: any) {
      const msg = e.message || ''
      const match = msg.match(/try again in (\d+)/)
      if (match) {
        const seconds = parseInt(match[1])
        cooldownUntil = new Date(Date.now() + seconds * 1000)
        remaining = seconds
      } else {
        error = msg
      }
    } finally {
      loading = false
    }
  }

  async function handleClaim() {
    claiming = true
    error = ''
    try {
      const res = await api.claimDailyGift()
      lastReward = res.reward
      cooldownUntil = null
      remaining = 0
      currentTier = Math.min(currentTier + 1, TIERS.length - 1)
    } catch (e: any) {
      const msg = e.message || ''
      const match = msg.match(/try again in (\d+)/)
      if (match) {
        const seconds = parseInt(match[1])
        cooldownUntil = new Date(Date.now() + seconds * 1000)
        remaining = seconds
      } else {
        error = msg
      }
    } finally {
      claiming = false
    }
  }

  $effect(() => {
    if (!cooldownUntil) return
    const interval = setInterval(() => {
      const now = Date.now()
      const diff = Math.max(0, Math.floor((cooldownUntil!.getTime() - now) / 1000))
      remaining = diff
      if (diff <= 0) {
        cooldownUntil = null
        remaining = 0
      }
    }, 1000)
    return () => clearInterval(interval)
  })

  function formatTime(s: number): string {
    const h = Math.floor(s / 3600)
    const m = Math.floor((s % 3600) / 60)
    return `${h}h ${m}m`
  }
</script>

<div class="space-y-3">
  <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">Daily Gift</h2>

  {#if error}
    <div class="bg-red-900/50 text-red-400 p-2 rounded text-sm">{error}</div>
  {/if}

  {#if loading}
    <div class="text-gray-500 text-xs text-center py-8">Loading...</div>
  {:else}
    <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-4 sm:p-5 text-center space-y-3">
      <div class="text-4xl">🎁</div>

      <div class="flex justify-center gap-1">
        {#each TIERS as tier, i}
          <span
            class="w-2.5 h-2.5 rounded-full transition-colors {i <= currentTier ? 'bg-yellow-400' : 'bg-[#1e293b]'}"
            title={tier}
          ></span>
        {/each}
      </div>

      <p class="text-xs text-gray-400">Tier: {TIERS[currentTier]}</p>

      {#if cooldownUntil && remaining > 0}
        <div class="text-lg font-mono text-yellow-400">{formatTime(remaining)}</div>
        <p class="text-xs text-gray-500">Next gift available</p>
      {:else}
        <button
          onclick={handleClaim}
          disabled={claiming}
          class="w-full bg-gradient-to-r from-yellow-600 to-orange-600 hover:from-yellow-500 hover:to-orange-500 disabled:from-yellow-800/50 disabled:to-orange-800/50 text-white py-2.5 rounded-lg text-sm font-bold transition-all active:scale-[0.98]"
        >
          {claiming ? 'Claiming...' : 'Claim Gift'}
        </button>
      {/if}

      {#if lastReward}
        <div class="bg-[#1e293b]/50 rounded p-2 text-xs space-y-1">
          <p class="text-gray-400">Last Reward</p>
          {#each Object.entries(lastReward) as [key, val]}
            <p class="text-white">{key}: {val as string}</p>
          {/each}
        </div>
      {/if}
    </div>
  {/if}
</div>
