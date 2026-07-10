<script lang="ts">
  import { lockBody } from './timers.svelte'

  let step = $state(1)
  let dismissed = $state(false)
  let done = $state(false)

  const STORAGE_KEY = 'galaxy_empire_tutorial_done'

  $effect(() => {
    done = localStorage.getItem(STORAGE_KEY) === 'true'
  })

  function next() {
    if (step < 4) {
      step++
    } else {
      complete()
    }
  }

  function prev() {
    if (step > 1) step--
  }

  function skip() {
    dismissed = true
  }

  function complete() {
    localStorage.setItem(STORAGE_KEY, 'true')
    done = true
  }

  $effect(() => {
    lockBody(!done && !dismissed)
  })

  const steps = [
    {
      title: 'Welcome to Galaxy Empire',
      subtitle: 'Your interstellar journey begins here',
      icon: '🚀',
      body: 'Build a thriving space empire from a single planet. Expand your colonies, construct a mighty fleet, research advanced technologies, and conquer the galaxy. Forge alliances, explore the unknown, and rise to become the dominant power in the universe.',
    },
    {
      title: 'Your Planet',
      subtitle: 'Resources & Construction',
      icon: '🌍',
      body: 'Your home planet generates three key resources: Metal, Crystal, and Gas. Build extractors to boost production, construct Solar Plants and Fusion Reactors for energy, and use your Shipyard to manufacture ships and defenses. Each building unlocks new capabilities — plan your build order wisely.',
    },
    {
      title: 'Your Fleet',
      subtitle: 'Ships & Missions',
      icon: '🚀',
      body: 'Build combat ships to attack enemies, cargo ships to plunder resources, colony ships to settle new worlds, and recyclers to harvest debris fields after battle. Send fleets on missions across the galaxy — attack, transport, colonize, or spy. Remember to fleetsave your resources before logging off!',
    },
    {
      title: 'Research & Progression',
      subtitle: 'Technology & Power',
      icon: '🔬',
      body: 'Advance through the tech tree to unlock stronger ships, faster drives, and deeper intel. Hire Commanders for empire-wide buffs like the Geologist (+10% production) or Construction Commander (-10% build time). Equip Galactonites for late-game vertical progression. The path to dominance is paved with research!',
    },
  ]
</script>

{#if !done && !dismissed}
  <div class="fixed inset-0 z-50 flex items-center justify-center p-4">
    <div class="absolute inset-0 bg-black/70 backdrop-blur-sm" onclick={skip} />
    <div class="relative bg-[#111827] rounded-xl border border-[#1e293b] shadow-2xl w-full max-w-lg overflow-hidden animate-fade-in">
      <button onclick={skip} class="absolute top-3 right-3 text-gray-500 hover:text-gray-300 transition-colors z-10">
        <svg class="w-5 h-5" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/></svg>
      </button>

      <div class="p-6 sm:p-8">
        <div class="flex justify-center mb-6">
          <div class="w-16 h-16 rounded-2xl bg-blue-600/20 flex items-center justify-center text-3xl">
            {steps[step - 1].icon}
          </div>
        </div>

        <div class="text-center mb-6">
          <h2 class="text-xl font-bold text-white mb-1">{steps[step - 1].title}</h2>
          <p class="text-sm text-blue-400 font-medium">{steps[step - 1].subtitle}</p>
        </div>

        <p class="text-sm text-gray-400 leading-relaxed text-center">
          {steps[step - 1].body}
        </p>
      </div>

      <div class="flex items-center justify-center gap-2 mb-4">
        {#each steps as _, i}
          <button
            onclick={() => step = i + 1}
            class="w-2 h-2 rounded-full transition-all {step === i + 1 ? 'bg-blue-500 w-5' : 'bg-gray-600 hover:bg-gray-500'}"
          />
        {/each}
      </div>

      <div class="flex items-center justify-between px-6 sm:px-8 pb-6">
        <button
          onclick={prev}
          disabled={step === 1}
          class="px-4 py-2 text-sm font-medium text-gray-400 hover:text-white disabled:opacity-30 disabled:cursor-not-allowed transition-colors"
        >Back</button>

        <div class="flex gap-2">
          <button
            onclick={skip}
            class="px-4 py-2 text-sm font-medium text-gray-500 hover:text-gray-300 transition-colors"
          >Skip</button>
          <button
            onclick={next}
            class="px-5 py-2 text-sm font-medium bg-blue-600 hover:bg-blue-700 text-white rounded-lg transition-all active:scale-95"
          >
            {step < 4 ? 'Next' : 'Get Started'}
          </button>
        </div>
      </div>
    </div>
  </div>
{/if}

<style>
  @keyframes fade-in {
    from { opacity: 0; transform: scale(0.95) translateY(8px); }
    to { opacity: 1; transform: scale(1) translateY(0); }
  }
  .animate-fade-in {
    animation: fade-in 0.25s ease-out;
  }
</style>
