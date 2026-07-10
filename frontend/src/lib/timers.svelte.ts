import { fmtTime } from './format'

let now = $state(Date.now())

function tick() {
  now = Date.now()
}

let running = false

export function startClock() {
  if (running) return
  running = true
  setInterval(tick, 1000)
}

export function secondsRemaining(endTime: string | null | undefined): number {
  if (!endTime) return 0
  return Math.max(0, Math.floor((new Date(endTime).getTime() - now) / 1000))
}

export function remainingFormatted(endTime: string | null | undefined): string {
  return fmtTime(secondsRemaining(endTime))
}

export function isExpired(endTime: string | null | undefined): boolean {
  return !endTime || new Date(endTime).getTime() <= now
}

export function countdown(endTime: string | null | undefined, fallback = ''): string {
  if (isExpired(endTime)) return fallback
  return remainingFormatted(endTime)
}

let modalCount = 0

export function lockBody(lock: boolean) {
  modalCount += lock ? 1 : -1
  if (modalCount < 0) modalCount = 0
  document.body.style.overflow = modalCount > 0 ? 'hidden' : ''
}
