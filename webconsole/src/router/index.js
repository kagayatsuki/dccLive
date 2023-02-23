import {createRouter, createWebHashHistory} from "vue-router/dist/vue-router";
import AdminLogin from "@/pages/AdminLogin";
import PendingPage from "@/pages/PendingPage";
import StatusOverview from "@/pages/StatusOverview";
import LogViewer from "@/pages/LogViewer";

const routes = [
    {path: '/', component: PendingPage},
    {path: '/login', component: AdminLogin},
    {path: '/overview', component: StatusOverview},
    {path: '/log', component: LogViewer}
];

const router = createRouter({
    history: createWebHashHistory(),
    routes
});

export default router;