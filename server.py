from flask import Flask
from flask import request
import matplotlib.pyplot as plt
import io
import time
import base64

app = Flask(__name__)
storedWeights = []
storedFoodEaten = []
timeStamps = []

@app.route("/")
def hello():
    print("We got stuff")
    currentFoodEaten = (request.args.get("eaten"))
    currentWeight = (request.args.get("weight"))
    print("Eaten", currentFoodEaten, currentWeight)

    if currentFoodEaten != None and currentWeight != None:
        storedFoodEaten.append(float(currentFoodEaten))
        storedWeights.append(float(currentWeight))
        timeStamps.append(time.time())
        return "We received value:" +str(currentFoodEaten)
    else:

        plt.figure(figsize=(10, 8))
        plt.subplot(2,1,1)
        plt.plot(storedWeights, marker='o', color='blue')
        plt.title("Weight Data")
        plt.ylabel("Weight (grams)")
        plt.legend()
        plt.grid(True)

        plt.subplot(2,1,2)
        plt.plot(storedFoodEaten, marker='o', color='green')
        plt.title("Food Eaten")
        plt.xlabel("Time Passed")
        plt.ylabel("Food eaten (grams)")
        plt.legend()
        plt.grid(True)

        buffer = io.BytesIO()
        plt.savefig(buffer, format='png')
        buffer.seek(0)
        image_base64 = base64.b64encode(buffer.getvalue()).decode('utf-8')
        buffer.close()
        plt.close()

        return f"""
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <meta http-equiv="refresh" content="1">
        </head>
        <body>
            <img src="data:image/png;base64,{image_base64}" alt="Weight Data Graph">
        </body>
        </html>
        """