import { defineConfig } from 'vite'
import { svelte } from '@sveltejs/vite-plugin-svelte'
import tailwindcss from '@tailwindcss/vite'
import { VitePWA } from 'vite-plugin-pwa'

export default defineConfig({
  plugins: [
    tailwindcss(),
    svelte(),
    VitePWA({
      registerType: 'autoUpdate',
      includeAssets: ['all_icons/**/*'],
      manifest: {
        name: 'Galaxy Empire',
        short_name: 'Galaxy Empire',
        description: 'Space strategy MMO — build your empire across the galaxy',
        theme_color: '#0a0e1a',
        background_color: '#0a0e1a',
        display: 'standalone',
        orientation: 'portrait',
        icons: [
          { src: '/all_icons/web/icon_web_PWA192_192x192.png', sizes: '192x192', type: 'image/png' },
          { src: '/all_icons/web/icon_web_PWA512_512x512.png', sizes: '512x512', type: 'image/png' },
          { src: '/all_icons/web/icon_web_PWA512_512x512.png', sizes: '512x512', type: 'image/png', purpose: 'maskable' },
          { src: '/all_icons/web/icon_web_Small_32x32.png', sizes: '32x32', type: 'image/png' },
          { src: '/all_icons/web/icon_web_Large_64x64.png', sizes: '64x64', type: 'image/png' },
          { src: '/all_icons/android/icon_android_LauncherHDPI_96x96.png', sizes: '96x96', type: 'image/png' },
          { src: '/all_icons/android/icon_android_LauncherXHDPI_144x144.png', sizes: '144x144', type: 'image/png' },
          { src: '/all_icons/android/icon_android_LauncherXXHDPI_192x192.png', sizes: '192x192', type: 'image/png' },
          { src: '/all_icons/android/icon_android_PlayStore_512x512.png', sizes: '512x512', type: 'image/png' },
          { src: '/all_icons/ios/icon_ios_iPhone_180x180.png', sizes: '180x180', type: 'image/png', purpose: 'any' },
          { src: '/all_icons/ios/icon_ios_iPad_120x120.png', sizes: '120x120', type: 'image/png', purpose: 'any' },
          { src: '/all_icons/ios/icon_ios_AppStore_1024x1024.png', sizes: '1024x1024', type: 'image/png' },
        ],
      },
      workbox: {
        globPatterns: ['**/*.{js,css,html,ico,png,svg,woff2}'],
      },
    }),
  ],
  server: {
    host: '0.0.0.0',
    port: 5173,
    proxy: {
      '/api': 'http://api:8080',
      '/ws': { target: 'ws://api:8080', ws: true },
    },
  },
})
