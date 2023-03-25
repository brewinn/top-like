function updateStats() {
  var request = new XMLHttpRequest();
  request.open('GET', '/stats', true);
  request.onreadystatechange = function(){
    if (this.readyState == 4 && this.status == 200) {
      var stats = JSON.parse(this.responseText);
      var usages = stats["usages"];
      var usage_list = document.getElementById("usages");
      usage_list.innerHTML = '';
      for (var key in usages) {
        var usage = document.createElement('li');
        usage.innerText = key + ': ' + usages[key].toFixed(2);
        usage_list.appendChild(usage);
      }
    }
  }
  request.send();
}

// Retrieve initial stats
updateStats();

// Update every INTERVAL milliseconds
var INTERVAL = 50
setInterval(updateStats, INTERVAL);
