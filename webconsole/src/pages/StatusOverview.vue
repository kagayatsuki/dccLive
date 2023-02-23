<template>
  <div class="overview-wrapper">
    <el-card>
      <div class="title-wrapper">
        <a class="icon-block"></a>
        <span class="title-block">WEB CONSOLE</span>
      </div>
    </el-card>
    <el-card v-loading="api_status.loading">
      <span class="api-access-data">当前用户: {{ api_status.current_user }}</span>
      <span class="api-access-data">接口访问次数: {{ api_status.access_count }}</span>
      <span class="api-access-data">直播间数量: </span>
      <el-progress :percentage="api_status.room_count / api_status.room_max * 100" :text-inside="true"
                   :stroke-width="16">
        <span>{{ `${api_status.room_count}/${api_status.room_max}` }}</span>
      </el-progress>
    </el-card>
    <el-card>
      <el-button @click="dialog.new = true">新建直播间</el-button>
      <el-button :disabled="true">管理员列表</el-button>
      <el-button :disabled="true">切断所有直播</el-button>
      <el-button :disabled="true">发送全体广播</el-button>
      <el-button @click="gotoLogViewer">系统日志</el-button>
    </el-card>
    <el-card class="room-card" v-for="(item, index) in room" :key="index">
      <span class="room-title">{{item.title}}</span>
      <div class="room-data">
        <span>主办单位: {{item.applier_mark}}</span>
        <span>最高观看: {{item.watched}}</span>
        <span>开始时间: {{item.time_start}}</span>
      </div>
      <div class="room-note">
        <i>{{item.note}}</i>
      </div>
      <div class="room-option">
        <el-button type="primary" size="small" @click="showStreamingKey(item)">查看推流密钥</el-button>
        <el-button size="small" @click="exportAccessLog(item)">导出访问日志</el-button>
        <el-button size="small" @click="exportWatchingData(item)">导出观看数据</el-button>
        <el-button type="danger" size="small" @click="confirmDeleteRoom(item)">删除直播间</el-button>
        <el-button type="danger" size="small" :disabled="true">关闭直播间</el-button>
      </div>
    </el-card>
    <el-dialog v-model="dialog.new" title="新建直播间" width="400" center>
      <el-form label-width="auto">
        <el-form-item label="标题">
          <el-input maxlength="32" v-model="form.new.title" />
        </el-form-item>
        <el-form-item label="主办单位">
          <el-input maxlength="32" v-model="form.new.dep" />
        </el-form-item>
        <el-form-item label="介绍信息">
          <el-input type="textarea" maxlength="200" v-model="form.new.note" />
        </el-form-item>
        <el-form-item label="开始时间">
          <el-date-picker v-model="form.new.start_time" type="datetime" placeholder="选择日期时间" />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button type="primary" style="margin: 0 auto" @click="postNewRoom">创建</el-button>
      </template>
    </el-dialog>
    <el-dialog v-model="dialog.delete" title="安全操作" width="350" center>
      <div style="display: flex; flex-direction: column; justify-content: center; align-items: center">
        <a style="display: block; box-sizing: border-box; padding: 8px 16px; background: rgba(255,255,255,.1); user-select: none">
          {{form.delete.id}}
        </a>
        <span style="margin: 8px 0">请在下方输入上方的直播间ID</span>
        <el-input maxlength="12" v-model="form.delete.input" style="width: 140px; margin-top: 6px" />
      </div>
      <template #footer>
        <el-button :disabled="form.delete.input !== form.delete.id" @click="postDeleteRoom">删除直播间</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script>
import {ElMessage, ElMessageBox} from "element-plus";
import api from "@/request/api";
import encodeObj2Form from "@/utils/encodedForm";

