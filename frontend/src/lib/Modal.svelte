<script lang="ts">
  let { show = false, title = undefined, children } = $props()
</script>

{#if show}
  <div class="fixed inset-0 bg-black/60 flex items-center justify-center z-50" onclick={() => show = false}>
    <div class="bg-[#111827] border border-[#1e293b] rounded-lg p-6 w-96 max-w-full" onclick={(e) => e.stopPropagation()}>
      {#if title}
        <div class="flex justify-between items-start mb-4">
          <h3 class="text-lg font-bold text-white">{title}</h3>
          <button onclick={() => show = false} class="text-gray-500 hover:text-white text-lg">&times;</button>
        </div>
      {/if}
      {#if children}
        {@render children()}
      {/if}
    </div>
  </div>
{/if}
