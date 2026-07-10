<script lang="ts">
  import { api, setOnUnauthorized, type User, type PlanetView } from './lib/api'
  import PlanetDashboard from './lib/PlanetDashboard.svelte'
  import GalaxyPanel from './lib/GalaxyPanel.svelte'
  import FleetPanel from './lib/FleetPanel.svelte'
  import MissionsPanel from './lib/MissionsPanel.svelte'
  import DashboardPanel from './lib/DashboardPanel.svelte'
  import FortressPanel from './lib/FortressPanel.svelte'
  import WormholePanel from './lib/WormholePanel.svelte'
  import EUXPanel from './lib/EUXPanel.svelte'
  import GalactonitePanel from './lib/GalactonitePanel.svelte'
  import AlliancePanel from './lib/AlliancePanel.svelte'
  import PowerRankPanel from './lib/PowerRankPanel.svelte'
  import CommanderPanel from './lib/CommanderPanel.svelte'
  import BattleReportsPanel from './lib/BattleReportsPanel.svelte'
  import SpyReportsPanel from './lib/SpyReportsPanel.svelte'
  import NebulaPanel from './lib/NebulaPanel.svelte'
  import AdminPanel from './lib/AdminPanel.svelte'
  import ProfilePage from './lib/ProfilePage.svelte'
  import InboxPanel from './lib/InboxPanel.svelte'
  import QuestPanel from './lib/QuestPanel.svelte'
  import DailyGiftPanel from './lib/DailyGiftPanel.svelte'
  import GEFPanel from './lib/GEFPanel.svelte'
  import Tutorial from './lib/Tutorial.svelte'
  import { fmtNum } from './lib/format'
  import { startClock } from './lib/timers.svelte'
  startClock()
  import LeftSidebar from './lib/LeftSidebar.svelte'
  import RightSidebar from './lib/RightSidebar.svelte'

  type View = 'login' | 'register' | 'planet' | 'galaxy' | 'fleet' | 'missions' | 'fortress' | 'wormhole' | 'eu-x' | 'galactonites' | 'alliance' | 'power-rank' | 'commanders' | 'battle-reports' | 'spy-reports' | 'nebula' | 'admin' | 'profile' | 'inbox' | 'quests' | 'daily-gift' | 'gef' | 'dashboard'

  let questCategory = $state<'intro' | 'daily' | 'gef'>('intro')

  function viewFromPath(): View {
    const p = window.location.pathname.replace('/', '') as View
    const valid: View[] = ['planet', 'galaxy', 'fleet', 'missions', 'dashboard', 'fortress', 'wormhole', 'eu-x', 'galactonites', 'alliance', 'power-rank', 'commanders', 'battle-reports', 'spy-reports', 'nebula', 'admin', 'profile', 'inbox', 'quests', 'daily-gift', 'gef']
    return valid.includes(p) ? p : 'login'
  }

  let view = $state<View>(viewFromPath())
  let showMore = $state(false)
  let showLeftMobile = $state(false)
  let showRightMobile = $state(false)
  let username = $state('')
  let password = $state('')
  let confirmPassword = $state('')
  let error = $state('')
  let loading = $state(false)
  let showPassword = $state(false)
  let showConfirmPassword = $state(false)
  let user = $state<User | null>(null)
  let planet = $state<PlanetView | null>(null)
  let planetList = $state<{ id: string; name?: string }[]>([])
  let currentGalaxy = $state(1)
  let currentSystem = $state(1)
  let activeCount = $state(0)

  $effect(() => {
    const onPopState = () => {
      const token = localStorage.getItem('token')
      if (!token) return
      const p = window.location.pathname.replace('/', '') as View
      const valid: View[] = ['planet', 'galaxy', 'fleet', 'missions', 'dashboard', 'fortress', 'wormhole', 'eu-x', 'galactonites', 'alliance', 'power-rank', 'commanders', 'battle-reports', 'spy-reports', 'nebula', 'admin', 'profile', 'inbox', 'quests', 'daily-gift', 'gef']
      if (valid.includes(hash)) view = hash
    }
    window.addEventListener('popstate', onPopState)
    return () => window.removeEventListener('popstate', onPopState)
  })

  // Shared planet polling (every 5s) — updates individual fields to avoid full re-render
  $effect(() => {
    if (!user || !planet?.id) return
    const interval = setInterval(async () => {
      try {
        const fresh = await api.getPlanet(planet!.id)
        if (planet) {
          for (const key of Object.keys(fresh)) {
            (planet as any)[key] = (fresh as any)[key]
          }
        }
      } catch {}
    }, 5000)
    return () => clearInterval(interval)
  })

  async function pollActivity() {
    if (!user) return
    try {
      const [fleets, research] = await Promise.all([
        api.getFleets(),
        api.getResearch(),
      ])
      let count = fleets.length
      if (planet?.const_building) count++
      if (research?.queued_tech) count++
      activeCount = count
    } catch {}
  }

  $effect(() => {
    if (!user) return
    pollActivity()
    const interval = setInterval(pollActivity, 10000)
    return () => clearInterval(interval)
  })

  // Refresh DM/credits without reassigning user (avoids triggering effects on user ref)
  async function refreshUserBalance() {
    try {
      const u = await api.me()
      if (user) {
        user.dark_matter = u.dark_matter
        user.credits = u.credits
      }
    } catch {}
  }

  $effect(() => {
    if (!user) return
    refreshUserBalance()
    const interval = setInterval(refreshUserBalance, 30000)
    return () => clearInterval(interval)
  })

  async function loadPlanet(userId: string) {
    try {
      const planets = await api.getPlanets()
      planetList = planets
      if (planets.length > 0) {
        planet = await api.getPlanet(planets[0].id)
      }
    } catch {}
  }

  async function switchPlanet(id: string) {
    if (id === planet?.id) return
    try {
      planet = await api.getPlanet(id)
    } catch {}
  }

  async function restoreSession() {
    const token = localStorage.getItem('token')
    if (!token) return
    try {
      user = await api.me()
      const p = window.location.pathname.replace('/', '') as View
      const valid: View[] = ['planet', 'galaxy', 'fleet', 'missions', 'dashboard', 'fortress', 'wormhole', 'eu-x', 'galactonites', 'alliance', 'power-rank', 'commanders', 'battle-reports', 'spy-reports', 'nebula', 'admin', 'profile', 'inbox', 'quests', 'daily-gift', 'gef']
      view = valid.includes(p) ? p : 'planet'
      if (window.location.pathname === '/' || !valid.includes(p)) history.replaceState(null, '', '/planet')
      await loadPlanet(user.id)
    } catch {
      localStorage.removeItem('token')
    }
  }

  setOnUnauthorized(() => {
    user = null
    planet = null
    view = 'login'
  })

  restoreSession()

  // Initialize DaisyUI theme
  $effect(() => {
    const saved = localStorage.getItem('theme') || 'dark'
    document.documentElement.setAttribute('data-theme', saved)
  })

  async function handleLogin() {
    error = ''
    if (!username || !password) {
      error = 'Please fill in all fields'
      return
    }
    loading = true
    try {
      const res = await api.login(username, password)
      localStorage.setItem('token', res.token)
      user = await api.me()
      await loadPlanet(user.id)
      view = 'planet'
      history.pushState(null, '', '/planet')
    } catch (e: any) {
      error = e.message
    } finally {
      loading = false
    }
  }

  async function handleRegister() {
    error = ''
    if (!username || !password || !confirmPassword) {
      error = 'Please fill in all fields'
      return
    }
    if (password !== confirmPassword) {
      error = 'Passwords do not match'
      return
    }
    if (password.length < 6) {
      error = 'Password must be at least 6 characters'
      return
    }
    if (username.length < 3) {
      error = 'Username must be at least 3 characters'
      return
    }
    loading = true
    try {
      const res = await api.register(username, password)
      localStorage.setItem('token', res.token)
      user = await api.me()
      await loadPlanet(user.id)
      view = 'planet'
      history.pushState(null, '', '/planet')
    } catch (e: any) {
      error = e.message
    } finally {
      loading = false
    }
  }

  function logout() {
    localStorage.removeItem('token')
    user = null
    planet = null
    showMore = false
    view = 'login'
    history.pushState(null, '', '/')
  }

  function switchPage(p: 'login' | 'register') {
    view = p
    error = ''
  }

  function navigate(v: View) {
    view = v
    showMore = false
    showLeftMobile = false
    if (v === 'galaxy') {
      const g = localStorage.getItem('goto_galaxy')
      const s = localStorage.getItem('goto_system')
      if (g) { currentGalaxy = parseInt(g); localStorage.removeItem('goto_galaxy') }
      if (s) { currentSystem = parseInt(s); localStorage.removeItem('goto_system') }
    }
    history.pushState(null, '', '/' + v)
  }

  const primaryTabs: { id: View; icon: string; label: string }[] = [
    { id: 'planet', icon: '🏠', label: 'Planet' },
    { id: 'galaxy', icon: '🌌', label: 'Galaxy' },
    { id: 'fleet', icon: '🚀', label: 'Fleet' },
  ]

  const moreItems: { id: View; icon: string; label: string }[] = [
    { id: 'dashboard', icon: '📊', label: 'Dashboard' },
    { id: 'missions', icon: '📋', label: 'Missions' },
    { id: 'fortress', icon: '⚔️', label: 'Fortress' },
    { id: 'wormhole', icon: '🌀', label: 'Wormhole' },
    { id: 'eu-x', icon: '📡', label: 'EU-X' },
    { id: 'galactonites', icon: '💎', label: 'Galactonites' },
    { id: 'alliance', icon: '🤝', label: 'Alliance' },
    { id: 'power-rank', icon: '📊', label: 'Power' },
    { id: 'commanders', icon: '👤', label: 'Commanders' },
    { id: 'battle-reports', icon: '📋', label: 'Reports' },
    { id: 'spy-reports', icon: '🕵️', label: 'Spy' },
    { id: 'nebula', icon: '🌫️', label: 'Nebula' },
    { id: 'quests', icon: '📋', label: 'Quests' },
    { id: 'daily-gift', icon: '🎁', label: 'Daily Gift' },
    { id: 'gef', icon: '⚔️', label: 'G.E.F.' },
    { id: 'inbox', icon: '📬', label: 'Inbox' },
    { id: 'profile', icon: '👤', label: 'Profile' },
    { id: 'admin', icon: '⚙️', label: 'Admin' },
  ]
