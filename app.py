import os
import shutil
import subprocess
from pathlib import Path

from flask import Flask, flash, redirect, render_template, request, session, url_for

ROOT = Path(__file__).resolve().parent
BACKEND_SRC = ROOT / "backend" / "clinic_backend.c"
BACKEND_EXE = ROOT / "backend" / ("clinic_backend.exe" if os.name == "nt" else "clinic_backend")

app = Flask(__name__)
app.secret_key = "clinic-sdp-lab"

USERS = {
    "reception": {"password": "rec123", "role": "reception"},
    "doctor": {"password": "doc123", "role": "doctor", "doctor_id": "1", "doctor_name": "Dr. Varun"},
    "doctor2": {"password": "doc123", "role": "doctor", "doctor_id": "2", "doctor_name": "Dr. Tharun"},
    "doctor3": {"password": "doc123", "role": "doctor", "doctor_id": "3", "doctor_name": "Dr. Renita"},
    "billing": {"password": "bill123", "role": "billing"},
    "owner": {"password": "owner123", "role": "owner"},
}

HISTORY_FIELDS = ["visit_id", "date", "doctor", "symptoms", "diagnosis", "prescription"]


def compile_backend():
    backend_sources = list((ROOT / "backend").glob("*.c"))
    latest_source_time = max(source.stat().st_mtime for source in backend_sources)
    if BACKEND_EXE.exists() and BACKEND_EXE.stat().st_mtime >= latest_source_time:
        return

    compiler = os.environ.get("CLINIC_BACKEND_CC")
    if compiler:
        compiler_path = shutil.which(compiler) or compiler
    else:
        compiler_path = next(
            (path for name in ("gcc", "clang", "cc") if (path := shutil.which(name))),
            None,
        )

    if not compiler_path:
        raise RuntimeError(
            "No C compiler was found. Install GCC/MinGW-w64 or Clang and make sure it is on PATH. "
            "You can also set CLINIC_BACKEND_CC to your compiler executable."
        )

    try:
        subprocess.run(
            [compiler_path, "-std=c11", str(BACKEND_SRC), "-o", str(BACKEND_EXE)],
            cwd=ROOT,
            check=True,
            capture_output=True,
            text=True,
        )
    except FileNotFoundError as exc:
        raise RuntimeError(
            f"C compiler '{compiler_path}' was not found. Set CLINIC_BACKEND_CC to a valid compiler path."
        ) from exc
    except subprocess.CalledProcessError as exc:
        details = (exc.stderr or exc.stdout or "").strip()
        raise RuntimeError(f"Backend compilation failed:\n{details}") from exc


def run_backend(*args):
    try:
        compile_backend()
    except RuntimeError as exc:
        raise RuntimeError(str(exc)) from exc
    result = subprocess.run(
        [str(BACKEND_EXE), *map(str, args)],
        cwd=ROOT,
        capture_output=True,
        text=True,
    )
    output = result.stdout.strip()
    if result.returncode != 0:
        raise RuntimeError(output or result.stderr)
    return output


def rows(command, fields, *args):
    output = run_backend(command, *args)
    data = []
    for line in output.splitlines():
        parts = line.split("|")
        item = {field: parts[index] if index < len(parts) else "" for index, field in enumerate(fields)}
        data.append(item)
    return data


def patient_histories(patient_ids):
    return {str(patient_id): rows("history", HISTORY_FIELDS, patient_id) for patient_id in patient_ids}


def require_login(*allowed):
    role = session.get("role")
    if not role:
        return redirect(url_for("login"))
    if role == "owner" or role in allowed:
        return None
    flash("You do not have permission to open that page.", "danger")
    return redirect(url_for("dashboard"))


def require_staff_action(*allowed):
    guard = require_login(*allowed)
    if guard:
        return guard
    if session.get("role") == "owner":
        flash("Owner access is view-only for this panel.", "danger")
        return redirect(url_for("dashboard"))
    return None


def can_open_appointment(appointment_id):
    if session.get("role") == "owner":
        return True
    doctor_id = session.get("doctor_id")
    if not doctor_id:
        return False
    queue_fields = ["appointment_id", "patient_id", "patient_name", "doctor", "problem", "status", "date"]
    doctor_queue = rows("queue", queue_fields, doctor_id)
    return any(item["appointment_id"] == str(appointment_id) for item in doctor_queue)


@app.context_processor
def inject_user():
    return {
        "current_role": session.get("role"),
        "current_user": session.get("username"),
        "current_doctor": session.get("doctor_name"),
    }


@app.route("/", methods=["GET", "POST"])
@app.route("/login", methods=["GET", "POST"])
def login():
    if request.method == "POST":
        username = request.form["username"].strip()
        password = request.form["password"].strip()
        user = USERS.get(username)
        if user and user["password"] == password:
            session["username"] = username
            session["role"] = user["role"]
            session["doctor_id"] = user.get("doctor_id")
            session["doctor_name"] = user.get("doctor_name")
            return redirect(url_for("dashboard"))
        flash("Invalid username or password.", "danger")
    return render_template("login.html")


