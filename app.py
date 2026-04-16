from flask import Flask, render_template, request, redirect
import subprocess
import os

app = Flask(__name__)


# Load all patients from file
def load_patients():
    patients = []

    if os.path.exists("patients.txt"):
        with open("patients.txt", "r") as file:
            for line in file:
                data = line.strip().split(",")

                if len(data) == 7:
                    patients.append({
                        "id": data[0],
                        "name": data[1],
                        "age": data[2],
                        "problem": data[3],
                        "doctor": data[4],
                        "shift": data[5],
                        "status": data[6]
                    })

    return patients


# Home page
@app.route("/")
def home():
    patients = load_patients()
    return render_template("receptionist.html", patients=patients)


# Add patient + appointment
@app.route("/add_patient", methods=["POST"])
def add_patient():
    id = request.form["id"]
    name = request.form["name"]
    age = request.form["age"]
    problem = request.form["problem"]
    shift = request.form["shift"]

    # Send input to C program
    input_data = f"1\n{id}\n{name}\n{age}\n{problem}\n{shift}\n5\n"

    subprocess.run(
        "receptionist.exe",
        input=input_data,
        text=True,
        shell=True
    )

    return redirect("/")


# Update status
@app.route("/update_status", methods=["POST"])
def update_status():
    patient_id = request.form["id"]

    patients = load_patients()

    for patient in patients:
        if patient["id"] == patient_id:
            patient["status"] = "Completed"

    with open("patients.txt", "w") as file:
        for patient in patients:
            file.write(
                f'{patient["id"]},'
                f'{patient["name"]},'
                f'{patient["age"]},'
                f'{patient["problem"]},'
                f'{patient["doctor"]},'
                f'{patient["shift"]},'
                f'{patient["status"]}\n'
            )

    return redirect("/")


if __name__ == "__main__":
    app.run(debug=True)