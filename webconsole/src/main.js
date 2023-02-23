import { createApp } from 'vue'
import App from './App.vue'
import installer from 'element-plus'
import 'element-plus/theme-chalk/index.css'
import 'element-plus/theme-chalk/dark/css-vars.css'

import router from "@/router";

const app = createApp(App);
app.use(router);
installer.install(app)
app.mount('#app');
