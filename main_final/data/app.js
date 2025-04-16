// Select elements from the DOM
const patientSelect = document.getElementById("patient-select");
const patientName = document.getElementById("patient-name");
const patientId = document.getElementById("patient-id");
const patientMacAdd = document.getElementById("patient-mac-add");
const patientCondition = document.getElementById("patient-condition");
const addPatientBtn = document.getElementById("add-patient-btn");
const updatePatientBtn = document.getElementById("update-patient-btn");
const modalAdd = document.getElementById("addPatientModal");
const modalUpdate = document.getElementById("updatePatientModal");
const closeModalAdd = document.getElementById("close-modal");
const closeModalUpdate = document.getElementById("close-modal-update");
const addPatientForm = document.getElementById("add-patient-form");
const updatePatientForm = document.getElementById("update-patient-form");

const base_url = "https://nodejs-serverless-patient-monitoring.vercel.app/api";

// Open the modal to add a new patient
addPatientBtn.addEventListener("click", () => {
  modalAdd.style.display = "block";
});

// Close the modal when the user clicks on 'x'
closeModalAdd.addEventListener("click", () => {
  modalAdd.style.display = "none";
});

// Open the modal to add a new patient
updatePatientBtn.addEventListener("click", () => {
  modalUpdate.style.display = "block";
});

// Close the modal when the user clicks on 'x'
closeModalUpdate.addEventListener("click", () => {
  modalUpdate.style.display = "none";
});

// Fetch all patients and populate the select dropdown
async function fetchPatients() {
  try {
    const response = await fetch(`${base_url}/patients`);
    const patients = await response.json();

    console.log(patients); // Log the response to check its structure

    // Clear current options
    patientSelect.innerHTML = `<option value="" disabled selected>Select a Patient</option>`;

    // Populate select options
    patients.forEach((patient) => {
      const option = document.createElement("option");
      option.value = patient.mac_address; // Assuming mac_address is unique
      option.textContent = `${patient.name} (ID: ${patient.patient_id})`;
      patientSelect.appendChild(option);
    });
  } catch (error) {
    console.error("Error fetching patients:", error);
  }
}

// Call fetchPatients when the page is loaded
document.addEventListener("DOMContentLoaded", fetchPatients);

const spo2Ctx = document.getElementById("spo2Chart").getContext("2d");
const bpmCtx = document.getElementById("bpmChart").getContext("2d");
const bodyTempCtx = document.getElementById("bodyTempChart").getContext("2d");

let spo2Chart, bpmChart, bodyTempChart;

