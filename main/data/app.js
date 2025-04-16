
// Select elements from the DOM
const patientSelect = document.getElementById('patient-select');
const patientName = document.getElementById('patient-name');
const patientId = document.getElementById('patient-id');
const patientCondition = document.getElementById('patient-condition');
const addPatientBtn = document.getElementById('add-patient-btn');
const modal = document.getElementById('addPatientModal');
const closeModal = document.getElementById('close-modal');
const addPatientForm = document.getElementById('add-patient-form');
const patientNameInput = document.getElementById('patient-name-input');
const patientConditionInput = document.getElementById('patient-condition-input');

// Patient data (This will usually come from the ESP32 server)
let patients = {
    1: { name: "John Doe", condition: "Stable" },
    2: { name: "Jane Smith", condition: "Critical" }
};

// Update patient info on the page when a patient is selected
patientSelect.addEventListener('change', (e) => {
    const selectedPatientId = e.target.value;
    if (patients[selectedPatientId]) {
        const patient = patients[selectedPatientId];
        patientName.textContent = patient.name;
        patientId.textContent = selectedPatientId;
        patientCondition.textContent = patient.condition;
    }
});

// Open the modal to add a new patient
addPatientBtn.addEventListener('click', () => {
    modal.style.display = 'block';
});

// Close the modal when the user clicks on 'x'
closeModal.addEventListener('click', () => {
    modal.style.display = 'none';
});

// Add new patient (this would usually send data to the server, but here we'll just add it to the local object for simplicity)
// addPatientForm.addEventListener('submit', (e) => {
//     e.preventDefault();
//     const newPatientId = Object.keys(patients).length + 1;
//     const newPatient = {
//         name: patientNameInput.value,
//         condition: patientConditionInput.value
//     };
//     patients[newPatientId] = newPatient;

//     // Update the patient dropdown
//     const option = document.createElement('option');
//     option.value = newPatientId;
//     option.textContent = `${newPatient.name} (ID: ${newPatientId})`;
//     patientSelect.appendChild(option);

//     // Reset the form and close the modal
//     patientNameInput.value = '';
//     patientConditionInput.value = '';
//     modal.style.display = 'none';
// });


















































// Initialize global variables for charts
//stocker les graphes
let spChart, heartbeatChart;

// Set up Server-Sent Events (SSE) for real-time updates
const eventSource = new EventSource('/events');

// Handle real-time data updates
eventSource.onmessage = function (event) {
  const data = JSON.parse(event.data);

  // Update Patient Information
  document.getElementById('patient-name').textContent = "John Doe"; // Placeholder for real patient name
  document.getElementById('patient-id').textContent = "1234"; // Placeholder for real patient ID
  document.getElementById('patient-condition').textContent = 
    data.spo2 < 90 || data.bpm < 60 || data.bpm > 100 ? "Critical" : "Stable";

  // Update Charts
  updateCharts(data.spo2, data.bpm);

  // Add Insights and Alerts
  addInsights(data);
  addAlerts(data);
};

// Handle connection errors
eventSource.onerror = function () {
  console.error("Failed to connect to server for real-time updates.");
};

// Function to initialize the charts
function initCharts() {
  const ctxSp = document.getElementById('sp-chart').getContext('2d');
  const ctxHeartbeat = document.getElementById('heartbeat-chart').getContext('2d');

  // Oxygen Saturation Chart
  spChart = new Chart(ctxSp, {
    type: 'line',
    data: {
      labels: [], // Time labels
      datasets: [{
        label: 'SpO2 Levels (%)',
        data: [],
        borderColor: '#4caf50',
        borderWidth: 2,
        fill: false,
        tension: 0.1
      }]
    },
    options: {
      scales: {
        x: { title: { display: true, text: 'Time (ms)' } },
        y: { title: { display: true, text: 'SpO2 (%)' }, min: 70, max: 100 }
      }
    }
  });

  // Heartbeat Chart
  heartbeatChart = new Chart(ctxHeartbeat, {
    type: 'line',
    data: {
      labels: [], // Time labels
      datasets: [{
        label: 'Heartbeat (BPM)',
        data: [],
        borderColor: '#e74c3c',
        borderWidth: 2,
        fill: false,
        tension: 0.1
      }]
    },
    options: {
      scales: {
        x: { title: { display: true, text: 'Time (ms)' } },
        y: { title: { display: true, text: 'BPM' }, min: 40, max: 120 }
      }
    }
  });
}

// Function to update charts with new data
function updateCharts(spo2, bpm) {
  const currentTime = new Date().toLocaleTimeString();

  // Update SpO2 Chart
  spChart.data.labels.push(currentTime);
  spChart.data.datasets[0].data.push(spo2);
  if (spChart.data.labels.length > 20) spChart.data.labels.shift(); // Keep only last 20 points
  if (spChart.data.datasets[0].data.length > 20) spChart.data.datasets[0].data.shift();
  spChart.update();

  // Update Heartbeat Chart
  heartbeatChart.data.labels.push(currentTime);
  heartbeatChart.data.datasets[0].data.push(bpm);
  if (heartbeatChart.data.labels.length > 20) heartbeatChart.data.labels.shift(); // Keep only last 20 points
  if (heartbeatChart.data.datasets[0].data.length > 20) heartbeatChart.data.datasets[0].data.shift();
  heartbeatChart.update();
}

// Function to add insights based on data
function addInsights(data) {
  const insightsList = document.getElementById('insights-list');
  insightsList.innerHTML = ""; // Clear existing insights

  if (data.spo2 < 90) {
    const li = document.createElement('li');
    li.textContent = `Low SpO2 detected: ${data.spo2}%. Consider checking oxygen supply.`;
    insightsList.appendChild(li);
  }

  if (data.bpm < 60 || data.bpm > 100) {
    const li = document.createElement('li');
    li.textContent = `Abnormal heart rate detected: ${data.bpm} BPM. Immediate attention required.`;
    insightsList.appendChild(li);
  }

  if (data.spo2 >= 90 && data.bpm >= 60 && data.bpm <= 100) {
    const li = document.createElement('li');
    li.textContent = "Vitals are stable.";
    insightsList.appendChild(li);
  }
}

// Function to add alerts based on critical conditions
function addAlerts(data) {
  const alertsList = document.getElementById('alerts-list');
  alertsList.innerHTML = ""; // Clear existing alerts

  if (data.spo2 < 85) {
    const li = document.createElement('li');
    li.textContent = `Critical alert: SpO2 dangerously low (${data.spo2}%). Take action immediately.`;
    alertsList.appendChild(li);
  }

  if (data.bpm < 50 || data.bpm > 110) {
    const li = document.createElement('li');
    li.textContent = `Critical alert: Heart rate abnormal (${data.bpm} BPM). Emergency intervention required.`;
    alertsList.appendChild(li);
  }
}

// Initialize charts on page load
window.onload = function () {
  initCharts();
};
