import express from "express";
import router from "./router.js";

const port = 8081;

const app = express();

// Запуск сервера
app.listen(port, () => {
    console.log(`Server is running at http://localhost:${port}`);
});
app.use(express.json());
app.use("/", router);

const data = {
    temperature: 0,
    co2: 0,
    humidity: 0,
    pressure: 0,
};
