import { test, expect } from '@playwright/test'

test.describe('Authentication', () => {
  test('shows login page', async ({ page }) => {
    await page.goto('/')
    await expect(page.locator('text=Galaxy Empire')).toBeVisible()
    await expect(page.locator('text=Sign In')).toBeVisible()
  })

  test('switches to register form', async ({ page }) => {
    await page.goto('/')
    await page.click('text=Register')
    await expect(page.locator('text=Create Account')).toBeVisible()
    await expect(page.locator('text=Confirm Password')).toBeVisible()
  })

  test('password toggle shows/hides password', async ({ page }) => {
    await page.goto('/')
    const passwordInput = page.locator('input[type="password"]')
    const toggleButton = page.locator('button[tabindex="-1"]').first()
    await passwordInput.fill('secret123')
    await toggleButton.click()
    await expect(page.locator('input[type="text"]')).toBeVisible()
  })

  test('shows error on empty login', async ({ page }) => {
    await page.goto('/')
    await page.click('text=Sign In')
    await expect(page.locator('text=Please fill in all fields')).toBeVisible()
  })
})
