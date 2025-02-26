from flask import Flask, request, jsonify
import joblib
import pandas as pd
import os

# Initialize Flask app
app = Flask(__name__)

# Load the tuned XGBoost model
try:
    model = joblib.load("xgboost_tuned_model.pkl")
except Exception as e:
    print(f"Error loading model: {e}")
    model = None

# Define the prediction route
@app.route('/predict', methods=['POST'])
def predict():
    if not model:
        return jsonify({"error": "Model not loaded"}), 500

    try:
        # Parse incoming JSON data
        data = request.get_json()

        # Extract features and prepare a DataFrame
        features = pd.DataFrame([data])

        # Ensure feature order matches training
        features = features[['Hour', 'Day of Week', 'Is Weekend', 'Recent Activity', 'Temperature']]

        # Make a prediction
        prediction = model.predict(features)
        prediction_label = "Open" if prediction[0] == 1 else "Closed"

        # Return the prediction as JSON
        return jsonify({'prediction': prediction_label})
    except Exception as e:
        return jsonify({'error': str(e)}), 400

# Run the Flask app
if __name__ == "__main__":
    port = int(os.environ.get("PORT", 5000))
    app.run(debug=True, host='0.0.0.0', port=port)
