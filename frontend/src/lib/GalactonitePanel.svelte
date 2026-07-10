<script lang="ts">
  import { api, type GalactoniteData, type PlanetView } from './api'
  import { fmtNum } from './format'

  let { planetId, planet }: { planetId: string; planet?: PlanetView } = $props()

  let data = $state<GalactoniteData | null>(null)
  let loading = $state(true)
  let error = $state('')
  let upgrading = $state<string | null>(null)
  let message = $state('')
  let selectedFuseIds = $state<string[]>([])
  let fusing = $state(false)
  let convertMetal = $state(0)
  let convertCrystal = $state(0)
  let convertGas = $state(0)
  let converting = $state(false)
  let buyAmount = $state(100)
  let buyCurrency = $state('dm')
  let buying = $state(false)
  let activeTab = $state<'inventory' | 'equipped' | 'fusion' | 'energy'>('inventory')
  let categoryFilter = $state<string>('All')
  const tabs: ('inventory' | 'equipped' | 'fusion' | 'energy')[] = ['inventory', 'equipped', 'fusion', 'energy']

  async function load() {
    loading = true
    error = ''
    try {
      data = await api.getGalactonites()
    } catch (e: any) {
      error = e.message
    } finally {
      loading = false
    }
  }

  async function handleEquip(galactoniteId: string) {
    try {
      await api.equipGalactonite(galactoniteId, planetId)
      message = 'Equipped'
      await load()
    } catch (e: any) {
      message = e.message
    }
  }

  async function handleUnequip(galactoniteId: string) {
    if (!confirm('Unequipping costs 1 Dark Matter. Continue?')) return
    try {
      await api.unequipGalactonite(galactoniteId)
      message = 'Unequipped'
      await load()
    } catch (e: any) {
      message = e.message
    }
  }

  async function handleUpgrade(galactoniteId: string) {
    upgrading = galactoniteId
    try {
      await api.upgradeGalactonite(galactoniteId)
      message = 'Upgraded!'
      await load()
    } catch (e: any) {
      message = e.message
    } finally {
      upgrading = null
    }
  }

  async function handleFuse() {
    if (selectedFuseIds.length !== 3) return
    fusing = true
    try {
      const result = await api.fuseGalactonite(selectedFuseIds)
      message = `Fused into ${typeLabel(result.type_name)} ${starsDisplay(result.stars)}!`
      selectedFuseIds = []
      await load()
    } catch (e: any) {
      message = e.message
    } finally {
      fusing = false
    }
  }

  async function handleConvert() {
    if (!planetId) return
    converting = true
    try {
      await api.convertResources(planetId, convertMetal, convertCrystal, convertGas)
      message = 'Resources converted!'
      convertMetal = 0
      convertCrystal = 0
      convertGas = 0
      await load()
    } catch (e: any) {
      message = e.message
    } finally {
      converting = false
    }
  }

  async function handleBuyEnergy() {
    buying = true
    try {
      await api.buyCrystalEnergy(buyAmount, buyCurrency)
      message = `Purchased ${buyAmount} Crystal Energy!`
      await load()
    } catch (e: any) {
      message = e.message
    } finally {
      buying = false
    }
  }

  function typeLabel(t: string): string {
    const map: Record<string, string> = {
      metal_galactonite: 'Metal', blue_galactonite: 'Crystal', cloud_galactonite: 'Gas',
      flaming_galactonite: 'Attack', shield_galactonite: 'Shield', concentrated_galactonite: 'Armor',
      pink_galactonite: 'Pink', indigo_galactonite: 'Indigo', aqua_galactonite: 'Aqua',
      rouge_galactonite: 'Rouge', orange_galactonite: 'Orange', yellow_galactonite: 'Yellow',
      beige_galactonite: 'Beige', purple_galactonite: 'Purple', cobalt_galactonite: 'Cobalt',
      sapphire_galactonite: 'Sapphire', aegis_galactonite: 'Aegis', vortex_galactonite: 'Vortex',
      void_galactonite: 'Void',
    }
    return map[t] || t
  }

  function starsDisplay(s: number): string {
    return '★'.repeat(s) + '☆'.repeat(Math.max(0, 6 - s))
  }

  const shardSeeds: Record<string, { primary: number; secondary: number; label1: string; label2: string }> = {
    metal_galactonite:        { primary: 25, secondary: 0, label1: 'Metal Prod', label2: '' },
    blue_galactonite:         { primary: 28, secondary: 0, label1: 'Crystal Prod', label2: '' },
    cloud_galactonite:        { primary: 39, secondary: 0, label1: 'Gas Prod', label2: '' },
    flaming_galactonite:      { primary: 11, secondary: 0, label1: 'Attack', label2: '' },
    shield_galactonite:       { primary: 5.5, secondary: 0, label1: 'Shield', label2: '' },
    concentrated_galactonite: { primary: 12, secondary: 0, label1: 'Armor', label2: '' },
    pink_galactonite:         { primary: 7, secondary: 2, label1: 'Armor', label2: 'Gas Cost' },
    indigo_galactonite:       { primary: 20, secondary: 7, label1: 'Metal Prod', label2: 'Armor' },
    aqua_galactonite:         { primary: 25, secondary: 2, label1: 'Crystal Prod', label2: 'Explore' },
    rouge_galactonite:        { primary: 5, secondary: 2, label1: 'Shield', label2: 'Explore' },
    orange_galactonite:       { primary: 7, secondary: 2, label1: 'Armor', label2: 'Speed' },
    yellow_galactonite:       { primary: 30, secondary: 2, label1: 'Gas Prod', label2: 'Speed' },
    beige_galactonite:        { primary: 15, secondary: 7, label1: 'Shield', label2: 'Gas Cost' },
    purple_galactonite:       { primary: 11, secondary: 2, label1: 'Attack', label2: 'Speed' },
    cobalt_galactonite:       { primary: 7, secondary: 5, label1: 'Armor', label2: 'Attack' },
    sapphire_galactonite:     { primary: 15, secondary: 5, label1: 'Crystal Prod', label2: 'Shield' },
    aegis_galactonite:       { primary: 10, secondary: 3, label1: 'Armor', label2: 'Shield' },
    vortex_galactonite:       { primary: 8, secondary: 3, label1: 'Attack', label2: 'Speed' },
    void_galactonite:         { primary: 10, secondary: 0, label1: 'All Resources', label2: '' },
  }

  function computeEffect(seed: number, stars: number, level: number): number {
    return Math.floor(seed * (1 + 2 * (stars - 1)) + seed * (stars + 1) * (level - 1) / 6)
  }

  function effectText(typeName: string, stars: number, level: number): string {
    const def = shardSeeds[typeName]
    if (!def) return ''
    const p = computeEffect(def.primary, stars, level)
    let text = `${def.label1} +${p}%`
    if (def.secondary > 0) {
      const s = computeEffect(def.secondary, stars, level)
      text += ` / ${def.label2} +${s}%`
    }
    return text
  }

  function upgradeCost(stars: number, currentLevel: number): number {
    const bases: Record<number, number> = { 1: 10, 2: 40, 3: 265, 4: 1080, 5: 4000, 6: 30000 }
    const base = bases[stars]
    if (!base) return 999999999
    return Math.floor(base * Math.pow(2.5, currentLevel))
  }

  function categoryOf(typeName: string): string {
    const resource = ['metal_galactonite', 'blue_galactonite', 'cloud_galactonite']
    const combat = ['flaming_galactonite', 'shield_galactonite', 'concentrated_galactonite']
    const special = ['sapphire_galactonite', 'aegis_galactonite', 'vortex_galactonite', 'void_galactonite']
    if (resource.includes(typeName)) return 'Resource'
    if (combat.includes(typeName)) return 'Combat'
    if (special.includes(typeName)) return 'Special'
    return 'Hybrid'
  }

  $effect(() => { load() })
