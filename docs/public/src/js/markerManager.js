
//Component to visualise Time on a a-text entity
AFRAME.registerComponent('timenow',
    {
        init: function () {
            // Set up the tick throttling. Slow down to 500ms
            this.tick = AFRAME.utils.throttleTick(this.tick, 500, this);
        },
        tick: function () {
            this.el.setAttribute('value', this.getTime());
        },
        getTime: function () {
            var d = new Date();
            return d.toLocaleTimeString();
        }
    })


//Listen to the markers
AFRAME.registerComponent('registerevents', {
    init: function () {
        const handler = this.el;

        handler.addEventListener("markerFound", async (event) => {
            const markerId = event.target.id;
            console.log('Marker Found: ', markerId);

            // Fetch data from the InfluxDB through your Node.js server
            const data = await this.fetchInfluxDBData();

            // Update the display with the fetched data
            this.updateDisplay(event.target, data);

            // Look for the model entity associated with this marker
            const modelEntity = event.target.querySelector('#modelplant_' + markerId);

            if (modelEntity) {
                console.log('Model entity found:', modelEntity);

                // The model-loaded event listener
                modelEntity.addEventListener('model-loaded', (e) => {
                    console.log('Model loaded event triggered for marker: ', markerId);
                    this.updatePlanePosition(e.detail.model, data.distance);
                });

                // If the model is already loaded (in case the event was missed)
                if (modelEntity.getObject3D('mesh')) {
                    console.log('Model already loaded for marker: ', markerId);
                    this.updatePlanePosition(modelEntity.getObject3D('mesh'), data.distance);
                }
            } else {
                console.log('No model entity found for this marker.');
            }
        });

        handler.addEventListener("markerLost", (event) => {
            const markerId = event.target.id;
            console.log('Marker Lost: ', markerId);
        });
    },

    fetchInfluxDBData: async function() {
        try {
            const response = await fetch('/latest-data'); 
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            const data = await response.json();
            console.log('Fetched data from InfluxDB:', data);
            return data;
        } catch (error) {
            console.error('Error fetching data from server:', error);
            return {};
        }
    },

    updateDisplay: function(marker, data) {
        if (data.distance !== undefined) {
            marker.querySelector('#distance-text_' + marker.id).setAttribute('value', 'Water Level: ' + data.distance/1000 + 'm');
        }

        if (data.battery_voltage !== undefined) {
            marker.querySelector('#batt_voltage-text_' + marker.id).setAttribute('value', 'Voltage: ' + parseFloat(data.battery_voltage).toFixed(2) + ' V');
        }

        if (data.battery_percentage !== undefined) {
            marker.querySelector('#percentage-text_' + marker.id).setAttribute('value', 'Battery Level: ' + data.battery_percentage + '%');
        }
        if (data.battery_status !== undefined) {
            marker.querySelector('#status-text_' + marker.id).setAttribute('value', 'Battery Status: ' + data.battery_status);
        }
    },

    updatePlanePosition: function(model, distance) {
        console.log('Updating Plane position based on distance:', distance);
        console.log("distance value :", distance);
        // Calculate the y position based on the distance
        //var calculatedDistance = distance* -1 + 4500;
        //const yPosition = -1.2 + (calculatedDistance / 7000) * 2.2;

        // Traverse the model and update the y position and color of the plane
        model.traverse((node) => {
            if (node.material) {
                console.log('Material:', node.material);
            }
            if (node.name === 'Plane' && node.type === 'Mesh') {
                console.log('Plane found:', node);
                //node.position.y = yPosition; // Update the y position based on the distance
                node.material.color.r= 100/255; // Change the color to light blue
                node.material.color.g= 240/255; // Change the color to light blue
                node.material.color.b= 255/255; // Change the color to light blue

                node.material.needsUpdate = true; // Ensure the material updates
                console.log('Updated Plane position and color:', node.position, node.material.color);
            }
        });
    }
});





