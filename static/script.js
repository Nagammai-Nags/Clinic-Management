const patientSearch = document.getElementById("patientSearch");
const patientTable = document.getElementById("patientTable");

if (patientSearch && patientTable) {
  patientSearch.addEventListener("input", () => {
    const value = patientSearch.value.toLowerCase();
    patientTable.querySelectorAll("tbody tr").forEach((row) => {
      row.style.display = row.innerText.toLowerCase().includes(value) ? "" : "none";
    });
  });
}
