var fs = require('fs');
var mongo = require('mongodb');

// For: read data & get dps
var data = '';
var dps = [];
var readStream = fs.createReadStream('smoke.txt', 'utf8');

// For: store data to database
var MongoClient = mongo.MongoClient;
var url = "mongodb://localhost:27017/mydb";


/////////////////////////////////////////////
// Functions & Protocals

// read data from .txt file
readStream.on('data', function(chunk) {
    data += chunk;
}).on('end', function() {
    dps = getDataPointsFromTSV(data);
    // console.log('dataPoints: ', dps);
});

// put data into array "dps"
function getDataPointsFromTSV(tsv) {
    var dataPoints = [];
    var tsvLines = [];
    var points = [];
    tsvLines = tsv.split(/[\r?\n|\r|\n]+/);  // split by newlines - original: tsv.split(/[\r?\n|\r|\n]+/)
    // console.log(tsvLines);

    for (var i = 0; i < tsvLines.length; i++)
        if (tsvLines[i].length > 0) {
            points = tsvLines[i].split(/[\t]+/);
            // push different data into different dps array
            dataPoints.push({
                time: parseInt(points[0]),
                id: parseInt(points[1]),
                smoke: parseInt(points[2]),
                temp: parseFloat(points[3])
            });
        }

    return dataPoints;
}

// For: store data to database
MongoClient.connect(url, function(err, db) {
  if (err) throw err;
  var dbo = db.db("mydb");
  dbo.collection("customers").insertMany(dps, function(err, res) {
    if (err) throw err;
    console.log("Number of documents inserted: " + res.insertedCount);
    // console.log(res.ops);
    db.close();
  });
});

// For: obtain data from database

MongoClient.connect(url, function(err, db) {
    if (err) throw err;
    var dbo = db.db("mydb");
    dbo.collection("customers").find({}, { projection: { _id: 0 } }).toArray(function(err, result) {
      if (err) throw err;
      console.log("Retreive all data in the database:")
      console.log(result);
      db.close();
    });
  });


/*MongoClient.connect(url, function(err, db) {
  if (err) throw err;
  var dbo = db.db("mydb");
  var myquery = {};
  dbo.collection("customers").deleteMany(myquery, function(err, obj) {
    if (err) throw err;
    console.log(obj.result.n + " document(s) deleted");
    db.close();
  });
});*/
