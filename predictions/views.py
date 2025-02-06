from django.shortcuts import render
from django.http import JsonResponse
import joblib
import os
from django.conf import settings
import pandas as pd  # Import pandas
import plotly.express as px  # Import Plotly Express
from datetime import datetime
from django.views.decorators.csrf import csrf_exempt
import json


# Home Page
def home(request):
    return render(request, 'predictions/home.html')

# Predict API

def predict(request):
    if request.method == 'POST':
        try:
            # Eingaben des Benutzers
            date_input = request.POST.get('date')  # Format: YYYY-MM-DD
            time_input = request.POST.get('time')  # Format: HH:MM

            # Datum und Uhrzeit parsen
            dt = datetime.strptime(f"{date_input} {time_input}", "%Y-%m-%d %H:%M")

            # Backend-Generierung der Faktoren
            hour = dt.hour
            day_of_week = dt.weekday()  # 0 = Montag, 6 = Sonntag
            is_weekend = 1 if day_of_week in [5, 6] else 0  # Wochenende prüfen
            recent_activity = 5  # Beispiel: fester Wert oder dynamisch berechnet
            temperature = 22.5  # Beispiel: Wetterdaten-API oder fester Wert

            # Load the model
            model = joblib.load('predictions/xgboost_tuned_model.pkl')

            # Prepare the input data
            data = [[hour, day_of_week, is_weekend, recent_activity, temperature]]

            # Make predictions
            prediction = model.predict(data)[0]

            # Customize the result
            result = "Open" if prediction == 1 else "Closed"

            # Render result
            return render(request, 'predictions/result.html', {'result': result})
        except Exception as e:
            return JsonResponse({'error': str(e)}, status=400)

    return render(request, 'predictions/home.html')

def current_status(request):
    try:
        # Path to the `esp_data.json` file
        file_path = os.path.join('predictions', 'esp_data.json')

        # Check if the file exists
        if os.path.exists(file_path):
            # Read and parse the JSON file
            with open(file_path, 'r') as json_file:
                data = json.load(json_file)
            
            # Extract room, people_count, and door_state
            room = data.get("room", "Unknown Room")
            people_count = int(data.get("people_count", 0))
            door_state = int(data.get("door_state", 0))

            # Determine door status (Open or Closed)
            door_status = "Open" if door_state == 1 else "Closed"
        else:
            room = "Unknown Room"
            people_count = "N/A"
            door_status = "No ESP data file found."

    except Exception as e:
        room = "Unknown Room"
        people_count = "N/A"
        door_status = f"Error: {str(e)}"

    # Render the status page with the data
    return render(request, 'predictions/current_status.html', {
        'room': room,
        'people_count': people_count,
        'door_status': door_status
    })



# Charts Page
def charts(request):
    # Directory containing the data files
    data_dir = os.path.join('predictions', 'data')  # Update with your directory path

    # Check if the directory exists
    if not os.path.exists(data_dir):
        return render(request, 'predictions/charts.html', {"error": "Data directory not found!"})

    # Read all CSV files in the directory
    all_files = [os.path.join(data_dir, f) for f in os.listdir(data_dir) if f.endswith('.csv')]

    # Ensure there are files to read
    if not all_files:
        return render(request, 'predictions/charts.html', {"error": "No data files found!"})

    # Combine all files into a single DataFrame
    dataframes = [pd.read_csv(file) for file in all_files]
    combined_df = pd.concat(dataframes, ignore_index=True)

    # Mapping von Zahlen zu Wochentagen
    day_mapping = {
        0: "Sunday",
        1: "Monday",
        2: "Tuesday",
        3: "Wednesday",
        4: "Thursday",
        5: "Friday",
        6: "Saturday"
    }

    # Konvertiere "Day of Week" in Wochentagsnamen
    combined_df["Day of Week"] = combined_df["Day of Week"].map(day_mapping)

    # Setze die richtige Reihenfolge der Wochentage
    ordered_days = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
    combined_df["Day of Week"] = pd.Categorical(combined_df["Day of Week"], categories=ordered_days, ordered=True)

    # Berechne die durchschnittlichen Türzustände als Prozent
    average_door_state = combined_df.groupby("Day of Week", as_index=False)["Door State"].mean()
    average_door_state["Door State"] *= 100  # Umrechnung in Prozent

    # Generate Plotly charts
    line_chart = px.line(
        combined_df,
        x='Timestamp',
        y='Temperature',
        title='Temperature Over Time'
    )
    bar_chart = px.bar(
        average_door_state,
        x="Day of Week",
        y="Door State",
        title="Average Door State by Day of the Week (%)",
        labels={"Door State": "Average Door State (%)"},  # Achsenbeschriftung
        color="Door State",  # Farbgebung basierend auf dem Prozentwert
        color_continuous_scale="Viridis"
    )
    scatter_chart = px.scatter(
        combined_df,
        x='Temperature',
        y='Recent Activity',
        color='Door State',
        title='Recent Activity vs. Temperature'
    )

    # Convert charts to HTML
    charts_data = {
        "line_chart": line_chart.to_html(full_html=False),
        "bar_chart": bar_chart.to_html(full_html=False),
        "scatter_chart": scatter_chart.to_html(full_html=False),
    }

    return render(request, 'predictions/charts.html', {"charts_data": charts_data})

@csrf_exempt
def live_data(request):
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            file_path = os.path.join('predictions', 'esp_data.json')

            with open(file_path, 'w') as json_file:
                json.dump(data, json_file)

            return JsonResponse({'status': 'success', 'message': 'Data received'})
        except Exception as e:
            return JsonResponse({'status': 'error', 'message': str(e)}, status=400)

    elif request.method == 'GET':
        try:
            file_path = os.path.join('predictions', 'esp_data.json')
            if os.path.exists(file_path):
                with open(file_path, 'r') as json_file:
                    data = json.load(json_file)
                return JsonResponse(data)
            else:
                return JsonResponse({'status': 'error', 'message': 'No data available'}, status=404)
        except Exception as e:
            return JsonResponse({'status': 'error', 'message': str(e)}, status=400)