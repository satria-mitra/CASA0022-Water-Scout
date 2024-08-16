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
        latestValue = o._value;
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

async function fetchWaterHeightLast2Days() {
  const query = `
    from(bucket: "${bucket}")
      |> range(start: -2d)
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

module.exports = {
  fetchLatestWaterHeight,
  fetchWaterHeightLast2Days,
};
