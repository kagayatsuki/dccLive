<template>
  <div class="login-frame">
    <div class="login-card">
      <el-card style="height: 100%">
        <a class="login-title">管理员登录</a>
        <div class="login-form">
          <span>用户名</span>
          <el-input maxlength="32" v-model="form.name" />
          <span>口令</span>
          <el-input maxlength="32" type="password" v-model="form.pwd" />
        </div>
        <el-button type="primary" :disabled="loginLock" @click="login">登录</el-button>
      </el-card>
    </div>
  </div>
</template>

<script>
import {ElMessage} from "element-plus";
import axios from "axios";

export default {
  name: "AdminLogin",
  data() {
    return {
      form: {
        name: '',
        pwd: ''
      },
      loginLock: false
    }
  },
  methods: {
    login() {
      this.loginLock = true
      let form = `user=${this.form.name}&pwd=${this.form.pwd}`
      axios.post('./api/login', form,
          { headers: {'content-type': 'application/x-www-form-urlencoded'} }).then(res => {
        if (res.data.code === 'success') {
          ElMessage.success("登录成功")
          document.cookie = `console_session=${res.data.data}`
          this.$router.push('/overview')
        } else {
          throw `登录失败: ${res.data.msg}`
        }
      }).catch(
          reason => ElMessage.error(reason)
      ).finally(() => this.loginLock = false)
    }
  }
}
</script>

<style scoped>
.login-frame {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 80%;
}

.login-card {
  width: 300px;
  height: 220px;
}

.login-title {
  display: block;
  font-size: 18px;
  padding-bottom: 24px;
}

.login-form {
  display: grid;
  grid-template-columns: 26% 74%;
  align-items: center;
  grid-row-gap: 16px;
  padding-bottom: 16px;
}

.login-form > span {
  font-size: 14px;
  color: #999;
}
</style>