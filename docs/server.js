const express = require('express');
const path = require('path');
const { fetchLatestWaterHeight, fetchWaterHeightLast2Days } = require('./influxdbClient');

const app = express();
const PORT = process.env.PORT || 3000;

app.use(express.static(path.join(__dirname, 'public')));

app.get('/latest-water-height', async (req, res) => {
  try {
    const data = await fetchLatestWaterHeight();
    res.json({ _value: data });
  } catch (error) {
    res.status(500).send({ error: 'Failed to fetch latest water height' });
  }
});

app.get('/water-height-last-2-days', async (req, res) => {
  try {
    const data = await fetchWaterHeightLast2Days();
    res.json(data);
  } catch (error) {
    res.status(500).send({ error: 'Failed to fetch water height data for last 2 days' });
  }
});

app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

app.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});
