<script lang="ts">
  import { api, type FleetInventory, type DefenseInventory } from './api'
  import { SHIP_TYPES, SHIP_KEYS } from './shipTypes'

  const BUILDING_OPTIONS = [
    { value: 'metal_mine', label: 'Metal Mine' },
    { value: 'crystal_mine', label: 'Crystal Mine' },
    { value: 'gas_mine', label: 'Gas Mine' },
    { value: 'solar_plant', label: 'Solar Plant' },
    { value: 'fusion_reactor', label: 'Fusion Reactor' },
    { value: 'robotics', label: 'Robotics Facility' },
    { value: 'nanite_factory', label: 'Nanite Factory' },
    { value: 'shipyard', label: 'Shipyard' },
    { value: 'research_lab', label: 'Research Lab' },
    { value: 'radar', label: 'Radar Facility' },
    { value: 'alliance_depot', label: 'Alliance Depot' },
    { value: 'metal_storage', label: 'Metal Storage' },
    { value: 'crystal_storage', label: 'Crystal Storage' },
    { value: 'gas_storage', label: 'Gas Storage' },
    { value: 'moon_base', label: 'Moon Base' },
    { value: 'robotics_moon', label: 'Robotics (Moon)' },
    { value: 'shipyard_moon', label: 'Shipyard (Moon)' },
    { value: 'pioneer_lab', label: 'Pioneer Lab' },
    { value: 'eu_x_radar', label: 'EU-X Radar' },
    { value: 'wormhole_generator', label: 'Wormhole Generator' },
  ]

  const TECH_OPTIONS = [
    { value: 'energy_tech', label: 'Energy Tech' },
    { value: 'laser_tech', label: 'Laser Tech' },
    { value: 'ion_tech', label: 'Ion Tech' },
    { value: 'hyperspace_tech', label: 'Hyperspace Tech' },
    { value: 'plasma_tech', label: 'Plasma Tech' },
    { value: 'astrophysics', label: 'Astrophysics' },
    { value: 'computer_tech', label: 'Computer Tech' },
    { value: 'espionage_tech', label: 'Espionage Tech' },
    { value: 'ultra_temperature_tech', label: 'Ultra Temperature Tech' },
    { value: 'anti_gravity_tech', label: 'Anti-Gravity Tech' },
    { value: 'combustion_drive', label: 'Combustion Drive' },
    { value: 'impulse_drive', label: 'Impulse Drive' },
    { value: 'hyperspace_drive', label: 'Hyperspace Drive' },
    { value: 'weapons_tech', label: 'Weapons Tech' },
    { value: 'shielding_tech', label: 'Shielding Tech' },
    { value: 'strength_tech', label: 'Strength Tech' },
  ]

  const DEFENSE_TYPES = [
    { key: 'rocket_launcher', label: 'Rocket Launcher' },
    { key: 'light_laser', label: 'Light Laser' },
    { key: 'heavy_laser', label: 'Heavy Laser' },
    { key: 'ion_cannon', label: 'Ion Cannon' },
    { key: 'mk2_cannon', label: 'MK2 Cannon' },
    { key: 'plasma_cannon', label: 'Plasma Cannon' },
    { key: 'proton_cannon', label: 'Proton Cannon' },
    { key: 'small_shield_dome', label: 'Small Shield Dome' },
    { key: 'large_shield_dome', label: 'Large Shield Dome' },
  ]

  let activeTab = $state<'resources' | 'buildings' | 'user' | 'research' | 'fleet' | 'defense'>('resources')

  let planetId = $state('')
  let userId = $state('')
  let galaxy = $state(1)
  let system = $state(1)
  let position = $state(1)
  let username = $state('')
  let metal = $state(0)
  let crystal = $state(0)
  let gas = $state(0)
  let building = $state('metal_mine')
  let buildingLevel = $state(0)
  let darkMatter = $state(0)
  let credits = $state(0)
  let tech = $state('energy_tech')
  let techLevel = $state(0)

  let fleetShips = $state<Record<string, number>>(
    Object.fromEntries(SHIP_KEYS.map(k => [k, 0]))
  )
  let defenseUnits = $state<Record<string, number>>(
    Object.fromEntries(DEFENSE_TYPES.map(d => [d.key, 0]))
  )

  let error = $state('')
  let success = $state('')
  let loading = $state(false)

  const tabs: { id: typeof activeTab; label: string }[] = [
    { id: 'resources', label: 'Resources' },
    { id: 'buildings', label: 'Buildings' },
    { id: 'user', label: 'User' },
    { id: 'research', label: 'Research' },
    { id: 'fleet', label: 'Fleet' },
    { id: 'defense', label: 'Defense' },
  ]

  function showSuccess(msg: string) {
    success = msg
    error = ''
    setTimeout(() => success = '', 3000)
  }

  async function setResources() {
    loading = true; error = ''
    try {
      if (galaxy && system && position) {
        await api.adminSetResourcesByCoord(galaxy, system, position, metal, crystal, gas)
      } else if (planetId) {
        await api.adminSetResources(planetId, metal, crystal, gas)
      } else { error = 'Provide planet ID or coordinates'; loading = false; return }
      showSuccess('Resources set')
    } catch (e: any) { error = e.message }
    finally { loading = false }
  }

  async function setBuilding() {
    loading = true; error = ''
    try {
      if (galaxy && system && position) {
        await api.adminSetBuildingByCoord(galaxy, system, position, building, buildingLevel)
      } else if (planetId) {
        await api.adminSetBuilding(planetId, building, buildingLevel)
      } else { error = 'Provide planet ID or coordinates'; loading = false; return }
      showSuccess('Building set')
    } catch (e: any) { error = e.message }
    finally { loading = false }
  }

  async function setDarkMatterAction() {
    loading = true; error = ''
    try {
      if (username) {
        await api.adminSetDarkMatterByUsername(username, darkMatter)
      } else if (userId) {
        await api.adminSetDarkMatter(userId, darkMatter)
      } else { error = 'Provide username or user ID'; loading = false; return }
      showSuccess('Dark Matter set')
    } catch (e: any) { error = e.message }
    finally { loading = false }
  }

  async function setCreditsAction() {
    loading = true; error = ''
    try {
      if (username) {
        await api.adminSetCreditsByUsername(username, credits)
      } else if (userId) {
        await api.adminSetCredits(userId, credits)
      } else { error = 'Provide username or user ID'; loading = false; return }
      showSuccess('Credits set')
    } catch (e: any) { error = e.message }
    finally { loading = false }
  }

  async function setResearch() {
    loading = true; error = ''
    try {
      if (username) {
        await api.adminSetResearchByUsername(username, tech, techLevel)
      } else if (userId) {
        await api.adminSetResearch(userId, tech, techLevel)
      } else { error = 'Provide username or user ID'; loading = false; return }
      showSuccess('Research set')
    } catch (e: any) { error = e.message }
    finally { loading = false }
  }

  async function setFleet() {
    loading = true; error = ''
    try {
      const active: Record<string, number> = {}
      for (const [k, v] of Object.entries(fleetShips)) {
        if (v > 0) active[k] = v
      }
      if (galaxy && system && position) {
        await api.adminSetFleetByCoord(galaxy, system, position, active)
      } else if (planetId) {
        await api.adminSetFleet(planetId, active)
      } else { error = 'Provide planet ID or coordinates'; loading = false; return }
      showSuccess('Fleet set')
    } catch (e: any) { error = e.message }
    finally { loading = false }
  }

  async function setDefense() {
    loading = true; error = ''
    try {
      const active: Record<string, number> = {}
      for (const [k, v] of Object.entries(defenseUnits)) {
        if (v > 0) active[k] = v
      }
      if (galaxy && system && position) {
        await api.adminSetDefenseByCoord(galaxy, system, position, active)
      } else if (planetId) {
        await api.adminSetDefense(planetId, active)
      } else { error = 'Provide planet ID or coordinates'; loading = false; return }
      showSuccess('Defenses set')
    } catch (e: any) { error = e.message }
    finally { loading = false }
  }
