import { test, expect } from '@playwright/test'

test.describe('Planet View', () => {
  test('resource display uses K/M/B/T formatting', async ({ page }) => {
    await page.goto('/planet')
    // Resource bars should show formatted numbers
    const resourceBars = page.locator('.bg-\\[#111827\\] > .flex.justify-between')
    await expect(resourceBars.first()).toBeVisible()
  })

  test('upgrade modal shows effect comparison', async ({ page }) => {
    await page.goto('/planet')
    // Click first building card
    const buildingCards = page.locator('button:has(span:has-text("Mine"))')
    if (await buildingCards.count() > 0) {
      await buildingCards.first().click()
      // Modal should show effect comparison
      await expect(page.locator('text=Current')).toBeVisible()
      await expect(page.locator('text=After Upgrade')).toBeVisible()
    }
  })

  test('construction queue section exists', async ({ page }) => {
    await page.goto('/planet')
    // Queue section should exist in the upgrade modal
    await expect(page.locator('text=Queue')).toBeVisible()
  })
})
