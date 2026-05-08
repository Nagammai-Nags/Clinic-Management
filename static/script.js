const patientSearch = document.getElementById("patientSearch");
const patientTable = document.getElementById("patientTable");
const billingSearch = document.getElementById("billingSearch");
const billingTable = document.getElementById("billingTable");

if (patientSearch && patientTable) {
  patientSearch.addEventListener("input", () => {
    const value = patientSearch.value.toLowerCase();
    patientTable.querySelectorAll("tbody tr").forEach((row) => {
      row.style.display = row.innerText.toLowerCase().includes(value) ? "" : "none";
    });
  });
}

if (billingSearch && billingTable) {
  billingSearch.addEventListener("input", () => {
    const value = billingSearch.value.toLowerCase();
    billingTable.querySelectorAll("tbody tr").forEach((row) => {
      const patientCell = row.children[1].innerText.toLowerCase();
      row.style.display = patientCell.includes(value) ? "" : "none";
    });
  });
}
