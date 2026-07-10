<script lang="ts">
  import { api, type AllianceDetail, type AllianceMember } from './api'

  let data = $state<AllianceDetail | null>(null)
  let loading = $state(true)
  let error = $state('')
  let createName = $state('')
  let creating = $state(false)
  let activeTab = $state<'overview' | 'members' | 'mail'>('overview')

  // Bulletin
  let bulletinText = $state('')
  let savingBulletin = $state(false)

  // Mail
  let mailList = $state<any[]>([])
  let mailSubject = $state('')
  let mailBody = $state('')
  let sendingMail = $state(false)

  // Join requests
  let joinRequests = $state<any[]>([])

  // Invite
  let inviteUsername = $state('')
  let inviting = $state(false)

  // Vote
  let activeVote = $state<any>(null)
  let voting = $state(false)

  const allianceTabs: ('overview' | 'members' | 'mail')[] = ['overview', 'members', 'mail']

  async function refreshData() {
    try {
      data = await api.getMyAlliance()
    } catch (e: any) {
      error = e.message
    }
  }

  $effect(() => {
    loading = true
    error = ''
    api.getMyAlliance()
      .then(d => data = d)
      .catch((e: any) => error = e.message)
      .finally(() => loading = false)
  })

  async function loadMail() {
    if (!data) return
    try {
      mailList = await api.getMail(data.alliance.id)
    } catch {}
  }

  async function loadJoinRequests() {
    if (!data) return
    try {
      joinRequests = await api.getJoinRequests(data.alliance.id)
    } catch {}
  }

  async function loadVote() {
    if (!data) return
    try {
      activeVote = await api.getActiveVote(data.alliance.id)
    } catch {}
  }

  $effect(() => {
    if (data && activeTab === 'mail') {
      loadMail()
      loadJoinRequests()
      loadVote()
    }
  })

  async function handleCreate() {
    if (!createName.trim()) return
    creating = true
    error = ''
    try {
      data = await api.createAlliance(createName.trim())
      createName = ''
    } catch (e: any) {
      error = e.message
    } finally {
      creating = false
    }
  }

  async function handleLeave() {
    if (!data) return
    if (!confirm('Leave your alliance?')) return
    error = ''
    try {
      await api.leaveAlliance(data.alliance.id)
      data = null
    } catch (e: any) {
      error = e.message
    }
  }

  async function handleDisband() {
    if (!data) return
    if (!confirm('Disband the alliance? This cannot be undone.')) return
    error = ''
    try {
      await api.disbandAlliance(data.alliance.id)
      data = null
    } catch (e: any) {
      error = e.message
    }
  }

  async function handlePromote(member: AllianceMember) {
    if (!data) return
    error = ''
    try {
      await api.promoteMember(data.alliance.id, member.user_id)
      await refreshData()
    } catch (e: any) {
      error = e.message
    }
  }

  async function handleDemote(member: AllianceMember) {
    if (!data) return
    error = ''
    try {
      await api.demoteMember(data.alliance.id, member.user_id)
      await refreshData()
    } catch (e: any) {
      error = e.message
    }
  }

  async function handleSaveBulletin() {
    if (!data) return
    savingBulletin = true
    error = ''
    try {
      await api.setBulletin(data.alliance.id, bulletinText)
      await refreshData()
    } catch (e: any) {
      error = e.message
    } finally {
      savingBulletin = false
    }
  }

  async function handleSendMail() {
    if (!data || !mailSubject.trim() || !mailBody.trim()) return
    sendingMail = true
    error = ''
    try {
      await api.sendMail(data.alliance.id, mailSubject.trim(), mailBody.trim())
      mailSubject = ''
      mailBody = ''
      await loadMail()
    } catch (e: any) {
      error = e.message
    } finally {
      sendingMail = false
    }
  }

  async function handleInvite() {
    if (!data || !inviteUsername.trim()) return
    inviting = true
    error = ''
    try {
      await api.invitePlayer(data.alliance.id, inviteUsername.trim())
      inviteUsername = ''
      await refreshData()
      await loadJoinRequests()
    } catch (e: any) {
      error = e.message
    } finally {
      inviting = false
    }
  }

  async function handleApprove(userId: string) {
    if (!data) return
    error = ''
    try {
      await api.approveJoin(data.alliance.id, userId)
      await refreshData()
      await loadJoinRequests()
    } catch (e: any) {
      error = e.message
    }
  }

  async function handleDeny(userId: string) {
    if (!data) return
    error = ''
    try {
      await api.denyJoin(data.alliance.id, userId)
      await loadJoinRequests()
    } catch (e: any) {
      error = e.message
    }
  }

  async function handleInitiateVote() {
    if (!data) return
    error = ''
    try {
      await api.initiateVote(data.alliance.id)
      await loadVote()
    } catch (e: any) {
      error = e.message
    }
  }

  async function handleCastVote(vote: string) {
    if (!data) return
    voting = true
    error = ''
    try {
      await api.castVote(data.alliance.id, vote)
      await loadVote()
    } catch (e: any) {
      error = e.message
    } finally {
      voting = false
    }
  }

  function roleBadge(role: string): string {
    switch (role) {
      case 'president': return 'bg-yellow-700 text-yellow-200'
      case 'vice_president': return 'bg-gray-500 text-white'
      default: return 'bg-blue-800 text-blue-200'
    }
  }
