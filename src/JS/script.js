document.getElementById('vehicle-form').addEventListener('submit', function(e) {
    e.preventDefault();
    let vehicle = document.getElementById('vehicle-select').value;
    
    fetch('https://script.google.com/macros/s/AKfycbyldV64C-UN_ofXxscMjOMrdgKtQR2VCayEq-wXrizBKvqNeydALNENGGNhoQSa4-c/exec', {
      method: 'POST',
      body: JSON.stringify({ vehicle: vehicle })
    })
    .then(response => response.text())
    .then(data => {
      alert('Vehicle selection successful');
      document.getElementById('status').innerText = 'Selected vehicle: ' + vehicle;
    });
  });
  
  function checkOverload() {
    // Fetch data from spreadsheet and update overload status
    fetch('https://script.google.com/macros/s/your_weight_script_url/exec')
      .then(response => response.text())
      .then(data => {
        let weight = parseFloat(data);
        if (weight > 0) {
          document.getElementById('status').innerText = 'Current weight: ' + weight;
        }
      });
  }
  
  setInterval(checkOverload, 1000);  // Update every second
  