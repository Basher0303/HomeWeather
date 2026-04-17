import { JSONFilePreset } from "lowdb/node";
import path from "path";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const filePath = path.join(__dirname, "db.json");

const defaultData = { records: [] };

const db = await JSONFilePreset(filePath, defaultData);

export default db;