</script>

<div class="space-y-3">
  <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">Admin Panel</h2>

  {#if error}
    <div class="bg-red-900/50 text-red-400 p-2 rounded text-sm">{error}</div>
  {/if}
  {#if success}
    <div class="bg-green-900/50 text-green-400 p-2 rounded text-sm">{success}</div>
  {/if}

  <div class="flex gap-1 flex-wrap">
    {#each tabs as tab}
      <button
        onclick={() => activeTab = tab.id}
        class="px-2.5 py-1.5 rounded text-xs font-medium transition-colors
          {activeTab === tab.id
            ? 'bg-blue-600/20 text-blue-400 border border-blue-600/30'
            : 'text-gray-400 hover:text-white border border-transparent'}"
      >
        {tab.label}
      </button>
    {/each}
  </div>

  {#if activeTab === 'resources'}
    <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-2">
      <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide">Set Planet Resources</h3>
      <div>
        <label class="text-xs text-gray-400 block mb-1">Planet ID <span class="text-gray-600">(or coords below)</span></label>
        <input bind:value={planetId} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
      </div>
      <div class="grid grid-cols-3 gap-2">
        <div>
          <label class="text-xs text-gray-400 block mb-1">Galaxy</label>
          <input type="number" min="1" max="9" bind:value={galaxy} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
        </div>
        <div>
          <label class="text-xs text-gray-400 block mb-1">System</label>
          <input type="number" min="1" max="499" bind:value={system} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
        </div>
        <div>
          <label class="text-xs text-gray-400 block mb-1">Position</label>
          <input type="number" min="1" max="21" bind:value={position} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
        </div>
      </div>
      <div class="grid grid-cols-3 gap-2">
        <div>
          <label class="text-xs text-gray-400 block mb-1">Metal</label>
          <input type="number" min="0" bind:value={metal} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
        </div>
        <div>
          <label class="text-xs text-gray-400 block mb-1">Crystal</label>
          <input type="number" min="0" bind:value={crystal} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
        </div>
        <div>
          <label class="text-xs text-gray-400 block mb-1">Gas</label>
          <input type="number" min="0" bind:value={gas} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
        </div>
      </div>
      <button onclick={setResources} disabled={loading}
        class="w-full bg-orange-600 hover:bg-orange-700 disabled:bg-orange-800/50 text-white py-1.5 rounded text-xs font-medium transition-colors">
        {loading ? 'Setting...' : 'Set Resources'}
      </button>
    </div>

  {:else if activeTab === 'buildings'}
    <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-2">
      <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide">Set Building Level</h3>
      <div>
        <label class="text-xs text-gray-400 block mb-1">Planet ID <span class="text-gray-600">(or coords below)</span></label>
        <input bind:value={planetId} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
      </div>
      <div class="grid grid-cols-3 gap-2">
        <div>
          <label class="text-xs text-gray-400 block mb-1">Galaxy</label>
          <input type="number" min="1" max="9" bind:value={galaxy} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
        </div>
        <div>
          <label class="text-xs text-gray-400 block mb-1">System</label>
          <input type="number" min="1" max="499" bind:value={system} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
        </div>
        <div>
          <label class="text-xs text-gray-400 block mb-1">Position</label>
          <input type="number" min="1" max="21" bind:value={position} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
        </div>
      </div>
      <div>
        <label class="text-xs text-gray-400 block mb-1">Building</label>
        <select bind:value={building} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs">
          {#each BUILDING_OPTIONS as opt}
            <option value={opt.value}>{opt.label}</option>
          {/each}
        </select>
      </div>
      <div>
        <label class="text-xs text-gray-400 block mb-1">Level</label>
        <input type="number" min="0" bind:value={buildingLevel} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
      </div>
      <button onclick={setBuilding} disabled={loading}
        class="w-full bg-orange-600 hover:bg-orange-700 disabled:bg-orange-800/50 text-white py-1.5 rounded text-xs font-medium transition-colors">
        {loading ? 'Setting...' : 'Set Building'}
      </button>
    </div>

  {:else if activeTab === 'user'}
    <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-2">
      <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide">User Management</h3>
      <div>
        <label class="text-xs text-gray-400 block mb-1">User ID <span class="text-gray-600">(or username below)</span></label>
        <input bind:value={userId} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
      </div>
      <div>
        <label class="text-xs text-gray-400 block mb-1">Username</label>
        <input bind:value={username} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
      </div>
      <div class="grid grid-cols-2 gap-2">
        <div>
          <label class="text-xs text-gray-400 block mb-1">Dark Matter</label>
          <input type="number" min="0" bind:value={darkMatter} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
          <button onclick={setDarkMatterAction} disabled={loading}
            class="w-full mt-1 bg-purple-600 hover:bg-purple-700 disabled:bg-purple-800/50 text-white py-1.5 rounded text-xs font-medium transition-colors">
            {loading ? 'Setting...' : 'Set DM'}
          </button>
        </div>
        <div>
          <label class="text-xs text-gray-400 block mb-1">Credits</label>
          <input type="number" min="0" bind:value={credits} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
          <button onclick={setCreditsAction} disabled={loading}
            class="w-full mt-1 bg-purple-600 hover:bg-purple-700 disabled:bg-purple-800/50 text-white py-1.5 rounded text-xs font-medium transition-colors">
            {loading ? 'Setting...' : 'Set Credits'}
          </button>
        </div>
      </div>
    </div>

  {:else if activeTab === 'research'}
    <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-2">
      <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide">Set Research Level</h3>
      <div>
        <label class="text-xs text-gray-400 block mb-1">User ID <span class="text-gray-600">(or username below)</span></label>
        <input bind:value={userId} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
      </div>
      <div>
        <label class="text-xs text-gray-400 block mb-1">Username</label>
        <input bind:value={username} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
      </div>
      <div>
        <label class="text-xs text-gray-400 block mb-1">Technology</label>
        <select bind:value={tech} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs">
          {#each TECH_OPTIONS as opt}
            <option value={opt.value}>{opt.label}</option>
          {/each}
        </select>
      </div>
      <div>
        <label class="text-xs text-gray-400 block mb-1">Level</label>
        <input type="number" min="0" bind:value={techLevel} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
      </div>
      <button onclick={setResearch} disabled={loading}
        class="w-full bg-orange-600 hover:bg-orange-700 disabled:bg-orange-800/50 text-white py-1.5 rounded text-xs font-medium transition-colors">
        {loading ? 'Setting...' : 'Set Research'}
      </button>
    </div>

  {:else if activeTab === 'fleet'}
    <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-2">
      <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide">Set Planet Fleet</h3>
      <div>
        <label class="text-xs text-gray-400 block mb-1">Planet ID <span class="text-gray-600">(or coords below)</span></label>
        <input bind:value={planetId} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
      </div>
      <div class="grid grid-cols-3 gap-2">
        <div>
          <label class="text-xs text-gray-400 block mb-1">Galaxy</label>
          <input type="number" min="1" max="9" bind:value={galaxy} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
        </div>
        <div>
          <label class="text-xs text-gray-400 block mb-1">System</label>
          <input type="number" min="1" max="499" bind:value={system} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
        </div>
        <div>
          <label class="text-xs text-gray-400 block mb-1">Position</label>
          <input type="number" min="1" max="21" bind:value={position} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
        </div>
      </div>
      <div class="grid grid-cols-2 gap-1">
        {#each SHIP_TYPES as ship}
          <div class="flex items-center gap-1">
            <span class="text-gray-400 text-xs w-20 truncate">{ship.label}</span>
            <input type="number" min="0" bind:value={fleetShips[ship.key]}
              class="flex-1 w-16 bg-[#0a0e1a] border border-[#1e293b] text-white text-center rounded py-1 text-xs" />
          </div>
        {/each}
      </div>
      <button onclick={setFleet} disabled={loading}
        class="w-full bg-orange-600 hover:bg-orange-700 disabled:bg-orange-800/50 text-white py-1.5 rounded text-xs font-medium transition-colors">
        {loading ? 'Setting...' : 'Set Fleet'}
      </button>
    </div>

  {:else if activeTab === 'defense'}
    <div class="bg-[#0a0e1a] border border-[#1e293b] rounded-lg p-3 space-y-2">
      <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide">Set Planet Defenses</h3>
      <div>
        <label class="text-xs text-gray-400 block mb-1">Planet ID <span class="text-gray-600">(or coords below)</span></label>
        <input bind:value={planetId} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
      </div>
      <div class="grid grid-cols-3 gap-2">
        <div>
          <label class="text-xs text-gray-400 block mb-1">Galaxy</label>
          <input type="number" min="1" max="9" bind:value={galaxy} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
        </div>
        <div>
          <label class="text-xs text-gray-400 block mb-1">System</label>
          <input type="number" min="1" max="499" bind:value={system} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
        </div>
        <div>
          <label class="text-xs text-gray-400 block mb-1">Position</label>
          <input type="number" min="1" max="21" bind:value={position} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1 text-xs" />
        </div>
      </div>
      <div class="grid grid-cols-2 gap-1">
        {#each DEFENSE_TYPES as def}
          <div class="flex items-center gap-1">
            <span class="text-gray-400 text-xs w-24 truncate">{def.label}</span>
            <input type="number" min="0" bind:value={defenseUnits[def.key]}
              class="flex-1 w-16 bg-[#0a0e1a] border border-[#1e293b] text-white text-center rounded py-1 text-xs" />
          </div>
        {/each}
      </div>
      <button onclick={setDefense} disabled={loading}
        class="w-full bg-orange-600 hover:bg-orange-700 disabled:bg-orange-800/50 text-white py-1.5 rounded text-xs font-medium transition-colors">
        {loading ? 'Setting...' : 'Set Defenses'}
      </button>
    </div>
  {/if}
</div>
