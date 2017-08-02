// let express = require("express");
// let app = express();
// app.listen(300);

// app.get("/", function(req, res) {
//  // body...
//  res.send("trang chu");
// });
let http = require('http');
let MongoClient = require('mongodb').MongoClient;
let url = "mongodb://localhost:27017/mydb";


let mcu_time = 0;
let server_time = 0;

let express = require('express');
let bodyParser = require('body-parser');
const fs = require('fs');

let moment = require('moment');
let app = express();
let server = http.createServer(app);
app.use(bodyParser.json()); // to support JSON-encoded bodies
app.use(bodyParser.urlencoded({ // to support URL-encoded bodies
    extended: true
}));

app.use(bodyParser.json());

app.get('/', function(req, res) {
    // fs.readFile('index.html', function(err, data) {
    //     res.writeHead(200, { 'Content-Type': 'text/html' });
    //     res.write(data);
    //     res.end();
    // });
    res.sendFile(__dirname + '/index.html');
});
app.post('/data', function(req, res) {
    let t = req.body.t,
        h = req.body.h,
        postDataTime = req.body.time;
    let timestamp = moment(server_time).add((postDataTime - mcu_time) / 1000, 's').format();
    MongoClient.connect(url, function(err, db) {
        if (err) throw err;
        let myobj = {temperature: t, Humidity: h, timestamp: timestamp };
        db.collection("info").insertOne(myobj, function(err, res) {
            if (err) throw err;
            // console.log("1 record inserted");
            db.close();
        });
    });
    res.send("OK");
});
app.get('/json', function(req, res) {
    // body...
    MongoClient.connect(url, function(err, db) {
        // assert.equal(null, err);
        db.collection('info').find().sort({ timestamp: -1 }).limit(5).toArray(
            function(err, items) {
                res.header('Access-Control-Allow-Origin', '*');
                res.send(items);
                db.close();
            }
        );
    });
});

app.post('/time', function(req, res) {
    // body...
    // let hours = currentTime.getHours();
    // let minutes = currentTime.getMinutes();
    // let seconds = currentTime.getSeconds();
    // let str = "";
    // str+= hours + ":";
    // str+= minutes + ":";
    // str+= seconds;
    //   console.log(str);
    // res.send(str);
    mcu_time = req.body.time;
    server_time = moment().format();
    res.send('OK');
});
let port = process.env.PORT || 3000;
server = app.listen(port, function() {
    let host = server.address().address
    let port = server.address().port

    console.log("Example app listening at http://%s:%s", host, port)
});