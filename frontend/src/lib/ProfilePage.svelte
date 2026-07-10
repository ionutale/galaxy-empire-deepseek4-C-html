<script lang="ts">
  import { api } from './api'
  import { fmtNum } from './format'

  let profile = $state<{ username: string; dark_matter: number; credits: number; created_at: string } | null>(null)
  let currentPassword = $state('')
  let newPassword = $state('')
  let confirmNewPassword = $state('')
  let passwordError = $state('')
  let passwordSuccess = $state('')
  let loading = $state(false)
  let isDark = $state(true)

  $effect(() => {
    api.getProfile().then(p => profile = p).catch(() => {})
  })

  $effect(() => {
    isDark = (document.documentElement.getAttribute('data-theme') || 'dark') === 'dark'
  })

  function toggleTheme() {
    const next = isDark ? 'light' : 'dark'
    document.documentElement.setAttribute('data-theme', next)
    localStorage.setItem('theme', next)
    isDark = !isDark
  }

  async function handleChangePassword() {
    passwordError = ''
    passwordSuccess = ''
    if (!currentPassword || !newPassword) {
      passwordError = 'Fill in all password fields'
      return
    }
    if (newPassword.length < 6) {
      passwordError = 'New password must be at least 6 characters'
      return
    }
    if (newPassword !== confirmNewPassword) {
      passwordError = 'Passwords do not match'
      return
    }
    loading = true
    try {
      await api.changePassword(currentPassword, newPassword)
      passwordSuccess = 'Password changed successfully'
      currentPassword = ''
      newPassword = ''
      confirmNewPassword = ''
    } catch (e: any) {
      passwordError = e.message
    } finally {
      loading = false
    }
  }
</script>

<div class="space-y-4">
  <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">Profile</h2>

  {#if profile}
    <div class="bg-[#111827] rounded-lg border border-[#1e293b] p-4 space-y-3">
      <div class="flex items-center justify-between">
        <span class="text-gray-400 text-sm">Username</span>
        <span class="text-white font-mono">{profile.username}</span>
      </div>
      <div class="flex items-center justify-between">
        <span class="text-gray-400 text-sm">Dark Matter</span>
        <span class="text-purple-400 font-mono">{fmtNum(profile.dark_matter)}</span>
      </div>
      <div class="flex items-center justify-between">
        <span class="text-gray-400 text-sm">Credits</span>
        <span class="text-yellow-400 font-mono">{fmtNum(profile.credits)}</span>
      </div>
      <div class="flex items-center justify-between">
        <span class="text-gray-400 text-sm">Registered</span>
        <span class="text-gray-300 font-mono text-xs">{new Date(profile.created_at).toLocaleDateString()}</span>
      </div>
    </div>
  {:else}
    <div class="bg-[#111827] rounded-lg border border-[#1e293b] p-4 text-gray-500 text-sm text-center py-8">Loading profile...</div>
  {/if}

  <div class="bg-[#111827] rounded-lg border border-[#1e293b] p-4 space-y-3">
    <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide">Change Password</h3>

    {#if passwordError}
      <div class="bg-red-900/50 text-red-400 p-2 rounded text-sm">{passwordError}</div>
    {/if}
    {#if passwordSuccess}
      <div class="bg-green-900/50 text-green-400 p-2 rounded text-sm">{passwordSuccess}</div>
    {/if}

    <div>
      <label class="text-xs text-gray-400 block mb-1">Current Password</label>
      <input type="password" bind:value={currentPassword} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1.5 text-xs" />
    </div>
    <div>
      <label class="text-xs text-gray-400 block mb-1">New Password</label>
      <input type="password" bind:value={newPassword} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1.5 text-xs" />
    </div>
    <div>
      <label class="text-xs text-gray-400 block mb-1">Confirm New Password</label>
      <input type="password" bind:value={confirmNewPassword} class="w-full bg-[#0a0e1a] border border-[#1e293b] text-white rounded px-2 py-1.5 text-xs" />
    </div>
    <button onclick={handleChangePassword} disabled={loading}
      class="w-full bg-blue-600 hover:bg-blue-700 disabled:bg-blue-800/50 text-white py-1.5 rounded text-xs font-medium transition-colors">
      {loading ? 'Changing...' : 'Change Password'}
    </button>
  </div>

  <div class="bg-[#111827] rounded-lg border border-[#1e293b] p-4 space-y-3">
    <h3 class="text-xs font-semibold text-gray-400 uppercase tracking-wide">Theme</h3>
    <button onclick={toggleTheme}
      class="w-full bg-[#0a0e1a] border border-[#1e293b] text-gray-300 hover:border-gray-500 py-2 rounded text-xs font-medium transition-colors">
      {isDark ? '☀ Switch to Light Mode' : '🌙 Switch to Dark Mode'}
    </button>
  </div>
</div>
