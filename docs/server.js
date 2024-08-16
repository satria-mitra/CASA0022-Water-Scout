  // server.js
  const express = require('express');
  const { InfluxDB } = require('@influxdata/influxdb-client');

  const app = express();
  const port = 3000;

  // InfluxDB connection details
  const url = 'http://13.60.236.94:8086';
  const token = 'hgUCzvxU115d997v-3enMJ-1TfZf_Z_LyGUF0qJEruxS_ABkXijn0mLWx1lpRrB_d4eZ3SHxde-ObQVL8RVgiQ==';
  const org = 'casa';
  const bucket = 'mqtt-node';

  const queryApi = new InfluxDB({ url, token }).getQueryApi(org);

  // Serve static files from the "public" directory
  app.use(express.static('public'));

  // Fetch the latest water height from InfluxDB
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

  // Fetch water height data from the last 2 days
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

  // Define a route to get the latest water height
  app.get('/latest-water-height', async (req, res) => {
    const data = await fetchLatestWaterHeight();
    res.json(data);  // Send the fetched data as a JSON response
  });

  // Define a route to get water height data from the last 2 days
  app.get('/water-height-last-2-days', async (req, res) => {
    const data = await fetchWaterHeightLast2Days();
    res.json(data);  // Send the fetched data as a JSON response
  });

  // Start the server
  app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
  });
