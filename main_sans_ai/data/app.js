// Select elements from the DOM
const patientSelect = document.getElementById("patient-select");
const patientName = document.getElementById("patient-name");
const patientId = document.getElementById("patient-id");
const patientCondition = document.getElementById("patient-condition");
const addPatientBtn = document.getElementById("add-patient-btn");
const modal = document.getElementById("addPatientModal");
const closeModal = document.getElementById("close-modal");
const addPatientForm = document.getElementById("add-patient-form");
const patientNameInput = document.getElementById("patient-name-input");
const patientConditionInput = document.getElementById(
  "patient-condition-input"
);

// Patient data (This will usually come from the ESP32 server)
let patients = {
  1: { name: "John Doe", condition: "Stable" },
  2: { name: "Jane Smith", condition: "Critical" },
};

// Update patient info on the page when a patient is selected
patientSelect.addEventListener("change", (e) => {
  const selectedPatientId = e.target.value;
  if (patients[selectedPatientId]) {
    const patient = patients[selectedPatientId];
    patientName.textContent = patient.name;
    patientId.textContent = selectedPatientId;
    patientCondition.textContent = patient.condition;
  }
});

// Open the modal to add a new patient
addPatientBtn.addEventListener("click", () => {
  modal.style.display = "block";
});

// Close the modal when the user clicks on 'x'
closeModal.addEventListener("click", () => {
  modal.style.display = "none";
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



// Function to fetch patient data dynamically from the server
async function fetchPatientData(patientId) {
  try {
    const response = await fetch(`/patient/${patientId}`);
    if (!response.ok) {
      throw new Error('Patient data not found');
    }
    const data = await response.json();
    updatePatientInfo(data);
    updateCharts(data.spo2, data.bpm);
    addInsights(data);
    addAlerts(data);
  } catch (error) {
    console.error('Error fetching patient data:', error);
  }
}

// Function to update patient info dynamically
function updatePatientInfo(data) {
  document.getElementById("patient-name").textContent = data.name || "N/A"; // Assuming data.name is the patient's name
  document.getElementById("patient-id").textContent = data.id || "N/A"; // Assuming data.id is the patient ID
  document.getElementById("patient-condition").textContent =
    data.spo2 < 90 || data.bpm < 60 || data.bpm > 100 ? "Critical" : "Stable"; // Set the condition dynamically based on data
}

// Function to update charts based on real-time data
function updateCharts(spo2, bpm) {
  // Assuming you have a chart library (e.g., Chart.js) to dynamically update the charts
  const spo2Chart = document.getElementById('spo2-chart').getContext('2d');
  const bpmChart = document.getElementById('bpm-chart').getContext('2d');

  new Chart(spo2Chart, {
    type: 'line',
    data: {
      labels: ['Last Update'],
      datasets: [{
        label: 'SpO2',
        data: [spo2],
        borderColor: 'blue',
        fill: false,
      }]
    },
  });

  new Chart(bpmChart, {
    type: 'line',
    data: {
      labels: ['Last Update'],
      datasets: [{
        label: 'Heart Rate (BPM)',
        data: [bpm],
        borderColor: 'red',
        fill: false,
      }]
    },
  });
}

// Function to add insights based on the real-time data
function addInsights(data) {
  const insightsContainer = document.getElementById('insights-container');
  insightsContainer.innerHTML = ''; // Clear any previous insights

  if (data.spo2 < 90) {
    insightsContainer.innerHTML += '<p>Warning: Low SpO2 detected. Consider increasing oxygen supply.</p>';
  }
  if (data.bpm < 60) {
    insightsContainer.innerHTML += '<p>Warning: Low heart rate detected. Please check the patient.</p>';
  } else if (data.bpm > 100) {
    insightsContainer.innerHTML += '<p>Warning: High heart rate detected. Possible stress or other conditions.</p>';
  }
}

// Function to add alerts based on the real-time data
function addAlerts(data) {
  const alertsContainer = document.getElementById('alerts-container');
  alertsContainer.innerHTML = ''; // Clear any previous alerts

  if (data.spo2 < 90 || data.bpm < 60 || data.bpm > 100) {
    alertsContainer.innerHTML += '<p><strong>Alert:</strong> Critical patient condition detected! Immediate action needed.</p>';
  }
}


// Real-time updates with EventSource (assuming the server sends patient data)
// const eventSource = new EventSource(`/events/${patientId}`);
// eventSource.onmessage = function(event) {
//   const data = JSON.parse(event.data);
//   updatePatientInfo(data);
//   updateCharts(data.spo2, data.bpm);
//   addInsights(data);
//   addAlerts(data);
// };
