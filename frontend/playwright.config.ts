import { defineConfig } from '@playwright/test'

export default defineConfig({
  testDir: './e2e',
  timeout: 30000,
  expect: { timeout: 10000 },
  use: {
    baseURL: 'http://localhost:8080',
    headless: true,
  },
  webServer: {
    command: 'cd .. && go run cmd/api/main.go &',
    port: 8080,
    timeout: 30000,
    reuseExistingServer: true,
  },
})