</script>

{#if view === 'login' || view === 'register'}
  <div class="min-h-dvh flex items-center justify-center bg-[#0a0e1a] px-4">
    <div class="bg-[#111827] p-6 sm:p-8 rounded-lg border border-[#1e293b] w-full max-w-sm">
      <h1 class="text-xl sm:text-2xl font-bold text-white mb-6 text-center">Galaxy Empire</h1>

      {#if error}
        <div class="bg-red-900/50 text-red-400 p-3 rounded mb-4 text-sm">{error}</div>
      {/if}

      <form onsubmit={(e) => { e.preventDefault(); view === 'login' ? handleLogin() : handleRegister() }}>
        <div class="mb-4">
          <label class="block text-sm text-gray-400 mb-1">Username</label>
          <input
            bind:value={username}
            class="w-full bg-[#0a0e1a] border border-[#1e293b] rounded px-3 py-2.5 text-white text-base focus:border-blue-500 focus:outline-none"
            required
            disabled={loading}
          />
        </div>
        <div class="mb-4">
          <label class="block text-sm text-gray-400 mb-1">Password</label>
          <div class="relative">
            <input
              type={showPassword ? 'text' : 'password'}
              bind:value={password}
              class="w-full bg-[#0a0e1a] border border-[#1e293b] rounded px-3 py-2.5 pr-10 text-white text-base focus:border-blue-500 focus:outline-none"
              required
              disabled={loading}
            />
            <button type="button" onclick={() => showPassword = !showPassword} class="absolute right-2.5 top-1/2 -translate-y-1/2 text-gray-500 hover:text-gray-300" tabindex="-1">
              {#if showPassword}
                <svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"/><line x1="1" y1="1" x2="23" y2="23"/></svg>
              {:else}
                <svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"/><circle cx="12" cy="12" r="3"/></svg>
              {/if}
            </button>
          </div>
        </div>
        {#if view === 'register'}
          <div class="mb-6">
            <label class="block text-sm text-gray-400 mb-1">Confirm Password</label>
            <div class="relative">
              <input
                type={showConfirmPassword ? 'text' : 'password'}
                bind:value={confirmPassword}
                class="w-full bg-[#0a0e1a] border border-[#1e293b] rounded px-3 py-2.5 pr-10 text-white text-base focus:border-blue-500 focus:outline-none"
                required
                disabled={loading}
              />
              <button type="button" onclick={() => showConfirmPassword = !showConfirmPassword} class="absolute right-2.5 top-1/2 -translate-y-1/2 text-gray-500 hover:text-gray-300" tabindex="-1">
                {#if showConfirmPassword}
                  <svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"/><line x1="1" y1="1" x2="23" y2="23"/></svg>
                {:else}
                  <svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"/><circle cx="12" cy="12" r="3"/></svg>
                {/if}
              </button>
            </div>
          </div>
        {:else}
          <div class="mb-6"></div>
        {/if}
        <button
          type="submit"
          disabled={loading}
          class="w-full bg-blue-600 hover:bg-blue-700 disabled:bg-blue-800/50 text-white py-3 rounded-lg font-medium transition-colors text-base active:scale-[0.98]"
        >
          {loading ? 'Please wait...' : view === 'login' ? 'Sign In' : 'Create Account'}
        </button>
      </form>

      <p class="text-gray-500 text-sm text-center mt-4">
        {#if view === 'login'}
          Don't have an account?
          <button class="text-blue-400 hover:underline" onclick={() => switchPage('register')}>Register</button>
        {:else}
          Already have an account?
          <button class="text-blue-400 hover:underline" onclick={() => switchPage('login')}>Sign In</button>
        {/if}
      </p>
    </div>
  </div>
{:else}
  <div class="starfield"></div>
  <div class="scanlines"></div>
  <!-- Desktop layout: left sidebar + main + right sidebar -->
  <div class="flex min-h-dvh bg-[#0a0e1a] relative z-10">
    <!-- Left sidebar (always visible on desktop) -->
    <div class="hidden md:flex md:flex-col md:shrink-0">
      <LeftSidebar {view} onNavigate={navigate} onLogout={logout} />
    </div>

    <!-- Main content -->
    <main class="flex-1 min-w-0 flex flex-col pb-16 md:pb-0">
      <!-- Top bar (sticky) -->
      <div class="sticky top-0 z-30 bg-[#111827]/95 backdrop-blur-md border-b border-[#1e293b]">
        <!-- Mobile row -->
        <div class="flex md:hidden items-center justify-between px-3 py-2">
          <button onclick={() => showLeftMobile = true} class="text-gray-400 hover:text-white p-1">
            <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 6h16M4 12h16M4 18h16"/></svg>
          </button>
          <div class="flex items-center gap-2">
            <span class="text-sm font-bold text-white">Galaxy Empire</span>
            {#if planetList.length > 1}
              <select
                value={planet?.id || ''}
                onchange={(e) => switchPlanet(e.currentTarget.value)}
                class="bg-[#0a0e1a] border border-[#1e293b] text-white text-[10px] rounded px-1 py-0.5 max-w-[80px]"
              >
                {#each planetList as p, i}
                  <option value={p.id} selected={p.id === planet?.id}>P{i + 1}</option>
                {/each}
              </select>
            {/if}
          </div>
          <div class="flex items-center gap-2">
            {#if user}
              <span class="text-xs text-purple-400 font-medium">💎 {fmtNum(user.dark_matter)}</span>
              <span class="text-xs text-yellow-400 font-medium">💰 {fmtNum(user.credits)}</span>
            {/if}
            <button onclick={() => showRightMobile = true} class="text-gray-400 hover:text-white p-1 relative">
              <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 17h5l-1.405-1.405A2.032 2.032 0 0118 14.158V11a6.002 6.002 0 00-4-5.659V5a2 2 0 10-4 0v.341C7.67 6.165 6 8.388 6 11v3.159c0 .538-.214 1.055-.595 1.436L4 17h5m6 0v1a3 3 0 11-6 0v-1m6 0H9"/></svg>
              {#if activeCount > 0}
                <span class="absolute -top-0.5 -right-0.5 bg-red-500 text-white text-[10px] font-bold rounded-full min-w-[16px] h-4 flex items-center justify-center px-1 leading-none shadow-lg">
                  {activeCount > 99 ? '99+' : activeCount}
                </span>
              {/if}
            </button>
          </div>
        </div>
        <!-- Desktop row -->
        <div class="hidden md:flex items-center justify-between px-4 py-2">
          <div class="flex items-center gap-3">
            <span class="text-sm font-bold text-white">Galaxy Empire</span>
            {#if planetList.length > 1}
              <select
                value={planet?.id || ''}
                onchange={(e) => switchPlanet(e.currentTarget.value)}
                class="bg-[#0a0e1a] border border-[#1e293b] text-white text-xs rounded px-2 py-0.5"
              >
                {#each planetList as p, i}
                  <option value={p.id} selected={p.id === planet?.id}>Planet {i + 1}</option>
                {/each}
              </select>
            {/if}
          </div>
          <div class="flex items-center gap-4">
            {#if user}
              <span class="text-xs text-purple-400 font-medium">💎 {fmtNum(user.dark_matter)}</span>
              <span class="text-xs text-yellow-400 font-medium">💰 {fmtNum(user.credits)}</span>
            {/if}
          </div>
        </div>
      </div>

      <div class="flex-1 overflow-y-auto max-w-6xl mx-auto px-3 sm:px-4 py-3">
        {#if view === 'planet'}
          {#if planet}
            <PlanetDashboard {planet} />
          {:else}
            <div class="text-gray-500 text-center py-8">Loading your empire...</div>
          {/if}
        {:else if view === 'galaxy'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] p-3 sm:p-4">
            <GalaxyPanel bind:galaxy={currentGalaxy} bind:system={currentSystem} />
          </div>
        {:else if view === 'fleet'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <FleetPanel />
          </div>
        {:else if view === 'dashboard'}
          <DashboardPanel />
        {:else if view === 'missions'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <MissionsPanel />
          </div>
        {:else if view === 'fortress'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <FortressPanel planets={planetList} />
          </div>
        {:else if view === 'wormhole'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <WormholePanel />
          </div>
        {:else if view === 'eu-x'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <EUXPanel />
          </div>
        {:else if view === 'galactonites'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <GalactonitePanel planetId={planet?.id || ''} planet={planet ?? undefined} />
          </div>
        {:else if view === 'alliance'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <AlliancePanel />
          </div>
        {:else if view === 'power-rank'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <PowerRankPanel />
          </div>
        {:else if view === 'commanders'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <CommanderPanel />
          </div>
        {:else if view === 'battle-reports'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <BattleReportsPanel />
          </div>
        {:else if view === 'spy-reports'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <SpyReportsPanel />
          </div>
        {:else if view === 'quests'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <div class="flex gap-1 mb-3 sm:mb-3 px-3 sm:px-0 pt-3 sm:pt-0">
              <button
                onclick={() => questCategory = 'intro'}
                class="px-2.5 py-1 text-xs rounded transition-colors {questCategory === 'intro' ? 'bg-blue-600 text-white' : 'bg-[#0a0e1a] text-gray-400 border border-[#1e293b]'}"
              >Intro</button>
              <button
                onclick={() => questCategory = 'daily'}
                class="px-2.5 py-1 text-xs rounded transition-colors {questCategory === 'daily' ? 'bg-blue-600 text-white' : 'bg-[#0a0e1a] text-gray-400 border border-[#1e293b]'}"
              >Daily</button>
              <button
                onclick={() => questCategory = 'gef'}
                class="px-2.5 py-1 text-xs rounded transition-colors {questCategory === 'gef' ? 'bg-blue-600 text-white' : 'bg-[#0a0e1a] text-gray-400 border border-[#1e293b]'}"
              >G.E.F.</button>
            </div>
            <div class="px-3 sm:px-0">
              <QuestPanel category={questCategory} />
            </div>
          </div>
        {:else if view === 'daily-gift'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <DailyGiftPanel />
          </div>
        {:else if view === 'gef'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <GEFPanel planetId={planet?.id || ''} />
          </div>
        {:else if view === 'nebula'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <NebulaPanel planetId={planet?.id || ''} />
          </div>
        {:else if view === 'inbox'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <InboxPanel planetId={planet?.id || ''} />
          </div>
        {:else if view === 'profile'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <ProfilePage />
          </div>
        {:else if view === 'admin'}
          <div class="bg-[#111827] rounded-lg border border-[#1e293b] sm:p-4">
            <AdminPanel />
          </div>
        {/if}
      </div>
    </main>

    <!-- Right sidebar (always visible on desktop) -->
    <div class="hidden md:flex md:flex-col md:shrink-0">
      <RightSidebar currentPlanet={planet} onNavigate={navigate} />
    </div>
  </div>

  <!-- Mobile bottom nav -->
  <nav class="fixed bottom-0 left-0 right-0 md:hidden bg-[#111827]/95 backdrop-blur-md border-t border-[#1e293b] z-40">
    <div class="flex items-center justify-around px-2 py-1">
      {#each primaryTabs as tab}
        <button
          onclick={() => navigate(tab.id)}
          class="flex flex-col items-center gap-0.5 px-3 py-1.5 rounded-lg transition-colors min-w-0
            {view === tab.id ? 'text-blue-400' : 'text-gray-500 hover:text-gray-300'}"
        >
          <span class="text-lg">{tab.icon}</span>
          <span class="text-[10px] font-medium leading-tight">{tab.label}</span>
        </button>
      {/each}
      <button
        onclick={() => showMore = !showMore}
        class="flex flex-col items-center gap-0.5 px-3 py-1.5 rounded-lg transition-colors
          {showMore ? 'text-blue-400' : 'text-gray-500 hover:text-gray-300'}"
      >
        <span class="text-lg">⚡</span>
        <span class="text-[10px] font-medium">More</span>
      </button>
      <button onclick={logout} class="flex flex-col items-center gap-0.5 px-3 py-1.5 text-gray-500 hover:text-gray-300">
        <span class="text-lg">🚪</span>
        <span class="text-[10px] font-medium">Logout</span>
      </button>
    </div>
  </nav>

  <!-- Mobile more overlay -->
  {#if showMore}
    <div class="fixed inset-0 z-50 md:hidden" onclick={() => showMore = false}>
      <div class="absolute inset-0 bg-black/60 backdrop-blur-sm" />
      <div class="absolute bottom-16 left-2 right-2 bg-[#111827] border border-[#1e293b] rounded-xl p-3 shadow-2xl" onclick={(e) => e.stopPropagation()}>
        <div class="grid grid-cols-4 gap-2">
          {#each moreItems as item}
            <button
              onclick={() => navigate(item.id)}
              class="flex flex-col items-center gap-1 p-2.5 rounded-lg transition-colors
                {view === item.id ? 'bg-blue-600/20 text-blue-400' : 'text-gray-400 hover:text-white hover:bg-[#0a0e1a]'}"
            >
              <span class="text-xl">{item.icon}</span>
              <span class="text-[10px] font-medium text-center leading-tight">{item.label}</span>
            </button>
          {/each}
        </div>
      </div>
    </div>
  {/if}

  <!-- Mobile left nav drawer -->
  {#if showLeftMobile}
    <div class="fixed inset-0 z-50 md:hidden" onclick={() => showLeftMobile = false}>
      <div class="absolute inset-0 bg-black/60 backdrop-blur-sm" />
      <div class="absolute left-0 top-0 bottom-0 w-56 bg-[#111827] border-r border-[#1e293b] shadow-2xl animate-slide-right" onclick={(e) => e.stopPropagation()}>
        <LeftSidebar {view} onNavigate={navigate} onLogout={() => { showLeftMobile = false; logout() }} />
      </div>
    </div>
  {/if}

  <!-- Mobile right activity drawer -->
  {#if showRightMobile}
    <div class="fixed inset-0 z-50 md:hidden" onclick={() => showRightMobile = false}>
      <div class="absolute inset-0 bg-black/60 backdrop-blur-sm" />
      <div class="absolute right-0 top-0 bottom-0 w-60 bg-[#111827] border-l border-[#1e293b] shadow-2xl animate-slide-left" onclick={(e) => e.stopPropagation()}>
        <RightSidebar currentPlanet={planet} onNavigate={navigate} />
      </div>
    </div>
  {/if}

  <Tutorial />
{/if}

<style>
  @keyframes slide-right {
    from { transform: translateX(-100%); }
    to { transform: translateX(0); }
  }
  @keyframes slide-left {
    from { transform: translateX(100%); }
    to { transform: translateX(0); }
  }
  .animate-slide-right { animation: slide-right 0.2s ease-out; }
  .animate-slide-left { animation: slide-left 0.2s ease-out; }

</style>