</script>

<div class="galactonite-panel">
  <div class="flex justify-between items-center mb-4">
    <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">Galactonites</h2>
    {#if data}
      <span class="text-sm text-cyan-400">Energy: {fmtNum(data.crystal_energy)}</span>
    {/if}
  </div>

  {#if error}
    <div class="bg-red-900/50 text-red-400 p-3 rounded mb-4 text-sm">{error}</div>
  {/if}

  {#if message}
    <div class="bg-green-900/50 text-green-400 p-3 rounded mb-4 text-sm">{message}</div>
  {/if}

  {#if loading}
    <div class="text-gray-500 text-center py-8 text-sm">Loading...</div>
  {:else if data}
    <!-- Tab bar -->
    <div class="flex gap-1 mb-4 border-b border-[#1e293b] pb-2">
      {#each tabs as tab}
        <button
          onclick={() => activeTab = tab}
          class="text-xs px-3 py-1 rounded {activeTab === tab ? 'bg-blue-600 text-white' : 'text-gray-400 hover:text-white'}"
        >{tab.charAt(0).toUpperCase() + tab.slice(1)}</button>
      {/each}
    </div>

    {#if activeTab === 'inventory'}
      <!-- Category filters -->
      <div class="flex gap-1 mb-3 flex-wrap">
        {#each ['All', 'Resource', 'Combat', 'Hybrid', 'Special'] as cat}
          <button
            onclick={() => categoryFilter = cat}
            class="text-xs px-2 py-0.5 rounded {categoryFilter === cat ? 'bg-gray-600 text-white' : 'bg-[#0a0e1a] text-gray-500 border border-[#1e293b] hover:text-gray-300'}"
          >{cat}</button>
        {/each}
      </div>

      <div class="mb-2 text-xs text-gray-500">Equipping to: {planetId.slice(0, 8)}</div>

      <div class="mb-4">
        <h3 class="text-sm font-semibold text-gray-300 mb-2">Inventory ({data.inventory.length})</h3>
        {#if data.inventory.length === 0}
          <div class="text-gray-600 text-sm">No Galactonites owned.</div>
        {:else}
          <div class="grid grid-cols-2 gap-2">
            {#each data.inventory.filter(g => categoryFilter === 'All' || categoryOf(g.type_name) === categoryFilter) as g}
              <div class="bg-[#0a0e1a] border border-[#1e293b] rounded p-3 {g.equipped ? 'opacity-50' : ''}">
                <div class="flex justify-between items-start">
                  <span class="text-white text-sm font-medium">{typeLabel(g.type_name)}</span>
                  <span class="text-yellow-400 text-xs">{starsDisplay(g.stars)}</span>
                </div>
                <div class="text-xs text-gray-500 mt-0.5">Lv.{g.level}</div>
                <div class="text-xs text-green-400 mt-0.5">{effectText(g.type_name, g.stars, g.level)}</div>
                {#if g.equipped}
                  <div class="flex gap-1 mt-2">
                    <button
                      onclick={() => handleUnequip(g.id)}
                      class="text-xs bg-yellow-700 hover:bg-yellow-600 text-white px-2 py-1 rounded"
                    >Unequip</button>
                  </div>
                {:else}
                  <div class="flex gap-1 mt-2">
                    <button
                      onclick={() => handleEquip(g.id)}
                      class="text-xs bg-blue-600 hover:bg-blue-700 text-white px-2 py-1 rounded"
                    >Equip</button>
                    <button
                      onclick={() => handleUpgrade(g.id)}
                      disabled={upgrading === g.id || g.level >= 11}
                      class="text-xs bg-purple-700 hover:bg-purple-600 disabled:bg-gray-700 text-white px-2 py-1 rounded"
                      title="Cost: {fmtNum(upgradeCost(g.stars, g.level))} CE"
                    >{upgrading === g.id ? '...' : `Lv.${g.level+1}`}</button>
                  </div>
                {/if}
              </div>
            {/each}
          </div>
        {/if}
      </div>
    {:else if activeTab === 'equipped'}
      <div class="mb-4">
        <div class="flex items-center justify-between mb-2">
          <h3 class="text-sm font-semibold text-gray-300">Equipped on Current Planet</h3>
          {#if planet}
            {@const grcLv = planet.galactonite_research_center_level}
            {@const slotsMax = 1 + Math.floor(grcLv / 2)}
            {@const slotsUsed = data.equipped.filter(s => s.planet_id === planetId).length}
            <span class="text-xs text-cyan-400 font-mono">
              {slotsUsed}/{slotsMax} slots
              {#if grcLv > 0}
                <span class="text-gray-600"> · GRC Lv.{grcLv}</span>
              {/if}
            </span>
          {/if}
        </div>
        {#if data.inventory.filter(g => g.equipped).length === 0}
          <div class="text-gray-600 text-sm">Nothing equipped on this planet.</div>
        {:else}
          <div class="grid grid-cols-2 gap-2">
            {#each data.inventory.filter(g => g.equipped) as g}
              <div class="bg-[#0a0e1a] border border-green-700 rounded p-3">
                <div class="flex justify-between items-start">
                  <span class="text-white text-sm font-medium">{typeLabel(g.type_name)}</span>
                  <span class="text-yellow-400 text-xs">{starsDisplay(g.stars)}</span>
                </div>
                <div class="text-xs text-gray-500 mt-0.5">Lv.{g.level}</div>
                <div class="text-xs text-green-400 mt-0.5">{effectText(g.type_name, g.stars, g.level)}</div>
                <div class="flex gap-1 mt-2">
                  <button
                    onclick={() => handleUnequip(g.id)}
                    class="text-xs bg-yellow-700 hover:bg-yellow-600 text-white px-2 py-1 rounded"
                  >Unequip</button>
                </div>
              </div>
            {/each}
          </div>
        {/if}
      </div>
    {:else if activeTab === 'fusion'}
      <!-- Fusion (same as before) -->
      <div class="mb-6">
        <h3 class="text-sm font-semibold text-gray-300 mb-2">Fusion</h3>
        <p class="text-xs text-gray-500 mb-2">Select 3 identical Galactonites (same type and stars) to fuse into a higher-star shard.</p>

        {#if data.inventory.filter(g => !g.equipped).length === 0}
          <div class="text-gray-600 text-sm">No unequipped Galactonites available for fusion.</div>
        {:else}
          <div class="grid grid-cols-3 gap-1 mb-3">
            {#each data.inventory.filter(g => !g.equipped) as g}
              <button
                onclick={() => {
                  const idx = selectedFuseIds.indexOf(g.id)
                  if (idx >= 0) {
                    selectedFuseIds = selectedFuseIds.filter(id => id !== g.id)
                  } else if (selectedFuseIds.length < 3) {
                    selectedFuseIds = [...selectedFuseIds, g.id]
                  }
                }}
                class="text-xs p-2 rounded border text-left {selectedFuseIds.includes(g.id) ? 'bg-blue-800 border-blue-500 text-white' : 'bg-[#0a0e1a] border-[#1e293b] text-gray-400 hover:border-gray-600'}"
              >
                <div class="font-medium">{typeLabel(g.type_name)}</div>
                <div class="text-yellow-400">{starsDisplay(g.stars)}</div>
                <div class="text-gray-500">Lv.{g.level}</div>
              </button>
            {/each}
          </div>

          <button
            onclick={handleFuse}
            disabled={selectedFuseIds.length !== 3 || fusing}
            class="text-xs bg-red-700 hover:bg-red-600 disabled:bg-gray-700 text-white px-3 py-1 rounded"
          >
            {fusing ? 'Fusing...' : `Fuse ${selectedFuseIds.length}/3`}
          </button>
          {#if selectedFuseIds.length === 3}
            {@const first = data.inventory.find(g => g.id === selectedFuseIds[0])}
            {#if first && first.stars < 6}
              <div class="mt-3 p-3 bg-[#0a0e1a] border border-yellow-600 rounded">
                <p class="text-xs text-gray-400 mb-1">Fusion Result Preview</p>
                <div class="flex items-center gap-2">
                  <span class="text-sm font-medium text-white">{typeLabel(first.type_name)}</span>
                  <span class="text-yellow-400 text-xs">{starsDisplay(first.stars + 1)}</span>
                </div>
                <p class="text-xs text-gray-500 mt-1">Stars: {first.stars} → {first.stars + 1}</p>
              </div>
            {/if}
          {/if}
        {/if}
      </div>
    {:else if activeTab === 'energy'}
      <div class="mb-4 p-3 bg-[#0a0e1a] border border-cyan-700 rounded">
        <span class="text-sm text-cyan-400 font-semibold">Crystal Energy: {fmtNum(data.crystal_energy)}</span>
      </div>
      <!-- Convert Resources -->
      <div class="mb-6">
        <h3 class="text-sm font-semibold text-gray-300 mb-2">Convert Resources</h3>
        <p class="text-xs text-gray-500 mb-2">CE = metal/1000 + crystal/500 + gas/200</p>
        <div class="grid grid-cols-3 gap-2 mb-2">
          <div>
            <label class="block text-xs text-gray-400">Metal</label>
            <input type="number" bind:value={convertMetal} min="0" step="1000"
              class="w-full bg-[#0a0e1a] border border-[#1e293b] rounded px-2 py-1 text-white text-sm" />
          </div>
          <div>
            <label class="block text-xs text-gray-400">Crystal</label>
            <input type="number" bind:value={convertCrystal} min="0" step="500"
              class="w-full bg-[#0a0e1a] border border-[#1e293b] rounded px-2 py-1 text-white text-sm" />
          </div>
          <div>
            <label class="block text-xs text-gray-400">Gas</label>
            <input type="number" bind:value={convertGas} min="0" step="200"
              class="w-full bg-[#0a0e1a] border border-[#1e293b] rounded px-2 py-1 text-white text-sm" />
          </div>
        </div>
        <div class="text-xs text-gray-500 mb-2">
          Yields: ~{fmtNum(Math.floor(convertMetal/1000 + convertCrystal/500 + convertGas/200))} CE
        </div>
        <button
          onclick={handleConvert}
          disabled={converting || (convertMetal === 0 && convertCrystal === 0 && convertGas === 0)}
          class="text-xs bg-green-700 hover:bg-green-600 disabled:bg-gray-700 text-white px-3 py-1 rounded"
        >{converting ? 'Converting...' : 'Convert'}</button>
      </div>

      <!-- Buy Crystal Energy -->
      <div class="mb-6">
        <h3 class="text-sm font-semibold text-gray-300 mb-2">Buy Crystal Energy</h3>
        <div class="flex gap-2 items-center mb-2">
          <select bind:value={buyAmount}
            class="bg-[#0a0e1a] border border-[#1e293b] rounded px-2 py-1 text-white text-sm">
            {#if buyCurrency === 'dm'}
              <option value={100}>100 CE (10 DM)</option>
              <option value={600}>600 CE (50 DM)</option>
              <option value={3000}>3000 CE (200 DM)</option>
            {:else}
              <option value={2000}>2000 CE (500 Credits)</option>
              <option value={10000}>10000 CE (2000 Credits)</option>
            {/if}
          </select>
          <select bind:value={buyCurrency}
            class="bg-[#0a0e1a] border border-[#1e293b] rounded px-2 py-1 text-white text-sm">
            <option value="dm">Dark Matter</option>
            <option value="credits">Credits</option>
          </select>
        </div>
        <button
          onclick={handleBuyEnergy}
          disabled={buying}
          class="text-xs bg-yellow-700 hover:bg-yellow-600 disabled:bg-gray-700 text-white px-3 py-1 rounded"
        >{buying ? 'Buying...' : 'Buy'}</button>
      </div>
    {/if}
  {/if}
</div>

<style>
  .galactonite-panel {
    min-height: 200px;
  }
</style>
