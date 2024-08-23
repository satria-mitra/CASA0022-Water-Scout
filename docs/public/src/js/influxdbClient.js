require('dotenv').config();
const { InfluxDB } = require('@influxdata/influxdb-client');

const url = process.env.INFLUXDB_URL;
const token = process.env.INFLUXDB_TOKEN;
const org = process.env.INFLUXDB_ORG;
const bucket = process.env.INFLUXDB_BUCKET;

const queryApi = new InfluxDB({ url, token }).getQueryApi(org);

async function fetchLatestWaterHeight() {
  const query = `
    from(bucket: "${bucket}")
      |> range(start: -1h)
      |> filter(fn: (r) => r["_measurement"] == "water-level")
      |> filter(fn: (r) => r["_field"] == "distance")
      |> last()`;

  return new Promise((resolve, reject) => {
    let latestValue;
    queryApi.queryRows(query, {
      next(row, tableMeta) {
        const o = tableMeta.toObject(row);
        // Perform the calculation: distance * -1 + 4500
        latestValue = o._value * -1 + 4500;
      },
      error(error) {
        console.error('Query failed', error);
        reject(error);
      },
      complete() {
        resolve(latestValue);
      }
    });
  });
}


async function fetchLatestBatteryData() {
  const query = `
    from(bucket: "${bucket}")
      |> range(start: -1h)
      |> filter(fn: (r) => r["_measurement"] == "water-level")
      |> filter(fn: (r) => r["_field"] == "distance" or r["_field"] == "battery_percentage" or r["_field"] == "battery_voltage" or r["_field"] == "battery_status" or r["_field"] == "rssi" or r["_field"] == "sni" or r["_field"] == "DateTime")
      |> last()`;

  return new Promise((resolve, reject) => {
    const latestValues = {};
    queryApi.queryRows(query, {
      error(error) {
        console.error('Query failed', error);
        reject(error);
      },
      complete() {
        resolve(latestValues);
      }
    });
  });
}



async function fetchWaterHeightLast2Days() {
  const query = `
    from(bucket: "${bucket}")
      |> range(start: -1d)
      |> filter(fn: (r) => r["_measurement"] == "water-level")
      |> filter(fn: (r) => r["_field"] == "distance")
      |> sort(columns: ["_time"], desc: true)
  `;

  return new Promise((resolve, reject) => {
    const results = [];
    queryApi.queryRows(query, {
      next(row, tableMeta) {
        const o = tableMeta.toObject(row);
        results.push({
          time: o._time,
          value: o._value,
        });
      },
      error(error) {
        console.error('Query failed', error);
        reject(error);
      },
      complete() {
        resolve(results);
      }
    });
  });
}

async function fetchLatestData() {
  const query = `
    from(bucket: "${bucket}")
      |> range(start: -24h)
      |> filter(fn: (r) => r["_measurement"] == "water-level")
      |> filter(fn: (r) => r["_field"] == "distance" or r["_field"] == "battery_percentage" or r["_field"] == "battery_voltage" or r["_field"] == "battery_status" or r["_field"] == "solar_status" or r["_field"] == "snr" or r["_field"] == "rssi")
      |> last()`;

  let result = {};

  try {
    const rows = await queryApi.collectRows(query);

  } catch (error) {
    console.error('Error fetching data from InfluxDB:', error);
  }

  return result;
}



module.exports = {
  fetchLatestWaterHeight,
  fetchWaterHeightLast2Days,
  fetchLatestBatteryData,
  fetchLatestData
};
