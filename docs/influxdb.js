const { InfluxDB } = require('@influxdata/influxdb-client');

const url = 'http://13.60.236.94:8086';
const token = 'hgUCzvxU115d997v-3enMJ-1TfZf_Z_LyGUF0qJEruxS_ABkXijn0mLWx1lpRrB_d4eZ3SHxde-ObQVL8RVgiQ=='; 
const org = 'casa';
const bucket = 'mqtt-node';

const queryApi = new InfluxDB({ url, token }).getQueryApi(org);

async function fetchLatestWaterHeight() {
  const query = `
    from(bucket: "${bucket}")
      |> range(start: -24h)
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

module.exports = { fetchLatestWaterHeight };
