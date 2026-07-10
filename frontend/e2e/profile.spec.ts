import { test, expect } from '@playwright/test'

test.describe('Profile Page', () => {
  test('profile page shows user info', async ({ page }) => {
    await page.goto('/profile')
    await expect(page.locator('text=Profile')).toBeVisible()
    await expect(page.locator('text=Account')).toBeVisible()
    await expect(page.locator('text=Change Password')).toBeVisible()
    await expect(page.locator('text=Theme')).toBeVisible()
  })

  test('change password form has required fields', async ({ page }) => {
    await page.goto('/profile')
    await expect(page.locator('text=Current Password')).toBeVisible()
    await expect(page.locator('text=New Password')).toBeVisible()
    await expect(page.locator('text=Confirm Password')).toBeVisible()
  })
})
