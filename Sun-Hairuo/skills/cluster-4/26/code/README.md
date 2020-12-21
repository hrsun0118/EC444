# Code Readme
Author: Hairuo Sun

Date: 2020-11-09

# Code Explanation
In the db.js file, I store data from smoke.txt into mongodb's database and obtain the data from this database. To achieve this goal, I went through 3 steps:
1. First, install mongodb and start its service in terminal.
2. Use "readStream" to get the smoke data from smoke.txt file. Parse the data into a 2D array. (data parsing is not required.)
3. Instantiate a MongoClient and a specific URL. Store the data using "insertMany()".
4. Retrieve the data from Mongodb database using "find()" method with MonogoCLient.

# Attribution
* [Design Pattern – Databases + Visualization](http://whizzer.bu.edu/briefs/design-patterns/dp-db)
* [Node.js MongoDB](https://www.w3schools.com/nodejs/nodejs_mongodb.asp)
* [js - read tab delimited file line by line](https://stackoverflow.com/questions/28927640/javascript-read-tab-delimited-file-line-by-line-than-split-each-line-using-ta)
* [Read Files with Node.js](https://stackabuse.com/read-files-with-node-js/)
* [Node.js MongoDB Insert](https://www.w3schools.com/nodejs/nodejs_mongodb_insert.asp)
* [Node.js MongoDB Find](https://www.w3schools.com/nodejs/nodejs_mongodb_find.asp)


-----
