<template>
  <div class="pending-box">
    <a class="pending-tip">
      DCC <i>Live</i>
    </a>
  </div>
</template>

<script>
import {ElMessage} from "element-plus";
import api from "@/request/api";

export default {
  name: "PendingPage",
  mounted() {
    ElMessage.info("正在检查授权")
    api.get("/authcheck").then(res => {
      if (res.data.code === 'success') {
        this.$router.push('/overview')
        ElMessage.success('授权验证通过')
      } else {
        throw '未登录'
      }
    }).catch(reason => {
      ElMessage.error(reason)
      this.$router.push('/login')
    })
  }
}
</script>

<style scoped>
.pending-box {
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  height: 100%;
}

.pending-tip {
  color: #333;
  font-size: 32px;
  display: block;
  margin-top: -72px;
}
</style>