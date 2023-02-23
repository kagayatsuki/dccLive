import axios from "axios";

const api = axios.create({
    baseURL: "./api",
    timeout: 6000,
    withCredentials: true,
    headers: {'content-type': 'application/x-www-form-urlencoded'}
});

export default api