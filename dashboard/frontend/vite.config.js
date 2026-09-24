import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
export default defineConfig({
  plugins: [vue()],
  server: {
    allowedHosts: ['elder.wio.me'],
    proxy: { '/api': 'http://api:8000', '/ws': 'ws://api:8000' }
  }
})
