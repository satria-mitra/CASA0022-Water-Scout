const express = require('express');
const { InfluxDB } = require('@influxdata/influxdb-client');
const cors = require('cors');
require('dotenv').config();

const app = express();
const port = process.env.PORT || 3000;

// InfluxDB connection details using environment variables
const url = process.env.INFLUXDB_URL;
const token = process.env.INFLUXDB_TOKEN;
const org = process.env.INFLUXDB_ORG;
const bucket = process.env.INFLUXDB_BUCKET;

const queryApi = new InfluxDB({ url, token }).getQueryApi(org);

// Enable CORS for all routes
app.use(cors());

// Serve static files from the "public" directory
app.use(express.static('public'));

// Serve models from the "src/models" directory
app.use('/src/models', express.static('src/models'));

// Serve textures from the "src/textures" directory
app.use('/src/textures', express.static('src/textures'));

// Define routes for your APIs
app.get('/latest-water-height', async (req, res) => {
    const data = await fetchLatestWaterHeight();
    res.json(data);
});

app.get('/water-height-last-2-days', async (req, res) => {
    const data = await fetchWaterHeightLast2Days();
    res.json(data);
});

// Error handling middleware
app.use((err, req, res, next) => {
    console.error(err.stack);
    res.status(500).send('Something went wrong!');
});

// Start the server
app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
});

async function fetchLatestWaterHeight() {
    const query = `
        from(bucket: "${bucket}")
            |> range(start: -24h)
            |> filter(fn: (r) => r["_measurement"] == "water-level")
            |> filter(fn: (r) => r["_field"] == "distance")
            |> last()`;

    let result = null;

    try {
        const rows = await queryApi.collectRows(query);
        if (rows.length > 0) {
            result = rows[0];
        }
    } catch (error) {
        console.error('Error fetching data from InfluxDB:', error);
    }

    return result;
}

async function fetchWaterHeightLast2Days() {
    const query = `
        from(bucket: "${bucket}")
            |> range(start: -2d)
            |> filter(fn: (r) => r["_measurement"] == "water-level")
            |> filter(fn: (r) => r["_field"] == "distance")
            |> aggregateWindow(every: 1h, fn: mean)
            |> yield(name: "mean")`;

    let results = [];

    try {
        results = await queryApi.collectRows(query);
    } catch (error) {
        console.error('Error fetching data from InfluxDB:', error);
    }

    return results;
}