AFRAME.registerComponent('markers_start_json', {
    schema: { 
      subtopics: {type: 'array'}
    },
      
      init: function () {
  
          console.log('Add markers to the scene');
  
          let sceneEl = document.querySelector('a-scene');
  
          fetch("./markers.json")
              .then(response => response.json())
              .then(json => {
                  console.log(json.content);
  
                  json.content.forEach(el => {
                      let markerURL = './src/patt/' + el.markerName + '.patt';
                      let markerEl = document.createElement('a-marker');
                      markerEl.setAttribute('type', 'pattern');
                      markerEl.setAttribute('url', markerURL);
                      markerEl.setAttribute('id', el.topic);
                      sceneEl.appendChild(markerEl);
  
                    //   let textEl = document.createElement('a-entity');
                    //   textEl.setAttribute('id', 'text' + el.textContent);
                    //   textEl.setAttribute('text', { color: 'red', align: 'center', value: el.textContent, width: '6' });
                    //   textEl.object3D.position.set(-0.0, 0.1, 0.5);
                    //   textEl.setAttribute('rotation', { x: 0, y: 0, z: 0 });
                    //   markerEl.appendChild(textEl);
  
                      let plantRT = document.createElement('a-entity');
                      plantRT.setAttribute('id', 'planRT_' + el.topic);
                      plantRT.setAttribute('rotation', { x: 0, y: 0, z: 0 });
                      plantRT.object3D.position.set(0, 0, 0.2);
                      plantRT.object3D.scale.set(1, 1, 1);
                      markerEl.appendChild(plantRT);
  
                      let screen = document.createElement('a-entity');
                      screen.setAttribute('id', 'screen_' + el.topic);
                      screen.setAttribute('gltf-model', '#display_glb');
                      screen.object3D.position.set(-2, 2, -0.01);
                      screen.object3D.scale.set(1.2, 1.2, 1.2);
                      screen.setAttribute('rotation', { x: 0, y: 0, z: 0 });
                      plantRT.appendChild(screen);
  
                      let modelplant = document.createElement('a-entity');
                      modelplant.setAttribute('id', 'modelplant_' + el.topic);
                      modelplant.setAttribute('gltf-model', '#plant_gltf');
                      modelplant.setAttribute('rotation', { x: 0, y: 180, z: 0 });
                      modelplant.object3D.position.set(-0.2, 0, 0);
                      modelplant.object3D.scale.set(0.15, 0.15, 0.15);
                      modelplant.setAttribute('inspect-model', ''); // Add the inspect-model component here
                      plantRT.appendChild(modelplant);

                      let titleText = document.createElement('a-text');
                      titleText.setAttribute('id', 'titleText_' + el.topic);
                      titleText.setAttribute('value', 'Live House Mill Data');
                      titleText.setAttribute('width', '3.9');
                      titleText.setAttribute('anchor', 'left');
                      titleText.object3D.position.set(-0.8, 0.85, 0.02);
                      screen.appendChild(titleText);
  
                      let timenowText = document.createElement('a-text');
                      timenowText.setAttribute('id', 'timenowText_' + el.topic);
                      timenowText.setAttribute('timenow', '');
                      timenowText.setAttribute('value', 't');
                      timenowText.setAttribute('width', '3.5');
                      timenowText.setAttribute('anchor', 'left');
                      timenowText.object3D.position.set(-0.9, -0.8, 0.02);
                      screen.appendChild(timenowText);
  
                      let distanceext = document.createElement('a-text');
                      distanceext.setAttribute('id', 'distance-text_' + el.topic);
                      distanceext.setAttribute('value', 'Water Level:');
                      distanceext.setAttribute('width', '3.5');
                      distanceext.setAttribute('anchor', 'left');
                      distanceext.object3D.position.set(-0.9, 0.4, 0.02);
                      screen.appendChild(distanceext);
  
                      let voltageText = document.createElement('a-text');
                      voltageText.setAttribute('id', 'batt_voltage-text_' + el.topic);
                      voltageText.setAttribute('value', 'Voltage:');
                      voltageText.setAttribute('width', '3.5');
                      voltageText.setAttribute('anchor', 'left');
                      voltageText.object3D.position.set(-0.9, 0.1, 0.02);
                      screen.appendChild(voltageText);
  
                      let percentageText = document.createElement('a-text');
                      percentageText.setAttribute('id', 'percentage-text_' + el.topic);
                      percentageText.setAttribute('value', 'Percentage:');
                      percentageText.setAttribute('width', '3.5');
                      percentageText.setAttribute('anchor', 'left');
                      percentageText.object3D.position.set(-0.9, -0.2, 0.02);
                      screen.appendChild(percentageText);

                      let batteryStatusText = document.createElement('a-text');
                      batteryStatusText.setAttribute('id', 'status-text_' + el.topic);
                      batteryStatusText.setAttribute('value', 'Status:');
                      batteryStatusText.setAttribute('width', '3.5');
                      batteryStatusText.setAttribute('anchor', 'left');
                      batteryStatusText.object3D.position.set(-0.9, -0.5, 0.02);
                      screen.appendChild(batteryStatusText);
  
                  });
              });
      }
  });