const weightUrl = "https://script.google.com/macros/s/AKfycbwE4skSgj-AFvaGckixr8OwcBqtXvRkdcq8uHOaOHIZPWz4Z-j4ShIZz-YhlTZEDRrq/exec"; // Update to your Google Script URL

        // Fetch the weight from Google Sheets
        function fetchWeight() {
            fetch(weightUrl)
                .then(response => response.json())  // Parse the response as JSON
                .then(data => {
                    console.log("Received weight data:", data);
                    const weight = parseFloat(data.weight);  // Access the "weight" property

                    if (isNaN(weight)) {
                        document.getElementById("currentWeight").innerText = "Invalid data";
                    } else {
                        document.getElementById("currentWeight").innerText = weight;
                    }

                    // Check if weight exceeds max weight
                    if (data.overload) {
                        document.getElementById("statusMessage").innerText = "Overload! Buzzer Activated";
                    } else {
                        document.getElementById("statusMessage").innerText = "Normal";
                    }
                })
                .catch(error => {
                    console.error('Error fetching weight:', error);
                    document.getElementById("currentWeight").innerText = "Error";
                });
        }

        // Update weight every 1 second
        setInterval(fetchWeight, 200);