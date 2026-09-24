import { createApp, ref, onMounted } from 'vue'
import './style.css'

const App = { setup() {
  const status = ref({ online: false, latest: null }); const messages = ref([])
  const load = async () => { status.value = await fetch('/api/status').then(r => r.json()); messages.value = await fetch('/api/messages').then(r => r.json()) }
  onMounted(() => { load(); const source = new EventSource('/api/events'); source.onmessage = e => { messages.value.unshift(JSON.parse(e.data)); messages.value = messages.value.slice(0, 100); load() } })
  return { status, messages }
}, template: `
  <main><header><h1>Elderly <span>Monitor</span></h1><div class="updated">实时监护 · elder.wio.me</div></header>
  <section class="summary"><article><small>当前状态</small><strong>{{ status.latest?.person_present ? '有人在家' : '暂无人员' }}</strong><p>{{ status.online ? '设备在线' : '等待上报' }}</p></article><article><small>设备连接</small><strong :class="status.online ? 'good' : 'bad'">{{ status.online ? '在线' : '离线' }}</strong><p>最近上报：{{ status.last_report || '暂无' }}</p></article><article><small>长时间无人阈值</small><strong>{{ Math.round((status.absence_timeout_sec || 300) / 60) }} 分钟</strong><p>由后台任务持续检查</p></article></section>
  <section class="timeline"><div class="section-head"><h2>24 小时活动时间线</h2><span>MQTT 实时事件</span></div><div class="track"><i v-for="n in 24" :key="n" :class="messages[n-1] ? 'active' : ''"></i></div><div class="hours"><span>00:00</span><span>06:00</span><span>12:00</span><span>18:00</span><span>24:00</span></div></section>
  <section class="messages"><div class="section-head"><h2>最近消息（{{ messages.length }}）</h2><span>自动更新</span></div><div class="row head"><span>时间</span><span>事件</span><span>状态</span></div><div class="row" v-for="(m,i) in messages" :key="i"><span>{{ new Date(m.received_at || Date.now()).toLocaleTimeString() }}</span><span>{{ m.event || 'regular_report' }}</span><span class="dot" :class="m.person_present ? 'presence' : ''">{{ m.person_present ? '有人' : '无人' }}</span></div><p v-if="!messages.length" class="empty">等待树莓派上报消息…</p></section>
  </main>` }
createApp(App).mount('#app')
