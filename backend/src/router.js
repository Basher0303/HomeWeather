import express from "express";
import db from "./db/db.js";
import { declOfNum } from "./utils.js";

const router = express.Router();

router.get("/test", (req, res) => {
    res.json({
        status: "ok",
        message: "Server is working",
    });
});

router.post("/set", async (req, res) => {
    const body = req.body || {};
    const data = {};
    body.temperature && (data.temperature = Math.round(body.temperature));
    body.co2 && (data.co2 = Math.round(body.co2));
    body.humidity && (data.humidity = Math.round(body.humidity));
    body.pressure && (data.pressure = Math.round(body.pressure / 133.322));
    data.timestamp = Date.now();
    await db.update(({ records }) => {
        records.push(data);

        if (records.length > 10) {
            records.splice(0, records.length - 10);
        }
    });
    res.json();
});

router.get("/get", async (req, res) => {
    const records = db.data?.records || [];
    const lastRecord = records.at(-1) || {};
    const last10 = records.filter((r) => r.timestamp).slice(-10);
    const intervals = [];

    for (let i = 1; i < last10.length; i++) {
        const diff = last10[i].timestamp - last10[i - 1].timestamp;
        intervals.push(diff);
    }

    const avgInterval = intervals.reduce((sum, v) => sum + v, 0) / intervals.length;

    res.json({
        ...lastRecord,
        avgInterval,
    });
});

router.post("/get-alisa", async (req, res) => {
    const body = req.body;

    db.read;
    const records = db.data?.records || [];
    const lastRecord = records.at(-1) || {};
    let responseText = "";

    responseText += `Температура: ${lastRecord?.temperature} ${declOfNum(lastRecord?.temperature, ["градус", "градуса", "градусов"])}.`;
    responseText += `Цэ о два: ${lastRecord?.co2} ${declOfNum(lastRecord?.co2, ["единица", "единицы", "единиц"])}.`;
    responseText += `Влажность: ${lastRecord?.humidity} ${declOfNum(lastRecord?.humidity, ["процент", "процента", "процентов"])}.`;
    responseText += `Давление: ${lastRecord?.pressure} миллиметров ртутного столба.`;

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

export default router;
