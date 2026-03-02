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
    const d = JSON.parse(xhr.responseText);

    setSlider("c1","coil1",d.c1);
    setSlider("c2","coil2",d.c2);
    setSlider("c3","coil3",d.c3);
    setSlider("c4","coil4",d.c4);

    document.getElementById("coil1Temp").innerText = d.t1.toFixed(1)+" °C";
    document.getElementById("coil2Temp").innerText = d.t2.toFixed(1)+" °C";
    document.getElementById("coil3Temp").innerText = d.t3.toFixed(1)+" °C";
    document.getElementById("coil4Temp").innerText = d.t4.toFixed(1)+" °C";

    // 🔥 Overheat popup
    document.getElementById("overheatPopup").style.display =
      d.overheat ? "flex" : "none";

    ModeSet = d.mode;
    updateModeButtons(d.mode);
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

