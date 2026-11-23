import { createApp, ref, computed, onMounted } from 'vue'
import './style.css'

const App = { setup() {
  const status = ref({ online: false, latest: null }); const messages = ref([])
  const range = ref('24h')
  const chart = computed(() => {
    const now = Date.now()
    const duration = range.value === '24h' ? 24 * 3600000 : 7 * 24 * 3600000
    const start = now - duration
    const events = messages.value.map(m => ({ ...m, time: new Date(m.received_at).getTime() }))
      .filter(m => Number.isFinite(m.time) && m.time >= start && m.time <= now)
      .sort((a, b) => a.time - b.time)
    const points = [{ time: start, present: events.length ? !events[0].person_present : !!status.value.latest?.person_present }]
    events.forEach(m => points.push({ time: m.time, present: !!m.person_present, event: m.event }))
    points.push({ time: now, present: !!status.value.latest?.person_present })
    const x = time => ((time - start) / duration * 100).toFixed(2)
    const path = points.map((p, i) => `${i ? 'L' : 'M'} ${x(p.time)} ${p.present ? 22 : 78}`).join(' ')
    const area = `${path} L 100 100 L 0 100 Z`
    return { points: events.map(m => ({ ...m, x: x(m.time), label: m.person_present ? '有人' : '无人' })), path, area,
      ticks: Array.from({ length: range.value === '24h' ? 7 : 8 }, (_, i, a) => ({ x: (i / (a.length - 1) * 100).toFixed(2), label: new Date(start + duration * i / (a.length - 1)).toLocaleString([], range.value === '24h' ? { hour: '2-digit', minute: '2-digit' } : { month: 'numeric', day: 'numeric' }) })) }
  })
  const load = async () => { status.value = await fetch('/api/status').then(r => r.json()); messages.value = await fetch('/api/messages').then(r => r.json()) }
  onMounted(() => { load(); const source = new EventSource('/api/events'); source.onmessage = e => { const item = JSON.parse(e.data); if (item.event !== 'regular_report') { messages.value.unshift(item); messages.value = messages.value.slice(0, 100) }; load() } })
  return { status, messages, range, chart }
}, template: `
  <main><header><h1>Elderly <span>Monitor</span></h1><div class="updated">实时监护 · elder.wio.me</div></header>
  <section class="summary"><article><small>当前状态</small><strong>{{ status.latest?.person_present ? '有人在家' : '暂无人员' }}</strong><p>{{ status.online ? '设备在线' : '等待上报' }}</p></article><article><small>设备连接</small><strong :class="status.online ? 'good' : 'bad'">{{ status.online ? '在线' : '离线' }}</strong><p>最近上报：{{ status.last_report || '暂无' }}</p></article><article><small>长时间无人阈值</small><strong>{{ Math.round((status.absence_timeout_sec || 300) / 60) }} 分钟</strong><p>由后台任务持续检查</p></article></section>
  <section class="timeline"><div class="section-head"><div><h2>居家状态趋势</h2><p class="chart-subtitle">根据事件记录还原有人 / 无人状态变化</p></div><div class="range-switch"><button :class="range === '24h' ? 'selected' : ''" @click="range = '24h'">24 小时</button><button :class="range === '7d' ? 'selected' : ''" @click="range = '7d'">7 天</button></div></div>
    <div class="chart"><div class="y-labels"><span>有人</span><span>无人</span></div><div class="plot"><div class="grid-line top"></div><div class="grid-line middle"></div><div class="grid-line bottom"></div><svg viewBox="0 0 100 100" preserveAspectRatio="none" aria-label="居家状态趋势图"><path class="area" :d="chart.area"/><path class="line" :d="chart.path"/><circle v-for="(p,i) in chart.points" :key="i" :cx="p.x" :cy="p.person_present ? 22 : 78" r="1.25" :class="p.person_present ? 'person-point' : 'away-point'"><title>{{ new Date(p.time).toLocaleString() }} · {{ p.label }} · {{ p.event || '状态更新' }}</title></circle></svg><div class="x-labels"><span v-for="(tick,i) in chart.ticks" :key="i" :style="{left: tick.x + '%'}">{{ tick.label }}</span></div></div></div>
    <div class="chart-legend"><span><i class="legend-person"></i>有人</span><span><i class="legend-away"></i>无人</span><span class="event-count">{{ chart.points.length }} 条状态事件</span></div>
  </section>
  </main>` }
createApp(App).mount('#app')
