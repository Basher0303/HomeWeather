const express = require("express");
const app = express();
const port = 8081;
const localIP = "192.168.0.166";

const data = {
    temperature: 0,
    co2: 0,
    humidity: 0,
    pressure: 0,
};

app.use(express.json());

app.get("/test", (req, res) => {
    res.json({
        status: "ok",
        message: "Server is working",
    });
});

app.post("/set", async (req, res) => {
    const body = req.body;
    data.temperature = Math.round(body.temperature);
    data.co2 = Math.round(body.co2);
    data.humidity = Math.round(body.humidity);
    data.pressure = Math.round(body.pressure / 133.322);
    console.log(body);
    res.json();
});

app.get("/get", async (req, res) => {
    res.json(data);
});

app.post("/get-alisa", async (req, res) => {
    const body = req.body;

    let responseText = "";

    responseText += `Температура: ${data.temperature} ${declOfNum(data.temperature, ["градус", "градуса", "градусов"])}.`;
    responseText += `Цэ о два: ${data.co2} ${declOfNum(data.co2, ["единица", "единицы", "единиц"])}.`;
    responseText += `Влажность: ${data.humidity} ${declOfNum(data.humidity, ["процент", "процента", "процентов"])}.`;
    responseText += `Давление: ${data.pressure} миллиметров ртутного столба.`;

    res.json({
        session: {
            session_id: body.session?.session_id,
            message_id: body.session?.message_id,
            user_id: body.session?.user_id,
        },
        response: {
            text: responseText.replaceAll("+", ""),
            tts: responseText,
            end_session: true,
        },
        version: body.version,
    });
});

// Запуск сервера
app.listen(port, () => {
    console.log(`Server is running at http://localhost:${port}`);
});

function convertSeconds(seconds) {
    const hours = Math.floor(seconds / 3600);
    const remainingSecondsAfterHours = seconds % 3600;

    const minutes = Math.floor(remainingSecondsAfterHours / 60);
    const remainingSeconds = Math.floor(remainingSecondsAfterHours % 60);

    return {
        hours: hours,
        minutes: minutes,
        seconds: remainingSeconds,
    };
}

function declOfNum(number, titles) {
    const cases = [2, 0, 1, 1, 1, 2];
    return titles[number % 100 > 4 && number % 100 < 20 ? 2 : cases[number % 10 < 5 ? number % 10 : 5]];
}
