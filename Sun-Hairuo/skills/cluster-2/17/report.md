#  CanvasJS

Author: Hairuo Sun

Date: 2020-10-08
-----

## Summary
In this skill, I wrote the canvas_ajax.html and canvas_ajax.js files to plot the stock closing price data for 4 different companies as 4 different splines in a shared graph on a webpage. I read in a csv file that stores all the data, and then send the data from server (.js file) to client (.html file) through AJAX protocol. At client, the graph is plotted using canvasjs on. In the graph, stock closing price is classified by companies: amazon, facebook, google and msft, and all companies' closing price is compared for the same day across 10 different days.

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-2/17/images/stock_chart.png">
<p>Screenshot 1: Stock Pricing Graph</p>
<br/>
<br/>
<br/>
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-2/17/images/dynamic-chart.png">
<p>Screenshot 2: Dynamic Chart Example</p>
<br/>
<br/>
<br/>
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-2/17/images/multi-series-chart.png">
<p>Screenshot 3:Multi-Series Chart Example</p>
<br/>
<br/>
<br/>
</div>

## Modules, Tools, Source Used Including Attribution
* CanvasJS
* node.js
* CSV
* HTML
* AJAX
* Express
* csv-parse
* fs (filesystem)
* path

## Supporting Artifacts
* [GitHub repo for my skill 17 completion](https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-2/17/)
* [DataloadingTest Example Code for AJAX](https://github.com/BU-EC444/code-examples/tree/master/DataLoadingTest)
* [Creating Charts from CSV](https://canvasjs.com/docs/charts/how-to/create-charts-from-csv/)
* [Visualization Brief](http://whizzer.bu.edu/briefs/design-patterns/dp-visual)
* [Stock Closing Price Data](http://whizzer.bu.edu/skills/stocks-csv.txt)
* [JavaScript Multi Series Charts & Graphs](https://canvasjs.com/javascript-charts/multi-series-chart/)
* [Setinverval JS function](https://www.w3schools.com/jsref/met_win_setinterval.asp)
* [JQuery $(document).ready(function())](https://api.jquery.com/ready/)
* [JavaScript Line Charts & Graphs](https://canvasjs.com/html5-javascript-line-chart/)

-----
