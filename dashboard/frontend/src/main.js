import { createApp, ref, computed, onMounted } from 'vue'
import './style.css'
import './dashboard-layout.css'

const formatDuration = (seconds) => {
  const minutes = Math.round((seconds || 0) / 60)
  return minutes < 60 ? `${minutes} 分钟` : `${Math.floor(minutes / 60)} 小时 ${minutes % 60} 分钟`
}
const shanghaiDate = (date = new Date()) => {
  const parts = new Intl.DateTimeFormat('en', { timeZone: 'Asia/Shanghai', year: 'numeric', month: '2-digit', day: '2-digit' }).formatToParts(date)
  const part = type => parts.find(item => item.type === type).value
  return `${part('year')}-${part('month')}-${part('day')}`
}

const App = {
  setup() {
    const status = ref({ online: false, latest: null })
    const history = ref({ date: '', events: [], intervals: [], present_seconds: 0 })
    const selectedDate = ref(shanghaiDate())
    const calendarMonth = ref(selectedDate.value.slice(0, 7))
    const calendarData = ref({ days: {} })
    const loading = ref(false)
    const currentPresent = computed(() => Boolean(status.value.latest?.person_present))
    const chart = computed(() => {
      const start = new Date(`${selectedDate.value}T00:00:00+08:00`).getTime()
      const end = start + 24 * 60 * 60 * 1000
      const events = [...history.value.events].map(event => ({ ...event, time: new Date(event.received_at).getTime() }))
        .filter(event => Number.isFinite(event.time) && event.time >= start && event.time <= end)
        .sort((a, b) => a.time - b.time)
      const isToday = selectedDate.value === shanghaiDate()
      const initialState = events.length ? !events[0].person_present : (isToday ? currentPresent.value : false)
      const points = [{ time: start, present: initialState }, ...events.map(event => ({ time: event.time, present: !!event.person_present, event }))]
      const lastState = points[points.length - 1].present
      points.push({ time: end, present: isToday ? currentPresent.value : lastState })
      const x = time => ((time - start) / (end - start) * 100).toFixed(3)
      const y = present => present ? 24 : 76
      const path = points.map((point, index) => `${index ? 'L' : 'M'} ${x(point.time)} ${y(point.present)}${index < points.length - 1 ? ` H ${x(points[index + 1].time)}` : ''}`).join(' ')
      const area = `${path} L 100 100 L 0 100 Z`
      return { path, area, points: events.map(event => ({ ...event, x: x(event.time), label: event.person_present ? '有人' : '无人' })) }
    })
    const timeline = computed(() => {
      const bins = Array(48).fill('')
      history.value.intervals.forEach(item => {
        const startDate = new Date(item.start); const endDate = new Date(item.end)
        const start = startDate.getHours() * 2 + Math.floor(startDate.getMinutes() / 30)
        const end = Math.min(47, endDate.getHours() * 2 + Math.ceil(endDate.getMinutes() / 30) - 1)
        for (let i = start; i <= end; i++) bins[i] = 'present'
      })
      return bins
    })
    const calendarDays = computed(() => {
      const [year, month] = calendarMonth.value.split('-').map(Number)
      const first = new Date(year, month - 1, 1).getDay()
      const count = new Date(year, month, 0).getDate()
      return [...Array(first === 0 ? 6 : first - 1).fill(null), ...Array.from({ length: count }, (_, i) => i + 1)]
    })
    const durationForDay = day => formatDuration(calendarData.value.days?.[`${calendarMonth.value}-${String(day).padStart(2, '0')}`] || 0)
    const durationClass = day => {
      const seconds = calendarData.value.days?.[`${calendarMonth.value}-${String(day).padStart(2, '0')}`] || 0
      if (!seconds) return 'duration-zero'
      if (seconds < 3600) return 'duration-short'
      if (seconds < 10800) return 'duration-medium'
      if (seconds < 21600) return 'duration-long'
      return 'duration-full'
    }
    const loadCalendar = async () => { calendarData.value = await fetch(`/family-7Kp9mQ2vLx4R/api/calendar?month=${calendarMonth.value}`).then(r => r.json()) }
    const load = async () => {
      loading.value = true
      try {
        status.value = await fetch('/family-7Kp9mQ2vLx4R/api/status').then(r => r.json())
        history.value = await fetch(`/family-7Kp9mQ2vLx4R/api/history?day=${selectedDate.value}`).then(r => r.json())
        calendarMonth.value = selectedDate.value.slice(0, 7)
        await loadCalendar()
      } finally { loading.value = false }
    }
    onMounted(() => { load(); const source = new EventSource('/family-7Kp9mQ2vLx4R/api/events'); source.onmessage = e => { if (JSON.parse(e.data).event !== 'regular_report') load() } })
    const chooseDay = day => { if (day) { selectedDate.value = `${calendarMonth.value}-${String(day).padStart(2, '0')}`; load() } }
    return { status, history, selectedDate, calendarMonth, calendarDays, durationForDay, durationClass, chooseDay, loadCalendar, loading, currentPresent, timeline, chart, formatDuration, load }
  },
  template: `
  <main><header><h1>Elderly <span>Monitor</span></h1><div class="updated">实时监护 · elder.wio.me</div></header>
  <section class="toolbar"><div><strong>历史日期</strong><span>追溯某一天的在家情况</span></div><div class="calendar"><div class="calendar-head"><button @click="calendarMonth = new Date(new Date(calendarMonth+'-01').setMonth(new Date(calendarMonth+'-01').getMonth()-1)).toISOString().slice(0,7); loadCalendar()">‹</button><b>{{ calendarMonth }}</b><button @click="calendarMonth = new Date(new Date(calendarMonth+'-01').setMonth(new Date(calendarMonth+'-01').getMonth()+1)).toISOString().slice(0,7); loadCalendar()">›</button></div><div class="week"><span v-for="d in ['一','二','三','四','五','六','日']">{{ d }}</span></div><div class="calendar-grid"><button v-for="(day,i) in calendarDays" :key="i" :class="{empty:!day, chosen:day && (calendarMonth + '-' + String(day).padStart(2,'0'))===selectedDate, [durationClass(day)]:day}" @click="chooseDay(day)" :disabled="!day"><b>{{ day || '' }}</b><small v-if="day">{{ durationForDay(day) }}</small></button></div></div><span v-if="loading" class="loading">正在更新…</span></section>
  <section class="summary"><article class="state-card" :class="currentPresent ? 'is-present' : 'is-away'"><small>当前状态</small><div class="state-line"><svg class="state-icon" viewBox="0 0 1024 1024" aria-label="居家状态"><path d="M690.005333 625.834667c-12.458667 0-23.893333 2.389333-34.474666 6.997333-0.853333-0.682667-1.194667-1.706667-2.048-2.389333L434.005333 412.16c-14.506667-14.506667-35.84-19.968-55.466666-14.506667l-0.512-0.512c-0.512 0.512-1.194667 0.512-1.706667 0.853334-7.509333 2.218667-14.336 5.973333-20.138667 11.093333-134.826667 81.066667-227.328 276.138667-227.328 437.76 0 22.528-3.242667 33.28 0 55.125333h422.570667V689.152l20.992 20.992c3.584 3.584 7.509333 6.485333 11.605333 9.045333 1.877333 19.968 18.261333 35.669333 38.229334 36.693334 20.138667 1.024 37.888-12.8 41.813333-32.426667h0.853333V715.093333c0-13.312 10.752-24.234667 24.234667-24.234666S713.386667 701.781333 713.386667 715.093333v187.050667h64.853333V715.093333c0.341333-23.552-8.874667-46.421333-25.429333-63.146666s-39.253333-26.112-62.805334-26.112z m120.832-399.018667c-3.413333-12.8-44.032-29.696-56.832-26.282667l-7.850666 2.048-38.570667-43.52c-4.096-4.608-24.576-25.429333-40.277333-34.474666-22.186667-12.8-194.389333 23.893333-216.576 57.856-9.386667 14.848-13.482667 32.085333-15.530667 40.277333l-13.482667 67.072-7.850666 2.048c-12.629333 3.242667-39.424 38.229333-36.181334 51.029333 1.536 6.314667 5.632 11.605333 11.264 14.677334 5.632 3.072 12.288 3.925333 18.432 2.218666l51.712-13.653333c12.8 67.072 71.509333 115.541333 139.776 115.541333 78.336 0.170667 141.994667-62.976 142.165334-141.312v-0.170666c0-16.554667-2.901333-32.426667-8.192-47.274667l60.928-16.554667c6.314667-1.536 11.605333-5.461333 14.848-11.093333 3.072-5.632 3.925333-12.288 2.218666-18.432z"></path></svg><strong>{{ currentPresent ? '有人在家' : '暂无人员' }}</strong></div><p>{{ status.online ? '设备在线 · 状态已同步' : '设备离线或等待上报' }}</p></article><article><small>选定日期在家时间</small><strong>{{ formatDuration(history.present_seconds) }}</strong><p>{{ selectedDate }}</p></article><article><small>设备连接</small><strong :class="status.online ? 'good' : 'bad'">{{ status.online ? '在线' : '离线' }}</strong><p>最近上报：{{ status.last_report ? new Date(status.last_report).toLocaleString() : '暂无' }}</p></article></section>
  <section class="timeline"><div class="section-head"><h2>{{ selectedDate }} 在家时间线</h2><span>绿色：有人 · 深色：无数据</span></div><div class="track"><i v-for="(value,n) in timeline" :key="n" :class="value"></i></div><div class="hours"><span>00:00</span><span>06:00</span><span>12:00</span><span>18:00</span><span>24:00</span></div><div class="intervals"><span v-for="item in history.intervals" :key="item.start">{{ new Date(item.start).toLocaleTimeString([], {hour:'2-digit',minute:'2-digit'}) }}–{{ new Date(item.end).toLocaleTimeString([], {hour:'2-digit',minute:'2-digit'}) }} · {{ formatDuration(item.seconds) }}</span><em v-if="!history.intervals.length">当天没有检测到有人记录</em></div></section>
  <section class="messages"><div class="section-head chart-heading"><div><h2>{{ selectedDate }} 居家状态趋势</h2><p class="chart-subtitle">根据事件记录还原有人 / 无人状态变化</p></div><span>{{ chart.points.length }} 条状态事件</span></div><div class="chart"><div class="y-labels"><span>有人</span><span>无人</span></div><div class="plot"><div class="grid-line top"></div><div class="grid-line middle"></div><div class="grid-line bottom"></div><svg viewBox="0 0 100 100" preserveAspectRatio="none" aria-label="居家状态趋势图"><path class="area" :d="chart.area"/><path class="line" :d="chart.path"/><circle v-for="(p,i) in chart.points" :key="i" :cx="p.x" :cy="p.person_present ? 24 : 76" r="1.25" :class="p.person_present ? 'person-point' : 'away-point'"><title>{{ new Date(p.time).toLocaleString() }} · {{ p.label }} · {{ p.event.event === 'person_entered' ? '检测到有人' : p.event.event === 'elderly_present' ? '持续在家' : p.event.event === 'long_time_absent' ? '长时间无人' : p.event.event }}</title></circle></svg><div class="x-labels"><span v-for="hour in [0,4,8,12,16,20,24]" :key="hour" :style="{left: (hour / 24 * 100) + '%'}">{{ String(hour).padStart(2,'0') }}:00</span></div></div></div><div class="chart-legend"><span><i class="legend-person"></i>有人</span><span><i class="legend-away"></i>无人</span><span class="event-count">状态点可悬停查看详情</span></div><p v-if="!chart.points.length" class="empty">当天没有符合筛选条件的事件</p></section>
  </main>`
}
createApp(App).mount('#app')
