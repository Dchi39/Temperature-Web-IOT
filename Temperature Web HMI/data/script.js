/* ================= GLOBAL ================= */
let ModeSet = "heatoff";

/* ================= INIT ================= */
document.addEventListener("DOMContentLoaded", () => {
  getOutput();
});

/* ================= GET DATA FROM ESP32 ================= */
function getOutput() {
  const xhr = new XMLHttpRequest();
  xhr.open("GET", "/getOutput", true);

  xhr.onload = function () {
    if (xhr.status !== 200) return;

    const data = JSON.parse(xhr.responseText);


    // ---- Update sliders ----
    setSlider("c1", "coil1", data.c1);
    setSlider("c2", "coil2", data.c2);
    setSlider("c3", "coil3", data.c3);
    setSlider("c4", "coil4", data.c4);

    // ---- Update mode ----
    ModeSet = data.mode;
    updateModeButtons(data.mode);
  };


  xhr.send();
}




/* ================= HELPER ================= */
function setSlider(sliderId, valueId, value) {
  document.getElementById(sliderId).value = value;
  document.getElementById(valueId).innerText = value;
}

function updateModeButtons(mode) {
  document.getElementById("heaton").disabled  = (mode === "heaton");
  document.getElementById("heatoff").disabled = (mode === "heatoff");
  document.getElementById("ideal").disabled   = (mode === "ideal");
}



/* ================= SLIDER CHANGE ================= */
function updateSliderValue() {
  const c1 = document.getElementById("c1").value;
  const c2 = document.getElementById("c2").value;
  const c3 = document.getElementById("c3").value;
  const c4 = document.getElementById("c4").value;




  // Update UI immediately
  setSlider("c1", "coil1", c1);
  setSlider("c2", "coil2", c2);
  setSlider("c3", "coil3", c3);
  setSlider("c4", "coil4", c4);


  // Send to ESP32
  const xhr = new XMLHttpRequest();
  xhr.open(
    "GET",
    `/slider?coil1=${c1}&coil2=${c2}&coil3=${c3}&coil4=${c4}&Mmode=${ModeSet}`,
    true
  );
  xhr.send();
}

/* ================= MODE BUTTON ================= */
function updateMode(mode) {
  ModeSet = mode;

  // Send mode to ESP32
  const xhr = new XMLHttpRequest();
  xhr.open("GET", `/slider?Mmode=${mode}`, true);
  xhr.send();



  // 🔥 Force instant sync from ESP32
  setTimeout(getOutput, 100);
}