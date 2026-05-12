const patientSearch = document.getElementById("patientSearch");
const patientTable = document.getElementById("patientTable");
const billingSearch = document.getElementById("billingSearch");
const billingTable = document.getElementById("billingTable");
const appShell = document.querySelector(".app-shell");
const sidebar = document.querySelector(".sidebar");
const sidebarBrand = document.querySelector(".brand");
const appointmentForm = document.querySelector(".appointment-form");

if (appShell && sidebar && sidebarBrand) {
  const openSidebar = () => appShell.classList.add("sidebar-open");
  const closeSidebar = () => appShell.classList.remove("sidebar-open");

  sidebarBrand.addEventListener("mouseenter", openSidebar);
  sidebarBrand.addEventListener("focus", openSidebar);
  sidebar.addEventListener("mouseleave", closeSidebar);
}

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

if (appointmentForm) {
  const doctorSelect = appointmentForm.querySelector("[name='doctor_id']");
  const dateInput = appointmentForm.querySelector("[name='appointment_date']");
  const timeInput = appointmentForm.querySelector("[name='appointment_time']");
  const submitButton = appointmentForm.querySelector("button[type='submit'], button:not([type])");
  const note = document.getElementById("doctorAvailability");
  const schedule = JSON.parse(appointmentForm.dataset.schedule || "[]");
  const openMinute = 10 * 60;
  const closeMinute = 21 * 60;
  const slotMinutes = 15;

  const formatMinute = (value) => {
    const hour = Math.floor(value / 60);
    const minute = value % 60;
    return `${String(hour).padStart(2, "0")}:${String(minute).padStart(2, "0")}`;
  };

  const formatMinute12h = (value) => {
    const hour = Math.floor(value / 60);
    const minute = value % 60;
    const period = hour >= 12 ? "PM" : "AM";
    const hour12 = hour % 12 || 12;
    return `${String(hour12).padStart(2, "0")}:${String(minute).padStart(2, "0")} ${period}`;
  };

  const parseTime = (value) => {
    const match = /^(\d{2}):(\d{2})$/.exec(value);
    if (!match) return null;
    return Number(match[1]) * 60 + Number(match[2]);
  };

  const slotSuggestion = (value) => {
    const total = parseTime(value);
    if (total === null) return null;
    const clamped = Math.min(Math.max(total, openMinute), closeMinute);
    const previous = Math.floor(clamped / slotMinutes) * slotMinutes;
    const next = Math.min(Math.ceil(clamped / slotMinutes) * slotMinutes, closeMinute);
    return previous === next ? formatMinute12h(previous) : `${formatMinute12h(previous)} or ${formatMinute12h(next)}`;
  };

  const validSlot = (value) => {
    const total = parseTime(value);
    return total !== null && total >= openMinute && total <= closeMinute && total % slotMinutes === 0;
  };

  const updateAvailability = () => {
    const dateValue = dateInput.value;
    const timeValue = timeInput.value;

    note.classList.remove("availability-note--ok", "availability-note--busy", "availability-note--warn");

    if (!dateValue || !timeValue) {
      note.textContent = "Select date and time to preview automatic scheduling.";
      submitButton.disabled = false;
      return;
    }

    if (!validSlot(timeValue)) {
      const suggestion = slotSuggestion(timeValue);
      note.classList.add("availability-note--warn");
      note.textContent = suggestion
        ? `Appointments take 15 minutes. Timing is 10:00 AM to 09:00 PM. Use ${suggestion}.`
        : "Appointments take 15 minutes. Timing is 10:00 AM to 09:00 PM.";
      submitButton.disabled = true;
      return;
    }

    if (!doctorSelect) {
      note.classList.add("availability-note--ok");
      note.textContent = `The scheduler will assign the earliest available doctor from ${formatMinute12h(parseTime(timeValue))}.`;
      submitButton.disabled = false;
      return;
    }

    const option = doctorSelect.selectedOptions[0];
    const doctorId = doctorSelect.value;
    if (!doctorId) {
      note.classList.add("availability-note--ok");
      note.textContent = `The scheduler will assign the earliest available doctor from ${formatMinute12h(parseTime(timeValue))}.`;
      submitButton.disabled = false;
      return;
    }

    const slot = `${dateValue} ${timeValue}`;
    const booked = schedule.find((item) => item.doctor_id === doctorId && item.slot === slot);
    const available = (option.dataset.available || "").toLowerCase();

    if (booked) {
      note.classList.add("availability-note--busy");
      note.textContent = `${option.textContent.trim()} already has ${booked.patient} at ${formatMinute12h(parseTime(timeValue))}. Choose another time.`;
      submitButton.disabled = true;
      return;
    }

    if (available && available !== "yes") {
      note.classList.add("availability-note--warn");
      note.textContent = `${option.textContent.trim()} is marked as ${option.dataset.available}, but this exact time is free.`;
      submitButton.disabled = false;
      return;
    }

    note.classList.add("availability-note--ok");
    note.textContent = `${option.textContent.trim()} is available at ${formatMinute12h(parseTime(timeValue))}.`;
    submitButton.disabled = false;
  };

  if (doctorSelect) doctorSelect.addEventListener("change", updateAvailability);
  dateInput.addEventListener("change", updateAvailability);
  timeInput.addEventListener("change", updateAvailability);
  timeInput.addEventListener("input", updateAvailability);
  updateAvailability();
}

