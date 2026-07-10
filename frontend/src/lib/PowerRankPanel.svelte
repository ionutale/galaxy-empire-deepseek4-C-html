<script lang="ts">
  import { api } from './api'
  import { fmtNum } from './format'

  let score = $state<number | null>(null)
  let error = $state('')

  $effect(() => {
    api.getPowerRank()
      .then(r => score = r.score)
      .catch(e => error = e.message)
  })
</script>

<div>
  <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide mb-4">Power Rank</h2>
  {#if error}
    <p class="text-red-400 text-sm">{error}</p>
  {:else if score === null}
    <p class="text-gray-500 text-sm">Loading...</p>
  {:else}
    <div class="bg-[#0a0e1a] rounded-lg p-6 border border-[#1e293b] text-center">
      <div class="text-4xl font-bold text-yellow-400">{fmtNum(score)}</div>
      <div class="text-xs text-gray-500 mt-1">Power Score</div>
    </div>
    <p class="text-xs text-gray-500 mt-3">
      Formula: (Metal + 2×Crystal + 3×Gas) / 1000. Recalculated every 10 minutes.
      You can attack targets within 10× your score.
    </p>
  {/if}
</div>
