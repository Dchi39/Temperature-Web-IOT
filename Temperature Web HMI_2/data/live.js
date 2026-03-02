/* ================= LIVE.JS ================= */
const ctx = document.getElementById('tempChart').getContext('2d');
const maxDataPoints = 10; // maximum readings on chart
const popup = document.getElementById('overheatPopup');

// Initialize Chart.js
const tempChart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: [], // dummy labels for each reading
        datasets: [
            { label: 'Coil1', data: [], borderColor: 'red', fill: false },
            { label: 'Coil2', data: [], borderColor: 'blue', fill: false },
            { label: 'Coil3', data: [], borderColor: 'green', fill: false },
            { label: 'Coil4', data: [], borderColor: 'orange', fill: false },
            // Setpoints as dotted lines
            { label: 'Setpoint1', data: [], borderColor: 'pink', borderDash: [5,5], fill: false, pointRadius: 0 },
            { label: 'Setpoint2', data: [], borderColor: 'cyan', borderDash: [5,5], fill: false, pointRadius: 0 },
            { label: 'Setpoint3', data: [], borderColor: 'lime', borderDash: [5,5], fill: false, pointRadius: 0 },
            { label: 'Setpoint4', data: [], borderColor: 'yellow', borderDash: [5,5], fill: false, pointRadius: 0 }
        ]
    },
    options: {
        responsive: true,
        maintainAspectRatio: false, // allow height from CSS
        animation: false,
        elements: { line: { tension: 0.2 } }, // slight curve for smooth lines
        scales: {
            y: {
                type: 'linear',
                min: 0,
                max: 200,
                ticks: {
                    stepSize: 20,
                    callback: function(value) {
                        if (value % 20 === 0) return value;
                        return null;
                    }
                },
                title: { display: true, text: 'Temperature (°C)' }
            },
            x: {
                display: false // hide X-axis
            }
        },
        plugins: {
            legend: { position: 'top' }
        }
    }
});

// Function to fetch data and update chart
function updateChart() {
    fetch('/getOutput')
        .then(res => res.json())
        .then(data => {
            // Add dummy label to maintain spacing
            tempChart.data.labels.push("");

            // Push coil temperatures
            tempChart.data.datasets[0].data.push(data.t1);
            tempChart.data.datasets[1].data.push(data.t2);
            tempChart.data.datasets[2].data.push(data.t3);
            tempChart.data.datasets[3].data.push(data.t4);

            // Push setpoints (always dotted)
            tempChart.data.datasets[4].data.push(data.c1);
            tempChart.data.datasets[5].data.push(data.c2);
            tempChart.data.datasets[6].data.push(data.c3);
            tempChart.data.datasets[7].data.push(data.c4);

            // Keep max data points without compressing chart
            if (tempChart.data.labels.length > maxDataPoints) {
                tempChart.data.labels.shift();
                tempChart.data.datasets.forEach(ds => ds.data.shift());
            }

            tempChart.update();

            // Show overheat popup
            popup.style.display = data.overheat ? 'flex' : 'none';
        })
        .catch(err => console.error('Fetch error:', err));
}

// Update every second
setInterval(updateChart, 1000);