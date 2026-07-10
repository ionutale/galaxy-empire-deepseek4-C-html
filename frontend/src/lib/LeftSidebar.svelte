<script lang="ts">
  type View = 'planet' | 'galaxy' | 'fleet' | 'dashboard' | 'missions' | 'fortress' | 'wormhole' | 'eu-x' | 'galactonites' | 'alliance' | 'power-rank' | 'commanders' | 'battle-reports' | 'spy-reports' | 'nebula' | 'admin' | 'inbox' | 'profile'

  let { view, onNavigate, onLogout }: { view: View; onNavigate: (v: View) => void; onLogout: () => void } = $props()

  const navItems: { id: View; icon: string; label: string }[] = [
    { id: 'planet', icon: '🏠', label: 'Planet' },
    { id: 'galaxy', icon: '🌌', label: 'Galaxy' },
    { id: 'fleet', icon: '🚀', label: 'Fleet' },
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
    { id: 'inbox', icon: '📬', label: 'Inbox' },
    { id: 'admin', icon: '⚙️', label: 'Admin' },
    { id: 'profile', icon: '👤', label: 'Profile' },
  ]
</script>

<aside class="w-56 bg-[#111827] border-r border-[#1e293b] flex flex-col h-full">
  <div class="px-4 py-4 border-b border-[#1e293b]">
    <h1 class="text-base font-bold text-white">Galaxy Empire</h1>
  </div>
  <nav class="flex-1 overflow-y-auto px-2 py-3 space-y-0.5">
    {#each navItems as item}
      <button
        onclick={() => onNavigate(item.id)}
        class="w-full flex items-center gap-2.5 px-3 py-2 rounded-lg text-sm transition-colors
          {view === item.id
            ? 'bg-blue-600/20 text-blue-400 font-medium'
            : 'text-gray-400 hover:text-white hover:bg-[#0a0e1a]'}"
      >
        <span class="text-base w-5 text-center">{item.icon}</span>
        <span>{item.label}</span>
      </button>
    {/each}
  </nav>
  <div class="px-2 py-3 border-t border-[#1e293b]">
    <button
      onclick={onLogout}
      class="w-full flex items-center gap-2.5 px-3 py-2 rounded-lg text-sm text-gray-500 hover:text-white hover:bg-[#0a0e1a] transition-colors"
    >
      <span class="text-base w-5 text-center">🚪</span>
      <span>Logout</span>
    </button>
  </div>
</aside>