</script>

<div class="space-y-4">
  <h2 class="text-sm font-semibold text-gray-400 uppercase tracking-wide">Alliance</h2>

  {#if error}
    <div class="bg-red-900/50 text-red-400 p-3 rounded text-sm">{error}</div>
  {/if}

  {#if loading}
    <div class="text-gray-500 text-sm">Loading...</div>
  {:else if !data}
    <div class="bg-[#0a0e1a] border border-[#1e293b] rounded p-4 max-w-md">
      <h3 class="text-sm font-semibold text-gray-300 mb-3">Create Alliance</h3>
      <p class="text-xs text-gray-500 mb-3">Requires Alliance Depot L1 on any planet.</p>
      <div class="flex gap-2">
        <input
          bind:value={createName}
          placeholder="Alliance name"
          maxlength={100}
          class="flex-1 bg-[#0a0e1a] border border-[#1e293b] rounded px-3 py-1.5 text-white text-sm focus:border-blue-500 focus:outline-none"
        />
        <button
          onclick={handleCreate}
          disabled={creating || !createName.trim()}
          class="bg-blue-600 hover:bg-blue-700 disabled:bg-gray-700 text-white text-xs px-3 py-1.5 rounded"
        >{creating ? 'Creating...' : 'Create'}</button>
      </div>
    </div>
  {:else}
    <div class="flex gap-1 mb-4 border-b border-[#1e293b] pb-2">
      {#each allianceTabs as tab}
        <button
          onclick={() => activeTab = tab}
          class="text-xs px-3 py-1 rounded {activeTab === tab ? 'bg-blue-600 text-white' : 'text-gray-400 hover:text-white'}"
        >{tab.charAt(0).toUpperCase() + tab.slice(1)}</button>
      {/each}
    </div>

    {#if activeTab === 'overview'}
      <div class="bg-[#0a0e1a] border border-[#1e293b] rounded p-4 mb-4">
        <div class="flex justify-between items-start">
          <div>
            <h3 class="text-lg font-bold text-white">{data.alliance.name}</h3>
            <p class="text-xs text-gray-500 mt-1">
              Founded {new Date(data.alliance.created_at).toLocaleDateString()}
              &middot; {data.members.length}/50 members
            </p>
          </div>
          <div class="flex gap-2">
            {#if data.my_role !== 'president'}
              <button onclick={handleLeave} class="text-xs bg-red-800 hover:bg-red-700 text-white px-2 py-1 rounded">Leave</button>
            {:else}
              <button onclick={handleDisband} class="text-xs bg-red-900 hover:bg-red-800 text-red-300 px-2 py-1 rounded">Disband</button>
            {/if}
          </div>
        </div>
      </div>

      <div class="bg-[#0a0e1a] border border-[#1e293b] rounded p-4 mb-4">
        <h3 class="text-sm font-semibold text-gray-300 mb-2">Bulletin</h3>
        {#if data.my_role === 'president' || data.my_role === 'vice_president'}
          <textarea
            bind:value={bulletinText}
            class="w-full bg-[#0a0e1a] border border-[#1e293b] rounded px-2 py-1 text-white text-sm h-20 mb-2"
            placeholder="Write alliance bulletin..."
          ></textarea>
          <button
            onclick={handleSaveBulletin}
            disabled={savingBulletin}
            class="text-xs bg-blue-600 hover:bg-blue-700 disabled:bg-gray-700 text-white px-3 py-1 rounded"
          >{savingBulletin ? 'Saving...' : 'Save'}</button>
        {:else}
          <div class="text-sm text-gray-400 italic">
            {data.alliance.bulletin || '(No bulletin set)'}
          </div>
        {/if}
      </div>

      {#if activeVote}
        <div class="bg-[#0a0e1a] border border-yellow-700 rounded p-4 mb-4">
          <h3 class="text-sm font-semibold text-yellow-400 mb-2">Active Transfer Vote</h3>
          <p class="text-xs text-gray-400 mb-2">
            Initiated by a Vice President. Expires {new Date(activeVote.expires_at).toLocaleString()}
          </p>
          <p class="text-xs text-gray-500 mb-2">
            {activeVote.yea_count} yea / {activeVote.nay_count} nay
          </p>
          {#if data.my_role !== 'president'}
            <div class="flex gap-2">
              <button
                onclick={() => handleCastVote('yea')}
                disabled={voting}
                class="text-xs bg-green-700 hover:bg-green-600 disabled:bg-gray-700 text-white px-3 py-1 rounded"
              >Yea</button>
              <button
                onclick={() => handleCastVote('nay')}
                disabled={voting}
                class="text-xs bg-red-700 hover:bg-red-600 disabled:bg-gray-700 text-white px-3 py-1 rounded"
              >Nay</button>
            </div>
          {/if}
        </div>
      {:else if data.my_role === 'vice_president'}
        <button
          onclick={handleInitiateVote}
          class="text-xs bg-yellow-700 hover:bg-yellow-600 text-white px-3 py-1 rounded"
        >Initiate President Transfer Vote</button>
      {/if}
    {:else if activeTab === 'members'}
      <div class="bg-[#0a0e1a] border border-[#1e293b] rounded p-4 mb-4">
        <h3 class="text-sm font-semibold text-gray-300 mb-2">Members ({data.members.length})</h3>
        <div class="space-y-1">
          {#each data.members as member}
            <div class="flex items-center justify-between py-1">
              <div class="flex items-center gap-2">
                <span class="text-sm text-white">{member.username}</span>
                <span class="text-xs px-1.5 py-0.5 rounded {roleBadge(member.role)}">{member.role.replace('_', ' ')}</span>
              </div>
              {#if data.my_role === 'president' && member.role !== 'president'}
                <div class="flex gap-1">
                  {#if member.role === 'member'}
                    <button onclick={() => handlePromote(member)} class="text-xs bg-gray-700 hover:bg-gray-600 text-gray-300 px-1.5 py-0.5 rounded" title="Promote to Vice President">VP</button>
                  {:else}
                    <button onclick={() => handleDemote(member)} class="text-xs bg-gray-700 hover:bg-gray-600 text-gray-300 px-1.5 py-0.5 rounded" title="Demote to Member">Demote</button>
                  {/if}
                </div>
              {/if}
            </div>
          {/each}
        </div>
      </div>

      {#if data.my_role === 'president' || data.my_role === 'vice_president'}
        <div class="bg-[#0a0e1a] border border-[#1e293b] rounded p-4 mb-4">
          <h3 class="text-sm font-semibold text-gray-300 mb-2">Invite Player</h3>
          <div class="flex gap-2">
            <input
              bind:value={inviteUsername}
              placeholder="Username"
              class="flex-1 bg-[#0a0e1a] border border-[#1e293b] rounded px-2 py-1 text-white text-sm"
            />
            <button
              onclick={handleInvite}
              disabled={inviting || !inviteUsername.trim()}
              class="text-xs bg-green-700 hover:bg-green-600 disabled:bg-gray-700 text-white px-3 py-1 rounded"
            >{inviting ? 'Inviting...' : 'Invite'}</button>
          </div>
        </div>
      {/if}

      {#if (data.my_role === 'president' || data.my_role === 'vice_president') && joinRequests.length > 0}
        <div class="bg-[#0a0e1a] border border-[#1e293b] rounded p-4 mb-4">
          <h3 class="text-sm font-semibold text-gray-300 mb-2">Join Requests ({joinRequests.length})</h3>
          <div class="space-y-1">
            {#each joinRequests.filter(r => r.status === 'pending') as req}
              <div class="flex items-center justify-between py-1">
                <span class="text-sm text-white">{req.username || req.user_id.slice(0, 8)}</span>
                <div class="flex gap-1">
                  <button onclick={() => handleApprove(req.user_id)} class="text-xs bg-green-700 hover:bg-green-600 text-white px-2 py-0.5 rounded">Approve</button>
                  <button onclick={() => handleDeny(req.user_id)} class="text-xs bg-red-700 hover:bg-red-600 text-white px-2 py-0.5 rounded">Deny</button>
                </div>
              </div>
            {/each}
          </div>
        </div>
      {/if}
    {:else if activeTab === 'mail'}
      {#if data.my_role === 'president' || data.my_role === 'vice_president'}
        <div class="bg-[#0a0e1a] border border-[#1e293b] rounded p-4 mb-4">
          <h3 class="text-sm font-semibold text-gray-300 mb-2">Send Alliance Mail</h3>
          <input
            bind:value={mailSubject}
            placeholder="Subject"
            class="w-full bg-[#0a0e1a] border border-[#1e293b] rounded px-2 py-1 text-white text-sm mb-2"
          />
          <textarea
            bind:value={mailBody}
            placeholder="Message body"
            class="w-full bg-[#0a0e1a] border border-[#1e293b] rounded px-2 py-1 text-white text-sm h-20 mb-2"
          ></textarea>
          <button
            onclick={handleSendMail}
            disabled={sendingMail || !mailSubject.trim() || !mailBody.trim()}
            class="text-xs bg-blue-600 hover:bg-blue-700 disabled:bg-gray-700 text-white px-3 py-1 rounded"
          >{sendingMail ? 'Sending...' : 'Send to All Members'}</button>
        </div>
      {/if}

      <div class="bg-[#0a0e1a] border border-[#1e293b] rounded p-4">
        <h3 class="text-sm font-semibold text-gray-300 mb-2">Alliance Mail</h3>
        {#if mailList.length === 0}
          <div class="text-gray-600 text-sm">No mail yet.</div>
        {:else}
          <div class="space-y-2">
            {#each mailList as msg}
              <div class="bg-[#111827] rounded p-3">
                <div class="flex justify-between text-xs text-gray-500">
                  <span>From: {msg.sender_name || msg.sender_id.slice(0, 8)}</span>
                  <span>{new Date(msg.sent_at).toLocaleString()}</span>
                </div>
                <div class="text-sm text-white font-medium mt-1">{msg.subject}</div>
                <div class="text-sm text-gray-400 mt-1 whitespace-pre-wrap">{msg.body}</div>
              </div>
            {/each}
          </div>
        {/if}
      </div>
    {/if}
  {/if}
</div>
