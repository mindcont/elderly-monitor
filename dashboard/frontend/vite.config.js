import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
export default defineConfig({
  base: '/family-7Kp9mQ2vLx4R/',
  plugins: [vue()],
  resolve: {
    alias: { vue: 'vue/dist/vue.esm-bundler.js' }
  },
  server: {
    allowedHosts: ['elder.wio.me'],
    proxy: { '/api': 'http://api:8000', '/ws': 'ws://api:8000' }
  }
})