const incomeChart = document.querySelector(".income-chart");

if (incomeChart) {
  const data = JSON.parse(incomeChart.dataset.income || "[]");
  const svg = incomeChart.querySelector("svg");
  const grid = svg.querySelector(".chart-grid");
  const yLabels = svg.querySelector(".chart-y-labels");
  const xLabels = svg.querySelector(".chart-x-labels");
  const line = svg.querySelector(".chart-line");
  const area = svg.querySelector(".chart-area");
  const points = svg.querySelector(".chart-points");
  const tooltip = document.createElement("div");

  tooltip.className = "chart-tooltip";
  incomeChart.appendChild(tooltip);

  const chart = { left: 104, right: 36, top: 28, bottom: 72, width: 740, height: 302 };
  const maxAmount = Math.max(...data.map((item) => Number(item.amount)), 0);
  const yMax = Math.max(100, Math.ceil(maxAmount / 100) * 100);
  const xFor = (day) => chart.left + ((day - 1) / 30) * chart.width;
  const yFor = (amount) => chart.top + chart.height - (Number(amount) / yMax) * chart.height;
  const make = (name, attrs = {}) => {
    const element = document.createElementNS("http://www.w3.org/2000/svg", name);
    Object.entries(attrs).forEach(([key, value]) => element.setAttribute(key, value));
    return element;
  };
  const formatAmount = (amount) => `Rs. ${Number(amount).toLocaleString("en-IN")}`;
  const formatAxisAmount = (amount) => {
    const value = Number(amount);
    if (value >= 100000) return `Rs. ${(value / 100000).toFixed(1).replace(".0", "")}L`;
    if (value >= 1000) return `Rs. ${(value / 1000).toFixed(1).replace(".0", "")}K`;
    return formatAmount(value);
  };
  const smoothPath = (items) => {
    if (!items.length) return "";

    const coords = items.map((item) => [xFor(item.day), yFor(item.amount)]);
    return coords.reduce((path, point, index) => {
      if (index === 0) return `M ${point[0]} ${point[1]}`;

      const previous = coords[index - 1];
      const controlX = (previous[0] + point[0]) / 2;
      return `${path} C ${controlX} ${previous[1]}, ${controlX} ${point[1]}, ${point[0]} ${point[1]}`;
    }, "");
  };

  for (let index = 0; index <= 5; index += 1) {
    const amount = (yMax / 5) * index;
    const y = yFor(amount);
    grid.appendChild(make("line", { x1: chart.left, x2: chart.left + chart.width, y1: y, y2: y }));
    yLabels.appendChild(make("text", {
      class: "chart-y-label",
      x: chart.left - 16,
      y: y + 3,
    })).textContent = formatAxisAmount(amount);
  }

  grid.appendChild(make("line", {
    class: "chart-axis",
    x1: chart.left,
    x2: chart.left,
    y1: chart.top,
    y2: chart.top + chart.height,
  }));
  grid.appendChild(make("line", {
    class: "chart-axis",
    x1: chart.left,
    x2: chart.left + chart.width,
    y1: chart.top + chart.height,
    y2: chart.top + chart.height,
  }));

  Array.from({ length: 31 }, (_, index) => index + 1).forEach((day) => {
    xLabels.appendChild(make("text", {
      class: "chart-x-label",
      x: xFor(day),
      y: chart.top + chart.height + 24,
    })).textContent = day;
  });

  const path = smoothPath(data);
  line.setAttribute("d", path);
  area.setAttribute(
    "d",
    `${path} L ${xFor(31)} ${chart.top + chart.height} L ${xFor(1)} ${chart.top + chart.height} Z`
  );

  data.forEach((item) => {
    const circle = make("circle", {
      class: "chart-point",
      cx: xFor(item.day),
      cy: yFor(item.amount),
      r: Number(item.amount) > 0 ? 5.5 : 4,
      tabindex: 0,
      "aria-label": `Day ${item.day}, paid amount ${formatAmount(item.amount)}`,
    });

    const showTooltip = () => {
      const rect = circle.getBoundingClientRect();
      circle.classList.add("chart-point--active");
      tooltip.innerHTML = `<span>Clinic income trend</span>Day ${item.day}`;
      tooltip.style.left = `${rect.left + rect.width / 2}px`;
      tooltip.style.top = `${rect.top}px`;
      tooltip.style.opacity = "1";
    };
    const hideTooltip = () => {
      circle.classList.remove("chart-point--active");
      tooltip.style.opacity = "0";
    };

    circle.addEventListener("mouseenter", showTooltip);
    circle.addEventListener("focus", showTooltip);
    circle.addEventListener("mouseleave", hideTooltip);
    circle.addEventListener("blur", hideTooltip);
    points.appendChild(circle);
  });
}