// Fetch patient info when selected
patientSelect.addEventListener("change", async (e) => {
  const macAddress = e.target.value;
  console.log(macAddress);

  if (macAddress) {
    try {
      const response = await fetch(
        `${base_url}/patient?mac_address=${macAddress}`
      );
      const patientInfo = await response.json();

      // // Update the DOM with patient info
      // patientName.textContent = patientInfo.name || "N/A";
      // patientId.textContent = patientInfo.patient_id || "N/A";
      // // patientCondition.textContent = patientInfo.condition || "N/A";
      // patientMacAdd.textContent = patientInfo.mac_address || "N/A";

      if (patientInfo) {
        patientName.textContent = patientInfo.name || "N/A";
        patientId.textContent = patientInfo.patient_id || "N/A";
        patientMacAdd.textContent = patientInfo.mac_address || "N/A";
        patientCondition.textContent = patientInfo.condition || "N/A";
      }

      if (
        patientInfo &&
        patientInfo.sensor_data &&
        patientInfo.sensor_data.length > 0
      ) {
        // Update the DOM with patient info
        // patientName.textContent = patientInfo.name || "N/A";
        // patientId.textContent = patientInfo.patient_id || "N/A";
        // patientMacAdd.textContent = patientInfo.mac_address || "N/A";
        patientCondition.textContent =
          patientInfo.sensor_data[0].condition || "N/A";

        // Prepare data for charting
        const sensorData = patientInfo.sensor_data || [];
        const timestamps = sensorData.map((data) =>
          new Date(data.timestamp).toLocaleTimeString()
        );
        const spo2Data = sensorData.map((data) => data.spo2);
        const bpmData = sensorData.map((data) => data.bpm);
        const bodyTempData = sensorData.map((data) =>
          parseFloat(data.body_temp)
        );

        // Destroy existing charts if they exist to avoid duplicates
        if (spo2Chart) spo2Chart.destroy();
        if (bpmChart) bpmChart.destroy();
        if (bodyTempChart) bodyTempChart.destroy();

        // Create SpO2 Chart
        spo2Chart = new Chart(spo2Ctx, {
          type: "line",
          data: {
            labels: timestamps, // X-axis: timestamps
            datasets: [
              {
                label: "SpO2",
                data: spo2Data, // Y-axis: SpO2 values
                borderColor: "rgba(75, 192, 192, 1)",
                fill: false,
                tension: 0.1,
              },
            ],
          },
          options: {
            scales: {
              x: {
                title: {
                  display: true,
                  text: "Time",
                },
              },
              y: {
                title: {
                  display: true,
                  text: "SpO2",
                },
              },
            },
            responsive: true,
            plugins: {
              title: {
                display: true,
                text: "SpO2 Over Time",
              },
            },
          },
        });

        // Create BPM Chart
        bpmChart = new Chart(bpmCtx, {
          type: "line",
          data: {
            labels: timestamps, // X-axis: timestamps
            datasets: [
              {
                label: "BPM",
                data: bpmData, // Y-axis: BPM values
                borderColor: "rgba(153, 102, 255, 1)",
                fill: false,
                tension: 0.1,
              },
            ],
          },
          options: {
            scales: {
              x: {
                title: {
                  display: true,
                  text: "Time",
                },
              },
              y: {
                title: {
                  display: true,
                  text: "BPM",
                },
              },
            },
            responsive: true,
            plugins: {
              title: {
                display: true,
                text: "BPM Over Time",
              },
            },
          },
        });

        // Create Body Temperature Chart
        bodyTempChart = new Chart(bodyTempCtx, {
          type: "line",
          data: {
            labels: timestamps, // X-axis: timestamps
            datasets: [
              {
                label: "Body Temperature",
                data: bodyTempData, // Y-axis: Body Temp values
                borderColor: "rgba(255, 159, 64, 1)",
                fill: false,
                tension: 0.1,
              },
            ],
          },
          options: {
            scales: {
              x: {
                title: {
                  display: true,
                  text: "Time",
                },
              },
              y: {
                title: {
                  display: true,
                  text: "Temperature (°C)",
                },
              },
            },
            responsive: true,
            plugins: {
              title: {
                display: true,
                text: "Body Temperature Over Time",
              },
            },
          },
        });
      } else {
        alert("No sensor data found for the selected patient.");
      }
    } catch (error) {
      alert(error);
      console.error("Error fetching patient info:", error);
    }
  }
});

// Handle update patient name form submission
updatePatientForm.addEventListener("submit", async (e) => {
  e.preventDefault(); // Prevent form from refreshing the page

  const macAddress = patientMacAdd.textContent; // Assuming this is stored in the DOM
  const updatedName = document.getElementById("patient-name-input").value;

  if (macAddress && updatedName) {
    try {
      const response = await fetch(`${base_url}/updatePatientName`, {
        method: "PUT",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          mac_address: macAddress,
          name: updatedName,
        }),
      });

      const result = await response.json();
      if (response.ok) {
        patientName.textContent = updatedName;
        alert("Patient name updated successfully");
        modalUpdate.style.display = "none"; // Close the modal
      } else {
        alert("Error updating patient name");
      }
    } catch (error) {
      console.error("Error updating patient name:", error);
      alert("Failed to update patient name");
    }
  }
});

addPatientForm.addEventListener("submit", async (e) => {
  e.preventDefault();
});

document
  .getElementById("submit-patient-btn")
  .addEventListener("click", async function () {
    // Get input values
    const macAddress = document.getElementById("patient-mac-add-input").value;
    const name = document.getElementById("patient-name-add-input").value;

    // Validate inputs
    if (!macAddress || !name) {
      alert("Please fill out all fields.");
      return;
    }

    try {
      // Send POST request
      const response = await fetch(base_url + "/newPatient", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          mac_address: macAddress,
          name: name,
        }),
      });

      if (response.ok) {
        alert("Patient added successfully!");
        // Optionally reset the form or close the modal
        document.getElementById("add-patient-form").reset();
      } else {
        alert("Failed to add patient. Please try again.");
      }
    } catch (error) {
      console.error("Error:", error);
      alert("An error occurred while adding the patient.");
    }
  });
