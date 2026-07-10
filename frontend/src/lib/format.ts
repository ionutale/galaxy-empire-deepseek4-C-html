export function fmtNum(n: number): string {
  const abs = Math.abs(n)
  if (abs >= 1_000_000_000_000) return (n / 1_000_000_000_000).toFixed(1).replace(/\.0$/, '') + 'T'
  if (abs >= 1_000_000_000) return (n / 1_000_000_000).toFixed(1).replace(/\.0$/, '') + 'B'
  if (abs >= 1_000_000) return (n / 1_000_000).toFixed(1).replace(/\.0$/, '') + 'M'
  if (abs >= 1000) return (n / 1000).toFixed(1).replace(/\.0$/, '') + 'K'
  return Math.floor(n).toLocaleString()
}

export function fmtTime(seconds: number): string {
  if (seconds < 60) return seconds + 's'
  if (seconds < 3600) return Math.floor(seconds / 60) + 'm ' + (seconds % 60) + 's'
  return Math.floor(seconds / 3600) + 'h ' + Math.floor((seconds % 3600) / 60) + 'm'
}

export function fmtRate(n: number): string {
  return (n >= 0 ? '+' : '') + n.toFixed(1) + '/h'
}

export function fmtCountdown(endTime: string): string {
  const end = new Date(endTime).getTime()
  const remaining = Math.max(0, Math.floor((end - Date.now()) / 1000))
  return fmtTime(remaining)
}

export function gotoCoords(galaxy: number, system: number) {
  localStorage.setItem('goto_galaxy', String(galaxy))
  localStorage.setItem('goto_system', String(system))
  window.location.href = '/galaxy'
}

export function fmtCoords(galaxy: number, system: number, pos: number): string {
  return `[${galaxy}:${system}:${pos}]`
}
