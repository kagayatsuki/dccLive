<template>
<div class="logs-page">
  <el-card>
    <el-button @click="getLogs('admin')">Admin</el-button>
    <el-button @click="getLogs('svr')">Service</el-button>
    <el-button @click="getLogs('cli')">Client</el-button>
    <el-button @click="getLogs('sys')">System</el-button>
  </el-card>
  <div class="log-wrapper">
    <span style="white-space: pre-line">{{logData}}</span>
  </div>
</div>
</template>

<script>
import api from "@/request/api";
import {ElMessage} from "element-plus";

export default {
  name: "LogViewer",
  data() {
    return {
      logData: '选择目标分类以查看日志信息'
    }
  },
  methods: {
    getLogs(type) {
      ElMessage.info('正在获取日志数据')
      const url = `/log/${type}`
      api.get(url).then(res => {
        this.logData = decodeURIComponent(res.data)
      }).catch(reason => ElMessage.error(reason))
    }
  }
}
</script>

<style scoped>
.log-wrapper {
  display: block;
  width: 100%;
  box-sizing: border-box;
  padding: 6px;
  text-align: left;
  font-size: 14px;
}
</style>