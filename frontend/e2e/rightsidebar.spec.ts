import { test, expect } from '@playwright/test'

test.describe('RightSidebar Activity', () => {
  test('activity panel shows construction and production sections', async ({ page }) => {
    await page.goto('/planet')
    // Activity panel headings
    await expect(page.locator('text=Construction')).toBeVisible({ timeout: 10000 })
    await expect(page.locator('text=Production')).toBeVisible()
    await expect(page.locator('text=Research')).toBeVisible()
    await expect(page.locator('text=Active Fleets')).toBeVisible()
  })

  test('activity items are clickable', async ({ page }) => {
    await page.goto('/planet')
    // All activity sections should have clickable items
    const clickableItems = page.locator('.cursor-pointer')
    const count = await clickableItems.count()
    expect(count).toBeGreaterThan(0)
  })
})