@app.route("/logout")
def logout():
    session.clear()
    return redirect(url_for("login"))


@app.route("/dashboard")
def dashboard():
    guard = require_login("reception", "doctor", "billing")
    if guard:
        return guard
    stat_fields = ["patients", "appointments", "invoices", "pending", "total", "paid"]
    stats = rows("stats", stat_fields)[0]
    return render_template("dashboard.html", stats=stats)


@app.route("/patients")
def patients():
    guard = require_login("reception")
    if guard:
        return guard
    patient_fields = [
        "id", "name", "age", "gender", "phone", "address", "blood",
        "problem", "doctor", "specialization", "status", "appointment_id",
    ]
    doctor_fields = ["id", "name", "specialization", "available", "fee"]
    patient_list = rows("list_patients", patient_fields)
    return render_template(
        "patients.html",
        patients=patient_list,
        doctors=rows("list_doctors", doctor_fields),
        histories=patient_histories(patient["id"] for patient in patient_list),
    )


@app.route("/patients/add", methods=["GET", "POST"])
def add_patient():
    guard = require_staff_action("reception")
    if guard:
        return guard
    if request.method == "POST":
        output = run_backend(
            "add_patient",
            request.form["name"],
            request.form["age"],
            request.form["gender"],
            request.form["phone"],
            request.form["address"],
            request.form["blood"],
        )
        flash(output.replace("|", " - "), "success")
        return redirect(url_for("patients"))
    return render_template("add_patient.html")


@app.route("/assign", methods=["POST"])
def assign():
    guard = require_staff_action("reception")
    if guard:
        return guard
    output = run_backend("assign", request.form["patient_id"], request.form["doctor_id"], request.form["problem"])
    flash(output.replace("|", " - "), "success")
    return redirect(url_for("patients"))


@app.route("/doctor", methods=["GET", "POST"])
def doctor():
    guard = require_login("doctor")
    if guard:
        return guard
    doctor_fields = ["id", "name", "specialization", "available", "fee"]
    if session.get("role") == "owner":
        selected = request.values.get("doctor_id", "0")
    else:
        selected = session.get("doctor_id", "0")
    queue_fields = ["appointment_id", "patient_id", "patient_name", "doctor", "problem", "status", "date"]
    patient_fields = [
        "id", "name", "age", "gender", "phone", "address", "blood",
        "problem", "doctor", "specialization", "status", "appointment_id",
    ]
    queue = rows("queue", queue_fields, selected)
    patient_lookup = {patient["id"]: patient for patient in rows("list_patients", patient_fields)}
    for item in queue:
        item["patient"] = patient_lookup.get(item["patient_id"], {})
    return render_template(
        "doctor.html",
        doctors=rows("list_doctors", doctor_fields),
        selected=selected,
        queue=queue,
        histories=patient_histories(item["patient_id"] for item in queue),
    )


@app.route("/doctor/start/<appointment_id>")
def start_consult(appointment_id):
    guard = require_staff_action("doctor")
    if guard:
        return guard
    if not can_open_appointment(appointment_id):
        flash("This patient is not assigned to your doctor account.", "danger")
        return redirect(url_for("doctor"))
    flash(run_backend("start_consult", appointment_id).replace("|", " - "), "success")
    return redirect(url_for("doctor"))


@app.route("/diagnosis/<appointment_id>/<patient_id>", methods=["GET", "POST"])
def diagnosis(appointment_id, patient_id):
    guard = require_staff_action("doctor")
    if guard:
        return guard
    if not can_open_appointment(appointment_id):
        flash("This patient is not assigned to your doctor account.", "danger")
        return redirect(url_for("doctor"))
    if request.method == "POST":
        output = run_backend(
            "add_diagnosis",
            appointment_id,
            request.form["symptoms"],
            request.form["diagnosis"],
            request.form["prescription"],
            request.form["medicine_fee"],
            request.form["lab_fee"],
        )
        flash(output.replace("|", " - "), "success")
        return redirect(url_for("billing"))
    return render_template(
        "diagnosis.html",
        appointment_id=appointment_id,
        patient_id=patient_id,
        history=rows("history", HISTORY_FIELDS, patient_id),
    )


@app.route("/billing")
def billing():
    guard = require_login("reception", "billing")
    if guard:
        return guard
    invoice_fields = [
        "invoice_id", "patient_id", "patient_name", "patient_phone", "doctor", "consultation",
        "medicine", "lab", "total", "paid", "status", "method", "date",
    ]
    invoices = rows("billing", invoice_fields)
    if session.get("role") == "reception":
        invoices = [invoice for invoice in invoices if invoice["status"] != "Paid"]
    return render_template("billing.html", invoices=invoices)


@app.route("/billing/pay", methods=["POST"])
def pay():
    guard = require_login("reception", "billing")
    if guard:
        return guard
    output = run_backend(
        "pay",
        request.form["invoice_id"],
        request.form["status"],
        request.form["method"],
        request.form["paid_amount"],
    )
    flash(output.replace("|", " - "), "success")
    return redirect(url_for("billing"))


if __name__ == "__main__":
    app.run(debug=True)
