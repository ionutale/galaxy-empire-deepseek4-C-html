import { test, expect } from '@playwright/test'

test.describe('Admin Panel', () => {
  test('admin panel shows 6 tabs', async ({ page }) => {
    await page.goto('/admin')
    // Wait for content
    await expect(page.locator('text=Admin Panel')).toBeVisible()
    const tabs = ['Resources', 'Buildings', 'User', 'Research', 'Fleet', 'Defense']
    for (const tab of tabs) {
      await expect(page.locator(`text=${tab}`)).toBeVisible()
    }
  })

  test('admin panel has coordinates input fields', async ({ page }) => {
    await page.goto('/admin')
    await expect(page.locator('text=Galaxy')).toBeVisible()
    await expect(page.locator('text=System')).toBeVisible()
    await expect(page.locator('text=Position')).toBeVisible()
  })

  test('admin panel has username input', async ({ page }) => {
    await page.goto('/admin')
    // Click User tab
    await page.click('button:has-text("User")')
    await expect(page.locator('text=Username')).toBeVisible()
  })
})
