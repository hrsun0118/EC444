# Code Readme
Author: Hairuo Sun

Date: 2020-10-08

# Code Explanation
In this folder, there are 5 components:
1. "canvas_ajax.html" file as client
2. "canvas_ajax.js" file as server
3. "stock.csv" file that includes data to be read
4. node_modules folder that contains packages for running ".js" file
5. package.json & package-lock.json files that are generated when running "node canvas_ajax.js".

I runned "node canvas_ajax.js" in terminal. First, the server side (".js" file) read in a csv file that stores all the data, and then send the data to the client side(".html "file) through AJAX protocol. Then, at client side, data is fetched through ajax as well and passed to a global variable "data". After data is obtained, I called the "build_chart" function to build the stocking price chart. Inside the build_chart function, I called "sort_data(brand)" function 4 times to get datasets for the 4 different brands' stocking closing price to be displayed. Eventually, I was able to successfully get the "Stock Closing Price Chart".


# Attribution
* [DataloadingTest Example Code for AJAX](https://github.com/BU-EC444/code-examples/tree/master/DataLoadingTest)
* [Creating Charts from CSV](https://canvasjs.com/docs/charts/how-to/create-charts-from-csv/)
* [Visualization Brief](http://whizzer.bu.edu/briefs/design-patterns/dp-visual)
* [Stock Closing Price Data](http://whizzer.bu.edu/skills/stocks-csv.txt)
* [JavaScript Multi Series Charts & Graphs](https://canvasjs.com/javascript-charts/multi-series-chart/)
* [Setinverval JS function](https://www.w3schools.com/jsref/met_win_setinterval.asp)
* [JQuery $(document).ready(function())](https://api.jquery.com/ready/)
* [JavaScript Line Charts & Graphs](https://canvasjs.com/html5-javascript-line-chart/)
