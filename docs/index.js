const express = require('express');
const path = require('path');
const { fetchLatestWaterHeight } = require('./influxdbClient');

const app = express();
const port = 3000;

app.use(express.static(path.join(__dirname, 'public')));

app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Route to fetch water height and print to console
app.get('/fetch-water-height', async (req, res) => {
  try {
    const waterHeight = await fetchLatestWaterHeight();
    console.log(`Latest Water Height: ${waterHeight}`);
    res.send(`Latest Water Height: ${waterHeight}`);
  } catch (error) {
    console.error('Error fetching water height:', error);
    res.status(500).send('Failed to fetch water height');
  }
});

app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
