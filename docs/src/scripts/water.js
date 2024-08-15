import * as THREE from './three.module.js'; // Use the correct relative path to three.module.js
import { Water } from 'three/examples/jsm/objects/Water.js';

const waterNormalsPath = './src/textures/waternormals.jpg';

export const ReplacePlanesWithWater = async (scene, width, length, position) => {
  const waterObjects = [];

  // Manually define the size and position of the water plane
  const water = await createManualWaterPlane(width, length, position, scene);
  waterObjects.push(water);
  scene.add(water);

  return waterObjects;
};

async function createManualWaterPlane(width, length, position, scene) {
  return new Promise((resolve) => {
    const waterGeometry = new THREE.PlaneGeometry(width, length);

    new THREE.TextureLoader().load(waterNormalsPath, (texture) => {
      texture.wrapS = texture.wrapT = THREE.RepeatWrapping;

      let waterOptions = {
        textureWidth: 512,
        textureHeight: 512,
        waterNormals: texture,
        sunDirection: new THREE.Vector3(),
        sunColor: 0xffffff,
        waterColor: 0x00faff,
        distortionScale: 3.7,
        fog: scene.fog !== undefined,
      }

      const water = new Water(waterGeometry, waterOptions);

      water.rotation.x = -Math.PI / 2;
      water.position.copy(position);

      // Debugging: Log the water object to ensure it's correctly initialized
      //console.log('Water Object:', water);

      resolve(water);
    });
  });
}

export const animateWater = (water) => {
  // Debugging: Log to check if the water object is being updated
  //console.log('Animating Water:', water);
  
  if (water && water.material && water.material.uniforms && water.material.uniforms['time']) {
    water.material.uniforms['time'].value += 1.0 / 360.0;
  } else {
    console.warn('Water uniforms not set properly:', water.material.uniforms);
  }
}