export default {
  name: "StatusOverview",
  data() {
    return {
      api_status: {
        access_count: 0,
        room_count: 2,
        room_max: 10,
        current_user: '',
        loading: false
      },
      room: [
        {
          title: '直播间',
          note: '直播间介绍内容',
          applier_mark: '院团委',
          watched: 100,
          time_start: '2023-2-23 00:00:00',
          id: 'Azv297be3f',
          push_key: 'tk=d9adf1fc45cab29e&id=roomAzv297be3f'
        }
      ],
      dialog: {
        new: false,
        delete: false
      },
      form: {
        new: {
          title: '',
          dep: '',
          start_time: '',
          note: ''
        },
        delete: {
          id: '',
          input: ''
        }
      }
    }
  },
  mounted() {
    this.getRoomList()
    this.getSysInfo()
  },
  methods: {
    showStreamingKey(room) {
      ElMessageBox.alert(`${room.push_key}`, '推流密钥')
    },
    linkFile(src) {
      const a = document.createElement('a')
      a.href = src
      a.target = '_blank'
      a.click()
      a.remove()
    },
    exportAccessLog(room) {
      this.linkFile(`./api/live/exportAccess?id=${room.id}`)
    },
    exportWatchingData(room) {
      this.linkFile(`./api/live/exportWatched?id=${room.id}`)
    },
    confirmDeleteRoom(room) {
      this.form.delete.id = room.id
      this.dialog.delete = true
    },
    postDeleteRoom() {
      ElMessage.info('正在请求删除直播间')
      this.dialog.delete = false
      this.form.delete.input = ''
      api.post('/live/delete', encodeObj2Form({id: this.form.delete.id})).then(res => {
        if (res.data.code === 'success') {
          ElMessage.success('直播间删除成功')
          this.getRoomList()
          this.getSysInfo()
        } else {
          throw `操作失败: ${res.data.msg}`
        }
      }).catch(reason => ElMessage.error(reason))
    },
    getRoomList() {
      ElMessage.info('正在获取直播间列表')
      api.get('/live/list').then(res => {
        if (res.data.code === 'success') {
          ElMessage.success('获取成功')
          this.room = res.data.data
        } else
          throw `获取失败: ${res.data.msg}`
      }).catch(reason => ElMessage.error(reason))
    },
    postNewRoom() {
      ElMessage.info('正在请求创建直播间')
      this.dialog.new = false
      api.post('/live/new', encodeObj2Form(this.form.new)).then(res => {
        if (res.data.code === 'success') {
          ElMessage.success('新建直播间成功')
          this.getRoomList()
          this.getSysInfo()
        } else
          throw `操作失败: ${res.data.msg}`
      }).catch(reason => ElMessage.error(reason))
    },
    getSysInfo() {
      ElMessage.info('获取系统基本信息')
      api.get('/live/info').then(res => {
        if (res.data.code === 'success')
          this.api_status = res.data.data
        else
          throw `信息获取失败: ${res.data.msg}`
      }).catch(reason => ElMessage.error(reason))
    },
    gotoLogViewer() {
      this.$router.push('/log')
    }
  }
}
</script>

<style scoped>
.title-wrapper {
  display: flex;
  align-items: center;
  justify-content: center;
}

.icon-block {
  display: inline-block;
  width: 0;
  height: 0;
  border-style: solid;
  border-width: 14px 0 14px 22px;
  border-color: transparent transparent transparent #00a8ff;
  border-radius: 5px;
}

.title-block {
  margin: 0 16px;
  font-weight: 600;
  font-size: 24px;
  color: #aaa;
}

.overview-wrapper {
  display: flex;
  flex-direction: column;
  width: 100%;
  box-sizing: border-box;
  padding: 12px;
  max-width: 800px;
  margin: 0 auto;
  text-align: left;
  gap: 16px;
}

.api-access-data {
  display: block;
  margin: 6px 0;
}

.room-card {
  color: #666;
}

.room-title {
  color: #aaa;
  padding-bottom: 6px;
}

.room-data {
  font-size: 14px;
  padding-bottom: 4px;
}

.room-data > span {
  padding-right: 24px;
}

.room-note {
  font-size: 14px;
}

.room-option {
  display: block;
  width: 100%;
  margin-top: 5px;
  border-top: 1px solid rgba(255, 255, 255, .1);
  box-sizing: border-box;
  padding-top: 6px;
}
</style>