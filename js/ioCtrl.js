const constants = ["ka", "kd", "ks"];
const axises = ["x", "y", "z"];
const colorType = ["r", "g", "b"];

// scaling
for (let j = 1; j <= 3; j++) {
    for (let i = 0; i < axises.length; i++) {
        document.getElementById(`c${j}-${axises[i]}-scale`).oninput = function() {
            document.getElementById(`c${j}-${axises[i]}-scale-value`).innerHTML = this.value;
            modelsConfig[`item${j}`].scale[i] = this.value;
        }
    }
}

// translation
for (let j =1; j <= 3; j++) {
    for (let i = 0; i < axises.length; i++) {
        document.getElementById(`c${j}-${axises[i]}-pos`).onchange = function() {
            if (!isNaN(this.value)) {
                modelsConfig[`item${j}`].pos[i] = parseFloat(this.value);
            }
        }
    }
}

// rotation
for (let j = 1; j <= 3; j++) {
    document.getElementById(`c${j}-autorotate`).onchange = function () {
        modelsConfig[`item${j}`].autorotate = this.checked;
    }

    for (let i = 0; i < axises.length; i++) {
        document.getElementById(`c${j}-rotate-axis-${axises[i]}`).onchange = function() {
            if (this.checked) {
                modelsConfig[`item${j}`].rotateAxis = [0, 0, 0]; 
                modelsConfig[`item${j}`].rotateAxis[i] = 1; 
            }
        }
    }

    document.getElementById(`c${j}-rotate-degree`).onchange = function () {
        if (modelsConfig[`item${j}`].autorotate === false && !isNaN(this.value)) {
            modelsConfig[`item${j}`].rotateDegree = this.value;
        }
    }
}

// shear
for (let j = 1; j <= 3; j++) {
    document.getElementById(`c${j}-shear-degree`).onchange = function () {
        modelsConfig[`item${j}`].shearDegree = this.value;
    }
}

// constant
for (let j = 1; j <= 3; j++) {
    for (let i = 0; i < constants.length; i++) {
        document.getElementById(`c${j}-${constants[i]}`).oninput = function() {
            document.getElementById(`c${j}-${constants[i]}-value`).innerHTML = this.value;
            modelsConfig[`item${j}`][constants[i]] = this.value;
        }
    }

    document.getElementById(`c${j}-shininess`).onchange = function() {
        if (!isNaN(this.value) && this.value >= 0) {
            modelsConfig[`item${j}`].shininess = this.value;
        }
    }
}

// light position
for (let j =1; j <= 3; j++) {
    for (let i = 0; i < axises.length; i++) {
        document.getElementById(`c${j}-light-${axises[i]}-pos`).onchange = function() {
            if (!isNaN(this.value)) {
                lightsConfig[`light${j}`].pos[i] = parseFloat(this.value);
            }
        }
    }
}

// light color
for (let j =1; j <= 3; j++) {
    for (let i = 0; i < colorType.length; i++) {
        document.getElementById(`c${j}-light-color-${colorType[i]}`).onchange = function () {
            if (!isNaN(this.value)) {
                lightsConfig[`light${j}`].color[i] = parseFloat(this.value) / 255;
            }
        }
    }
}

// model
for (let j = 1; j <= 3; j++) {
    document.getElementById(`c${j}-model-value`).onchange = function () {
        modelsConfig[`item${j}`].model = this.value;
    }
}

// shader
for (let j = 1; j <= 3; j++) {
    document.getElementById(`c${j}-shader-value`).onchange = function () {
        modelsConfig[`item${j}`].shader = this.value;
    }
}