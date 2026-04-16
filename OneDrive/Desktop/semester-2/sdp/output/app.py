from flask import Flask, render_template, request, redirect
import subprocess
import os

app = Flask(__name__)

# ---------------- PATIENT MODULE ---------------- #

@app.route('/')
def home():
    patients = []

    if os.path.exists("patients.txt"):
        with open("patients.txt", "r") as file:
            for line in file:
                patients.append(line.strip().split(","))

    return render_template("patient.html", patients=patients)


@app.route('/add', methods=['POST'])
def add_patient():
    pid = request.form['id']
    name = request.form['name']
    age = request.form['age']
    problem = request.form['problem']

    data = f"1\n{pid}\n{name}\n{age}\n{problem}\n4\n"

    subprocess.run(
        "patient.exe",
        input=data,
        text=True,
        capture_output=True
    )

    return redirect('/view')


@app.route('/view')
def view_patients():
    patients = []

    if os.path.exists("patients.txt"):
        with open("patients.txt", "r") as file:
            for line in file:
                patients.append(line.strip().split(","))

    return render_template("patient.html", patients=patients)


# ---------------- DOCTOR MODULE ---------------- #

@app.route('/doctor')
def doctor_home():
    appointments = []

    if os.path.exists("doctor_appointments.txt"):
        with open("doctor_appointments.txt", "r") as file:
            for line in file:
                row = line.strip().split(",")

                # only valid rows
                if len(row) == 5:
                    appointments.append(row)

    return render_template("doctor.html", appointments=appointments)


@app.route('/doctor/add', methods=['POST'])
def add_appointment():
    pid = request.form['id']
    name = request.form['name']
    doctor = request.form['doctor']
    time = request.form['time']

    # doctor status logic
    if doctor == "Dr.Naveen Kumar":
        status = "Confirmed"
    else:
        status = "Pending"

    with open("doctor_appointments.txt", "a") as file:
        file.write(f"{pid},{name},{doctor},{time},{status}\n")

    return redirect('/doctor')


@app.route('/doctor/view')
def view_appointments():
    appointments = []

    if os.path.exists("doctor_appointments.txt"):
        with open("doctor_appointments.txt", "r") as file:
            for line in file:
                row = line.strip().split(",")

                if len(row) == 5:
                    appointments.append(row)

    return render_template("doctor.html", appointments=appointments)


@app.route('/doctor/complete/<int:index>')
def complete_appointment(index):
    appointments = []

    if os.path.exists("doctor_appointments.txt"):
        with open("doctor_appointments.txt", "r") as file:
            for line in file:
                row = line.strip().split(",")

                if len(row) == 5:
                    appointments.append(row)

    if 0 <= index < len(appointments):
        appointments[index][4] = "Completed"

    with open("doctor_appointments.txt", "w") as file:
        for row in appointments:
            file.write(",".join(row) + "\n")

    return redirect('/doctor')


# ---------------- RUN APP ---------------- #

if __name__ == '__main__':
    app.run(debug=True)