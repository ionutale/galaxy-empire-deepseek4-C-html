import { test, expect } from '@playwright/test'

test.describe('Navigation', () => {
  test('URL path routing works', async ({ page }) => {
    await page.goto('/planet')
    await expect(page).toHaveURL('/planet')

    await page.goto('/galaxy')
    await expect(page).toHaveURL('/galaxy')

    await page.goto('/fleet')
    await expect(page).toHaveURL('/fleet')
  })

  test('unknown path redirects to login', async ({ page }) => {
    await page.goto('/nonexistent')
    await expect(page.locator('text=Galaxy Empire')).toBeVisible()
  })
})
